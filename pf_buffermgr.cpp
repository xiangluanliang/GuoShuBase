//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/27 16:59
// @Project: GuoShuBase
//
//

#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "pf_buffermgr.h"

using namespace std;

// 如果定义了PF_STATS，表示用户希望跟踪PF层的统计信息
#ifdef PF_STATS
#include "statistics.h"   // 包含用于StatisticsMgr接口的头文件

// 全局变量，用于统计管理器
StatisticsMgr *pStatisticsMgr;
#endif

// 如果定义了PF_LOG，包含日志记录功能
#ifdef PF_LOG

//
// WriteLog
//
// 这是一个独立的单元，用于创建一个新的日志文件并将psMessage写入该文件。
// 注意，我不会在这里关闭fLog文件。如果一切顺利，程序退出时将自动关闭。
//
void WriteLog(const char *psMessage)
{
   static FILE *fLog = NULL;

   // 第一次调用时创建一个新的日志文件
   if (fLog == NULL) {
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
         cerr << "Cannot create a new log file!\n";
         exit(1);
      }
   }
   fprintf (fLog, psMessage);
}
#endif

#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include "pf_buffermgr.h"
using namespace std;

//
// PF_BufferMgr类
//
// 这个类实现了缓冲管理器，负责管理内存中的页缓存。
// 包括页的分配、读取、写入、标记脏页、取消固定页等功能。
//
PF_BufferMgr::PF_BufferMgr(int _numPages) : hashTable(PF_HASH_TBL_SIZE) {
    this->numPages = _numPages;
    pageSize = PF_PAGE_SIZE + sizeof(PF_PageHdr);
    bufTable = new PF_BufPageDesc[numPages];
    for (int i = 0; i < numPages; i++) {
        bufTable[i].pData = new char[pageSize];
        memset(bufTable[i].pData, 0, pageSize);
        bufTable[i].prev = i - 1;
        bufTable[i].next = i + 1;
    }
    bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
    free = 0;
    first = last = INVALID_SLOT;
}

//
// ~PF_BufferMgr析构函数
//
// 释放缓冲管理器中所有页的内存。
//
PF_BufferMgr::~PF_BufferMgr() {
    for (int i = 0; i < numPages; i++)
        delete[] bufTable[i].pData;
    delete[] bufTable;
}

//
// GetPage方法
//
// 获取一个固定在缓冲区的页的指针。如果页已经在缓冲区中，则重新固定页并返回指针。
// 如果页不在缓冲区中，则从文件读取页，固定页并返回指针。
// 如果缓冲区已满，则替换一个未固定的页。
//
RC PF_BufferMgr::GetPage(int fd, PageNum pageNum, char **ppBuffer, int bMultiplePins)
{
    RC  rc;     // 返回码
    int slot;   // 缓冲区中页的位置

#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Looking for (%d,%d).\n", fd, pageNum);
   WriteLog(psMessage);
#endif

#ifdef PF_STATS
    pStatisticsMgr->Register(PF_GETPAGE, STAT_ADDONE);
#endif

    // 在缓冲区中查找页
    if ((rc = hashTable.Find(fd, pageNum, slot)) &&
        (rc != PF_HASHNOTFOUND))
        return (rc);                // 非预期的错误

    // 如果页不在缓冲区中...
    if (rc == PF_HASHNOTFOUND) {

#ifdef PF_STATS
        pStatisticsMgr->Register(PF_PAGENOTFOUND, STAT_ADDONE);
#endif

        // 分配一个空页，这也会将新分配的页提升为最近使用页
        if ((rc = InternalAlloc(slot)))
            return (rc);

        // 读取页，将其插入哈希表，并初始化页描述条目
        if ((rc = ReadPage(fd, pageNum, bufTable[slot].pData)) ||
            (rc = hashTable.Insert(fd, pageNum, slot)) ||
            (rc = InitPageDesc(fd, pageNum, slot))) {

            // 在返回错误前，将该槽位重新放入空闲列表
            Unlink(slot);
            InsertFree(slot);
            return (rc);
        }
#ifdef PF_LOG
        WriteLog("Page not found in buffer. Loaded.\n");
#endif
    }
    else {   // 页在缓冲区中...

#ifdef PF_STATS
        pStatisticsMgr->Register(PF_PAGEFOUND, STAT_ADDONE);
#endif

        // 如果不允许固定已固定的页，返回错误
        if (!bMultiplePins && bufTable[slot].pinCount > 0)
            return (PF_PAGEPINNED);

        // 页已经在内存中，只需增加固定计数
        bufTable[slot].pinCount++;
#ifdef PF_LOG
        sprintf (psMessage, "Page found in buffer.  %d pin count.\n", bufTable[slot].pinCount);
      WriteLog(psMessage);
#endif

        // 将该页设置为最近使用页
        if ((rc = Unlink(slot)) ||
            (rc = LinkHead (slot)))
            return (rc);
    }

    // 将ppBuffer指向该页
    *ppBuffer = bufTable[slot].pData;

    // 返回成功码
    return (0);
}

//
// AllocatePage方法
//
// 分配一个新的页到缓冲区，并返回指向该页的指针。
//
RC PF_BufferMgr::AllocatePage(int fd, PageNum pageNum, char **ppBuffer)
{
    RC  rc;     // 返回码
    int slot;   // 缓冲区中页的位置

#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Allocating a page for (%d,%d)....", fd, pageNum);
   WriteLog(psMessage);
#endif

    // 如果页已经在缓冲区中，返回错误
    if (!(rc = hashTable.Find(fd, pageNum, slot)))
        return (PF_PAGEINBUF);
    else if (rc != PF_HASHNOTFOUND)
        return (rc);              // 非预期的错误

    // 分配一个空页
    if ((rc = InternalAlloc(slot)))
        return (rc);

    // 将页插入哈希表，并初始化页描述条目
    if ((rc = hashTable.Insert(fd, pageNum, slot)) ||
        (rc = InitPageDesc(fd, pageNum, slot))) {

        // 在返回错误前，将该槽位重新放入空闲列表
        Unlink(slot);
        InsertFree(slot);
        return (rc);
    }

#ifdef PF_LOG
    WriteLog("Succesfully allocated page.\n");
#endif

    // 将ppBuffer指向该页
    *ppBuffer = bufTable[slot].pData;

    // 返回成功码
    return (0);
}

//
// MarkDirty方法
//
// 标记一个页为脏页，使其在从缓冲区释放时能被写回磁盘。
//
RC PF_BufferMgr::MarkDirty(int fd, PageNum pageNum)
{
    RC  rc;       // 返回码
    int slot;     // 缓冲区中页的位置

#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Marking dirty (%d,%d).\n", fd, pageNum);
   WriteLog(psMessage);
#endif

    // 页必须在缓冲区中并且被固定
    if ((rc = hashTable.Find(fd, pageNum, slot))){
        if ((rc == PF_HASHNOTFOUND))
            return (PF_PAGENOTINBUF);
        else
            return (rc);              // 非预期的错误
    }

    if (bufTable[slot].pinCount == 0)
        return (PF_PAGEUNPINNED);

    // 标记页为脏页
    bufTable[slot].bDirty = TRUE;

    // 将页设为最近使用页
    if ((rc = Unlink(slot)) ||
        (rc = LinkHead (slot)))
        return (rc);

    // 返回成功码
    return (0);
}

//
// UnpinPage方法
//
// 取消固定一个页，使其可以被从缓冲区释放。
//
RC PF_BufferMgr::UnpinPage(int fd, PageNum pageNum)
{
    RC  rc;       // 返回码
    int slot;     // 缓冲区中页的位置

    // 页必须在缓冲区中并且被固定
    if ((rc = hashTable.Find(fd, pageNum, slot))){
        if ((rc == PF_HASHNOTFOUND))
            return (PF_PAGENOTINBUF);
        else
            return (rc);              // 非预期的错误
    }

    if (bufTable[slot].pinCount == 0)
        return (PF_PAGEUNPINNED);

#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Unpinning (%d,%d). %d Pin count\n", fd, pageNum, bufTable[slot].pinCount-1);
   WriteLog(psMessage);
#endif

    // 如果固定计数为0，将其设为最近使用页
    if (--(bufTable[slot].pinCount) == 0) {
        if ((rc = Unlink(slot)) ||
            (rc = LinkHead (slot)))
            return (rc);
    }

    // 返回成功码
    return (0);
}

//
// FlushPages方法
//
// 释放所有属于指定文件的页，并将其放到空闲列表中。
// 如果任何文件的页被固定，返回警告。
//
RC PF_BufferMgr::FlushPages(int fd)
{
    RC rc, rcWarn = 0;  // 返回码

#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Flushing all pages for (%d).\n", fd);
   WriteLog(psMessage);
#endif

#ifdef PF_STATS
    pStatisticsMgr->Register(PF_FLUSHPAGES, STAT_ADDONE);
#endif

    // 线性扫描缓冲区以找到属于该文件的页
    int slot = first;
    while (slot != INVALID_SLOT) {

        int next = bufTable[slot].next;

        // 如果页属于指定的文件描述符
        if (bufTable[slot].fd == fd) {

#ifdef PF_LOG
            sprintf (psMessage, "Page (%d) is in buffer manager.\n", bufTable[slot].pageNum);
 WriteLog(psMessage);
#endif
            // 确保页未被固定
            if (bufTable[slot].pinCount) {
                rcWarn = PF_PAGEPINNED;
            }
            else {
                // 如果页是脏页，写入磁盘
                if (bufTable[slot].bDirty) {
#ifdef PF_LOG
                    sprintf (psMessage, "Page (%d) is dirty\n",bufTable[slot].pageNum);
 WriteLog(psMessage);
#endif
                    if ((rc = WritePage(fd, bufTable[slot].pageNum, bufTable[slot].pData)))
                        return (rc);
                    bufTable[slot].bDirty = FALSE;
                }

                // 从哈希表中删除页，并将槽位添加到空闲列表中
                if ((rc = hashTable.Delete(fd, bufTable[slot].pageNum)) ||
                    (rc = Unlink(slot)) ||
                    (rc = InsertFree(slot)))
                    return (rc);
            }
        }
        slot = next;
    }

#ifdef PF_LOG
    WriteLog("All necessary pages flushed.\n");
#endif

    // 返回警告或成功码
    return (rcWarn);
}

//
// ForcePages方法
//
// 如果页是脏页，则强制将页写入磁盘。
// 页将不会从缓冲池中被强制移除。
//
RC PF_BufferMgr::ForcePages(int fd, PageNum pageNum)
{
    RC rc;  // 返回码

#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Forcing page %d for (%d).\n", pageNum, fd);
   WriteLog(psMessage);
#endif

    // 线性扫描缓冲区以找到属于该文件的页
    int slot = first;
    while (slot != INVALID_SLOT) {

        int next = bufTable[slot].next;

        // 如果页属于指定的文件描述符
        if (bufTable[slot].fd == fd &&
            (pageNum==ALL_PAGES || bufTable[slot].pageNum == pageNum)) {

#ifdef PF_LOG
            sprintf (psMessage, "Page (%d) is in buffer pool.\n", bufTable[slot].pageNum);
 WriteLog(psMessage);
#endif
            // 无论页是否固定，如果脏则写入磁盘
            if (bufTable[slot].bDirty) {
#ifdef PF_LOG
                sprintf (psMessage, "Page (%d) is dirty\n",bufTable[slot].pageNum);
WriteLog(psMessage);
#endif
                if ((rc = WritePage(fd, bufTable[slot].pageNum, bufTable[slot].pData)))
                    return (rc);
                bufTable[slot].bDirty = FALSE;
            }
        }
        slot = next;
    }

    return 0;
}

//
// PrintBuffer方法
//
// 显示缓冲区中的所有页。
// 该方法可通过系统命令调用。
//
RC PF_BufferMgr::PrintBuffer()
{
    cout << "Buffer contains " << numPages << " pages of size "
         << pageSize <<".\n";
    cout << "Contents in order from most recently used to "
         << "least recently used.\n";

    int slot, next;
    slot = first;
    while (slot != INVALID_SLOT) {
        next = bufTable[slot].next;
        cout << slot << " :: \n";
        cout << "  fd = " << bufTable[slot].fd << "\n";
        cout << "  pageNum = " << bufTable[slot].pageNum << "\n";
        cout << "  bDirty = " << bufTable[slot].bDirty << "\n";
        cout << "  pinCount = " << bufTable[slot].pinCount << "\n";
        slot = next;
    }

    if (first==INVALID_SLOT)
        cout << "Buffer is empty!\n";
    else
        cout << "All remaining slots are free.\n";

    return 0;
}

//
// ClearBuffer方法
//
// 从缓冲管理器中移除所有条目。
// 该方法可通过系统命令调用。
//
RC PF_BufferMgr::ClearBuffer()
{
    RC rc;

    int slot, next;
    slot = first;
    while (slot != INVALID_SLOT) {
        next = bufTable[slot].next;
        if (bufTable[slot].pinCount == 0)
            if ((rc = hashTable.Delete(bufTable[slot].fd,
                                       bufTable[slot].pageNum)) ||
                (rc = Unlink(slot)) ||
                (rc = InsertFree(slot)))
                return (rc);
        slot = next;
    }

    return 0;
}

//
// ResizeBuffer方法
//
// 调整缓冲管理器的大小。
// 该方法可通过系统命令调用。
//
RC PF_BufferMgr::ResizeBuffer(int iNewSize)
{
    int i;
    RC rc;

    // 首先尝试清空旧的缓冲区!
    ClearBuffer();

    // 分配新的缓冲表内存
    PF_BufPageDesc *pNewBufTable = new PF_BufPageDesc[iNewSize];

    // 初始化新的缓冲表并分配页内存。初始时，空闲列表包含所有页
    for (i = 0; i < iNewSize; i++) {
        if ((pNewBufTable[i].pData = new char[pageSize]) == NULL) {
            cerr << "Not enough memory for buffer\n";
            exit(1);
        }

        memset ((void *)pNewBufTable[i].pData, 0, pageSize);

        pNewBufTable[i].prev = i - 1;
        pNewBufTable[i].next = i + 1;
    }
    pNewBufTable[0].prev = pNewBufTable[iNewSize - 1].next = INVALID_SLOT;

    // 记录旧的first和last槽位以及缓冲表本身。然后使用插入方法将每个条目插入到新的缓冲表中
    int oldFirst = first;
    PF_BufPageDesc *pOldBufTable = bufTable;

    // 设置新的页数，first，last和free
    numPages = iNewSize;
    first = last = INVALID_SLOT;
    free = 0;

    // 设置新的缓冲表
    bufTable = pNewBufTable;

    // 必须先从哈希表中移除任何可能的条目
    int slot, next, newSlot;
    slot = oldFirst;
    while (slot != INVALID_SLOT) {
        next = pOldBufTable[slot].next;

        // 从哈希表中移除条目
        if ((rc=hashTable.Delete(pOldBufTable[slot].fd, pOldBufTable[slot].pageNum)))
            return (rc);
        slot = next;
    }

    // 现在遍历旧的缓冲表，将旧条目复制到新的缓冲表中
    slot = oldFirst;
    while (slot != INVALID_SLOT) {

        next = pOldBufTable[slot].next;
        // 为旧页分配一个新的槽位
        if ((rc = InternalAlloc(newSlot)))
            return (rc);

        // 将页插入哈希表，并初始化页描述条目
        if ((rc = hashTable.Insert(pOldBufTable[slot].fd,
                                   pOldBufTable[slot].pageNum, newSlot)) ||
            (rc = InitPageDesc(pOldBufTable[slot].fd,
                               pOldBufTable[slot].pageNum, newSlot)))
            return (rc);

        // 在返回错误前，将该槽位重新放入空闲列表
        Unlink(newSlot);
        InsertFree(newSlot);

        slot = next;
    }

    // 最后，删除旧的缓冲表
    delete [] pOldBufTable;

    return 0;
}

//
// InsertFree方法
//
// 在空闲列表头部插入一个槽位。
//
RC PF_BufferMgr::InsertFree(int slot)
{
    bufTable[slot].next = free;
    free = slot;

    return (0);
}

//
// LinkHead方法
//
// 在已使用列表头部插入一个槽位，使其成为最近使用的页。
//
RC PF_BufferMgr::LinkHead(int slot)
{
    bufTable[slot].next = first;
    bufTable[slot].prev = INVALID_SLOT;

    if (first != INVALID_SLOT)
        bufTable[first].prev = slot;

    first = slot;

    if (last == INVALID_SLOT)
        last = first;

    return (0);
}

//
// Unlink方法
//
// 从已使用列表中 unlink 一个槽位。假设槽位有效。将 prev 和 next 指针设置为 INVALID_SLOT。
// 调用者负责将 unlinked 页放入空闲列表或已使用列表。
//
RC PF_BufferMgr::Unlink(int slot)
{
    if (first == slot)
        first = bufTable[slot].next;

    if (last == slot)
        last = bufTable[slot].prev;

    if (bufTable[slot].next != INVALID_SLOT)
        bufTable[bufTable[slot].next].prev = bufTable[slot].prev;

    if (bufTable[slot].prev != INVALID_SLOT)
        bufTable[bufTable[slot].prev].next = bufTable[slot].next;

    bufTable[slot].prev = bufTable[slot].next = INVALID_SLOT;

    return (0);
}

//
// InternalAlloc方法
//
// 内部方法。分配一个缓冲区槽位。该槽位将被插入到已使用列表头部。
// 这里是选择使用哪个槽位的方法：
// 如果空闲列表中有槽位，则从空闲列表中选择一个。
// 否则，选择一个受害者页来替换。如果无法选择受害者页（因为所有页都被固定），则返回错误。
//
RC PF_BufferMgr::InternalAlloc(int &slot)
{
    RC  rc;       // 返回码

    if (free != INVALID_SLOT) {
        slot = free;
        free = bufTable[slot].next;
    }
    else {

        for (slot = last; slot != INVALID_SLOT; slot = bufTable[slot].prev) {
            if (bufTable[slot].pinCount == 0)
                break;
        }

        if (slot == INVALID_SLOT)
            return (PF_NOBUF);

        // 如果页是脏页，写入磁盘
        if (bufTable[slot].bDirty) {
            if ((rc = WritePage(bufTable[slot].fd, bufTable[slot].pageNum, bufTable[slot].pData)))
                return (rc);

            bufTable[slot].bDirty = FALSE;
        }

        // 从哈希表中删除页，并将槽位从已使用缓冲列表中移除
        if ((rc = hashTable.Delete(bufTable[slot].fd, bufTable[slot].pageNum)) ||
            (rc = Unlink(slot)))
            return (rc);
    }

    // 将槽位链接到已使用列表头部
    if ((rc = LinkHead(slot)))
        return (rc);

    return (0);
}

//
// ReadPage方法
//
// 从磁盘读取一个页。
//
RC PF_BufferMgr::ReadPage(int fd, PageNum pageNum, char *dest)
{
#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Reading (%d,%d).\n", fd, pageNum);
   WriteLog(psMessage);
#endif

#ifdef PF_STATS
    pStatisticsMgr->Register(PF_READPAGE, STAT_ADDONE);
#endif

    long offset = pageNum * (long)pageSize + PF_FILE_HDR_SIZE;
    if (lseek(fd, offset, L_SET) < 0)
        return (PF_UNIX);

    int numBytes = read(fd, dest, pageSize);
    if (numBytes < 0)
        return (PF_UNIX);
    else if (numBytes != pageSize)
        return (PF_INCOMPLETEREAD);
    else
        return (0);
}

//
// WritePage方法
//
// 将一个页写入磁盘。
//
RC PF_BufferMgr::WritePage(int fd, PageNum pageNum, char *source)
{
#ifdef PF_LOG
    char psMessage[100];
   sprintf (psMessage, "Writing (%d,%d).\n", fd, pageNum);
   WriteLog(psMessage);
#endif

#ifdef PF_STATS
    pStatisticsMgr->Register(PF_WRITEPAGE, STAT_ADDONE);
#endif

    long offset = pageNum * (long)pageSize + PF_FILE_HDR_SIZE;
    if (lseek(fd, offset, L_SET) < 0)
        return (PF_UNIX);

    int numBytes = write(fd, source, pageSize);
    if (numBytes < 0)
        return (PF_UNIX);
    else if (numBytes != pageSize)
        return (PF_INCOMPLETEWRITE);
    else
        return (0);
}

//
// InitPageDesc方法
//
// 内部方法。初始化一个新固定的页的PF_BufPageDesc条目。
//
RC PF_BufferMgr::InitPageDesc(int fd, PageNum pageNum, int slot)
{
    bufTable[slot].fd       = fd;
    bufTable[slot].pageNum  = pageNum;
    bufTable[slot].bDirty   = FALSE;
    bufTable[slot].pinCount = 1;

    return (0);
}

//------------------------------------------------------------------------------
// Methods for manipulating raw memory buffers
//------------------------------------------------------------------------------

#define MEMORY_FD -1

//
// GetBlockSize方法
//
// 返回可以分配的块大小。这里简单地返回页大小，因为块将占用缓冲池中的一页。
//
RC PF_BufferMgr::GetBlockSize(int &length) const
{
    length = pageSize;
    return OK_RC;
}

//
// AllocateBlock方法
//
// 分配一个在缓冲池中不关联特定文件的页，并返回指向数据区域的指针。
//
RC PF_BufferMgr::AllocateBlock(char *&buffer)
{
    RC rc = OK_RC;

    int slot;
    if ((rc = InternalAlloc(slot)) != OK_RC)
        return rc;

    PageNum pageNum = reinterpret_cast<intptr_t>(bufTable[slot].pData);

    if ((rc = hashTable.Insert(MEMORY_FD, pageNum, slot) != OK_RC) ||
        (rc = InitPageDesc(MEMORY_FD, pageNum, slot)) != OK_RC) {
        Unlink(slot);
        InsertFree(slot);
        return rc;
    }

    buffer = bufTable[slot].pData;

    return OK_RC;
}

//
// DisposeBlock方法
//
// 释放缓冲池中的内存块。
//
RC PF_BufferMgr::DisposeBlock(char* buffer)
{
    return UnpinPage(MEMORY_FD, reinterpret_cast<intptr_t>(buffer));
}

