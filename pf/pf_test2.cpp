//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/13 18:36
// @Project: GuoShuBase
//
//
#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "pf.h"
#include "pf_internal.h"

using namespace std;

// PF_STATS 宏定义表示我们将跟踪PF层的统计信息
// 统计管理器在 pf_buffermgr.cc 中定义
// 这里我们需要放置初始化代码并在main结束时打印最终统计信息
#ifdef PF_STATS
#include "../utils/statistics.h"

// 在 pf_buffermgr.cc 中定义的外部变量
extern StatisticsMgr *pStatisticsMgr;

// 在 pf_statistics.cc 中定义的方法
// 用于在最后显示统计信息，或由调试器监控进度
extern void PF_Statistics();
#endif

//
// 常量定义
//
#define FILE1 "file1"  // 测试文件名

// PF层测试函数
static RC TestPF()
{
    PF_Manager pfm;        // PF管理器
    PF_FileHandle fh;      // 文件句柄
    PF_PageHandle ph;      // 页句柄
    RC rc;                 // 返回码
    char *pData;           // 页数据指针
    PageNum pageNum;       // 页号
    int i;                 // 循环计数器

    // 1. 创建测试文件
    cout << "创建文件: " << FILE1 << endl;
    if ((rc = pfm.CreateFile(FILE1))) {
        return rc;
    }

    // 2. 打开文件
    cout << "打开文件: " << FILE1 << endl;
    if ((rc = pfm.OpenFile(FILE1, fh))) {
        return rc;
    }

    // 3. 分配缓冲区大小的页数
    cout << "分配 " << PF_BUFFER_SIZE << " 个页面" << endl;
    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        // 分配新页
        if ((rc = fh.AllocatePage(ph)) ||
            (rc = ph.GetData(pData)) ||
            (rc = ph.GetPageNum(pageNum))) {
            return rc;
        }

        // 验证页号是否正确
        if (i != pageNum) {
            cout << "页号不正确: " << (int)pageNum << " 应为 " << i << endl;
            exit(1);
        }

        // 将页号写入页数据
        memcpy(pData, (char *)&pageNum, sizeof(PageNum));
    }

    // 4. 再次请求相同的页面
    cout << "再次请求相同的页面" << endl;
    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        if ((rc = fh.GetThisPage(i, ph)) ||
            (rc = ph.GetData(pData)) ||
            (rc = ph.GetPageNum(pageNum))) {
            return rc;
        }

        // 验证页号
        if (i != pageNum) {
            cout << "页号不正确: " << (int)pageNum << " 应为 " << i << endl;
            exit(1);
        }
    }

    // 5. 验证缓冲区统计信息（如果启用了统计）
#ifdef PF_STATS
    cout << "验证缓冲区管理器统计信息: ";
    int *piGP = pStatisticsMgr->Get(PF_GETPAGE);    // 获取页面次数
    int *piPF = pStatisticsMgr->Get(PF_PAGEFOUND);  // 在缓冲区找到页面的次数
    int *piPNF = pStatisticsMgr->Get(PF_PAGENOTFOUND); // 未找到页面的次数

    // 验证统计值是否符合预期
    if (piGP && (*piGP != PF_BUFFER_SIZE)) {
        cout << "获取页面次数不正确! (" << *piGP << ")" << endl;
        exit(1);
    }
    if (piPF && (*piPF != PF_BUFFER_SIZE)) {
        cout << "缓冲区中找到页面次数不正确! (" << *piPF << ")" << endl;
        exit(1);
    }
    if (piPNF != NULL) {
        cout << "未找到页面次数不正确! (" << *piPNF << ")" << endl;
        exit(1);
    }
    cout << " 验证通过!" << endl;

    delete piGP;
    delete piPF;
    delete piPNF;
#endif

    // 6. 解除页面固定
    cout << "解除页面固定" << endl;
    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        // 需要解除固定两次
        if ((rc = fh.UnpinPage(i)) ||
            (rc = fh.UnpinPage(i))) {
            return rc;
        }
    }

    // 7. 验证写入统计信息
#ifdef PF_STATS
    cout << "验证缓冲区管理器写入统计: ";
    int *piWP = pStatisticsMgr->Get(PF_WRITEPAGE);  // 写入页面次数
    int *piRP = pStatisticsMgr->Get(PF_READPAGE);   // 读取页面次数

    if (piWP && (*piWP != PF_BUFFER_SIZE)) {
        cout << "写入页面次数不正确! (" << *piGP << ")" << endl;
        exit(1);
    }
    if (piRP != NULL) {
        cout << "读取页面次数不正确! (" << *piPNF << ")" << endl;
        exit(1);
    }
    cout << " 验证通过!" << endl;

    delete piWP;
    delete piRP;
#endif

    // 8. 分配额外页面以清空缓冲区
    cout << "分配额外 " << PF_BUFFER_SIZE << " 个页面以清空缓冲区池" << endl;
    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        if ((rc = fh.AllocatePage(ph)) ||
            (rc = ph.GetData(pData)) ||
            (rc = ph.GetPageNum(pageNum))) {
            return rc;
        }

        // 验证页号
        if (i + PF_BUFFER_SIZE != pageNum) {
            cout << "页号不正确: " << (int)pageNum << " 应为 " << i << endl;
            exit(1);
        }

        // 解除页面固定
        if ((rc = fh.UnpinPage(i + PF_BUFFER_SIZE))) {
            return rc;
        }
    }

//     9. 再次获取原始页面
    cout << "再次请求原始页面" << endl;
    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        if ((rc = fh.GetThisPage(i, ph)) ||
            (rc = ph.GetData(pData)) ||
            (rc = ph.GetPageNum(pageNum))) {
            return rc;
        }

        // 验证页号
        if (i != pageNum) {
            cout << "页号不正确: " << (int)pageNum << " 应为 " << i << endl;
            exit(1);
        }

        // 解除页面固定
        if ((rc = fh.UnpinPage(i))) {
            return rc;
        }
    }

    // 10. 验证页面不在缓冲区中
#ifdef PF_STATS
    cout << "验证页面不在缓冲区池中: ";
    piPNF = pStatisticsMgr->Get(PF_PAGENOTFOUND);

    if (piPNF && (*piPNF != PF_BUFFER_SIZE)) {
        cout << "未找到页面次数不正确! (" << *piPF << ")" << endl;
        exit(1);
    }
    cout << " 验证通过!" << endl;

    delete piPNF;
#endif

    // 11. 将缓冲区刷新到磁盘
    cout << "将文件句柄刷新到磁盘" << endl;
    if ((rc = fh.FlushPages())) {
        return rc;
    }

    // 12. 再次刷新并验证写入次数
    cout << "再次将文件句柄刷新到磁盘" << endl;
    if ((rc = fh.FlushPages())) {
        return rc;
    }

    // 13. 关闭文件
    if ((rc = pfm.CloseFile(fh))) {
        return rc;
    }

    // 14. 输出最终统计信息（如果启用了统计）
#ifdef PF_STATS
    PF_Statistics();
#endif

    return 0;  // 返回成功
}

// 主函数
int main()
{
    RC rc;

    // 输出初始信息
    cerr.flush();
    cout.flush();
    cout << "开始PF层测试" << endl;
    cout.flush();

    // 如果没有启用统计，输出警告
#ifndef PF_STATS
    cout << " ** PF层编译时未启用-DPF_STATS标志 **" << endl;
    cout << " **    没有统计信息测试效果有限    **" << endl;
#endif


    // 删除上次测试的文件
    unlink(FILE1);

    // 运行测试
    if ((rc = TestPF())) {
        PF_PrintError(rc);
        return 1;
    }

    // 输出结束信息
    cout << "PF层测试结束" << endl;
    cout << "********************" << endl << endl;

    return 0;
}
