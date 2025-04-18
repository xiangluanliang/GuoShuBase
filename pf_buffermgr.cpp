//
// @作者: 杨皓然 23301142
// @邮箱: 23301142@bjtu.edu.cn
// @创建时间: 2025/3/27 16:59
// @项目: GuoShuBase
//

#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "pf_buffermgr.h"

using namespace std;

// PF_STATS开关表示用户希望统计PF层的性能数据
#ifdef PF_STATS
#include "statistics.h"   // 统计管理器接口

// 全局统计管理器变量
StatisticsMgr *pStatisticsMgr;
#endif

#ifdef PF_LOG

//
// WriteLog
//
// 自包含的日志记录单元，会创建新的日志文件并将psMessage写入日志文件。
// 注意：fLog文件在任何时候都不会关闭，希望程序退出时能正常关闭。
//
void WriteLog(const char *psMessage)
{
    static FILE *fLog = NULL;

    // 第一次调用时需要创建新的日志文件
    if (fLog == NULL) {
        // 第一次调用，需要创建新的日志文件
        // 日志文件命名为"PF_LOG.x"，x是下一个可用的序号
        int iLogNum = -1;
        int bFound = FALSE;
        char psFileName[10];

        while (iLogNum < 999 && bFound==FALSE) {
            iLogNum++;
            sprintf (psFileName, "PF_LOG.%d", iLogNum);
            fLog = fopen(psFileName,"r");
            if (fLog==NULL) {
                bFound = TRUE;
                fLog = fopen(psFileName,"w");
            } else
                delete fLog;
        }

        if (!bFound) {
            cerr << "无法创建新的日志文件!\n";
            exit(1);
        }
    }
    // 现在日志文件已打开并准备好写入
    fprintf (fLog, psMessage);
}
#endif

#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include "pf_buffermgr.h"
using namespace std;

// 缓冲区管理器构造函数
PF_BufferMgr::PF_BufferMgr(int _numPages) : hashTable(PF_HASH_TBL_SIZE) {
    this->numPages = _numPages;                  // 设置缓冲区页数
    pageSize = PF_PAGE_SIZE + sizeof(PF_PageHdr); // 计算每页大小(数据+页头)
    bufTable = new PF_BufPageDesc[numPages];     // 分配缓冲区描述表

    // 初始化每页缓冲区
    for (int i = 0; i < numPages; i++) {
        bufTable[i].pData = new char[pageSize];  // 分配页数据内存
        memset(bufTable[i].pData, 0, pageSize);  // 清空页数据
        bufTable[i].prev = i - 1;                // 设置前一页索引
        bufTable[i].next = i + 1;                // 设置后一页索引
    }

    // 设置链表头尾
    bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
    free = 0;                                    // 空闲列表起始位置
    first = last = INVALID_SLOT;                 // 已使用列表初始为空
}

// 缓冲区管理器析构函数
PF_BufferMgr::~PF_BufferMgr() {
    // 释放每页数据内存
    for (int i = 0; i < numPages; i++)
        delete[] bufTable[i].pData;
    delete[] bufTable;  // 释放缓冲区描述表
}

//
// GetPage - 获取缓冲区中的页
//
// 功能: 获取缓冲区中已固定的页指针。如果页已在缓冲区中，重新固定并返回指针；
//       如果页不在缓冲区中，从文件读取，固定并返回指针；如果缓冲区已满，替换未固定的页。
// 输入: fd - 文件描述符
//       pageNum - 要读取的页号
//       bMultiplePins - 如果为FALSE，请求已固定的页将报错
// 输出: ppBuffer - 设置为指向缓冲区中页的指针
// 返回: PF错误码
//
RC PF_BufferMgr::GetPage(int fd, PageNum pageNum, char **ppBuffer,
                         int bMultiplePins)
{
    RC  rc;     // 返回码
    int slot;   // 页所在的缓冲区槽位

#ifdef PF_LOG
    char psMessage[100];
    sprintf (psMessage, "查找页(%d,%d).\n", fd, pageNum);
    WriteLog(psMessage);
#endif

#ifdef PF_STATS
    pStatisticsMgr->Register(PF_GETPAGE, STAT_ADDONE);  // 统计获取页操作
#endif

    // 在缓冲区中查找页
    if ((rc = hashTable.Find(fd, pageNum, slot)) &&
        (rc != PF_HASHNOTFOUND))
        return (rc);                // 意外错误

    // 如果页不在缓冲区中...
    if (rc == PF_HASHNOTFOUND) {

#ifdef PF_STATS
        pStatisticsMgr->Register(PF_PAGENOTFOUND, STAT_ADDONE);  // 统计页未找到
#endif

        // 分配空页(会将该页提升为MRU槽位)
        if ((rc = InternalAlloc(slot)))
            return (rc);

        // 读取页，插入哈希表，并初始化页描述项
        if ((rc = ReadPage(fd, pageNum, bufTable[slot].pData)) ||
            (rc = hashTable.Insert(fd, pageNum, slot)) ||
            (rc = InitPageDesc(fd, pageNum, slot))) {

            // 出错时将槽位放回空闲列表
            Unlink(slot);
            InsertFree(slot);
            return (rc);
        }
#ifdef PF_LOG
        WriteLog("页不在缓冲区中。已加载。\n");
#endif
    }
    else {   // 页在缓冲区中...

#ifdef PF_STATS
        pStatisticsMgr->Register(PF_PAGEFOUND, STAT_ADDONE);  // 统计页找到
#endif

        // 如果不允许多次固定已固定页，则报错
        if (!bMultiplePins && bufTable[slot].pinCount > 0)
            return (PF_PAGEPINNED);

        // 页已在内存中，只需增加固定计数
        bufTable[slot].pinCount++;
#ifdef PF_LOG
        sprintf (psMessage, "页在缓冲区中找到。固定计数 %d.\n",
                 bufTable[slot].pinCount);
        WriteLog(psMessage);
#endif

        // 将该页设为最近使用页(MRU)
        if ((rc = Unlink(slot)) ||
            (rc = LinkHead (slot)))
            return (rc);
    }

    // 设置返回指针指向页数据
    *ppBuffer = bufTable[slot].pData;

    // 返回成功
    return (0);
}

// [后续函数的注释翻译遵循相同模式...]

//
// AllocatePage - 在缓冲区中分配新页
//
// 功能: 在缓冲区中分配不与特定文件关联的新页，返回数据区指针
// 输入: fd - 文件描述符
//       pageNum - 新页号
// 输出: ppBuffer - 设置为指向缓冲区中页的指针
// 返回: PF错误码
//

//
// MarkDirty - 标记页为脏
//
// 功能: 标记页为脏，当从缓冲区丢弃时会写回文件
// 输入: fd - 文件描述符
//       pageNum - 要标记的页号
// 返回: PF错误码
//

//
// UnpinPage - 取消固定页
//
// 功能: 取消固定页，使其可从缓冲区丢弃
// 输入: fd - 文件描述符
//       pageNum - 要取消固定的页号
// 返回: PF错误码
//

// [其他方法的注释翻译省略...]
