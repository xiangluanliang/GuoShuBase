//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/28 11:50
// @Project: GuoShuBase
//
// 用于管理数据库系统中的页面缓冲区（Buffer Pool）。
// 它的主要功能是从磁盘读取、缓存和管理页面，提供固定（pin）、释放（unpin）、脏页标记（dirty marking）和页面替换等操作。


#ifndef PF_BUFFERMGR_H
#define PF_BUFFERMGR_H

#include <list>
#include <memory>
#include "pf_internal.h"
#include "pf_hashtable.h"

#define INVALID_SLOT  (-1)

struct PF_BufPageDesc {
    std::unique_ptr<char[]> pData;  // 自动管理内存
    int next;
    int prev;
    bool bDirty;
    short pinCount;
    PageNum pageNum;
    int fd;
};

class PF_BufferMgr {
public:
    PF_BufferMgr(int _numPages);
    ~PF_BufferMgr()=default;

    RC GetPage(int fd, PageNum pageNum, char **ppBuffer, int bMultiplePins = true);
    RC AllocatePage(int fd, PageNum pageNum, char **ppBuffer);
    RC MarkDirty(int fd, PageNum pageNum);
    RC UnpinPage(int fd, PageNum pageNum);
    RC FlushPages(int fd);
    RC ForcePages(int fd, PageNum pageNum = ALL_PAGES);
    RC ClearBuffer();
    RC PrintBuffer();
    RC ResizeBuffer(int iNewSize);
    RC GetBlockSize(int &length) const;
    RC AllocateBlock(char *&buffer);
    RC DisposeBlock(char *buffer);

private:
    RC InsertFree(int slot);
    RC LinkHead(int slot);
    RC Unlink(int slot);
    RC InternalAlloc(int &slot);
    RC ReadPage(int fd, PageNum pageNum, char *dest);
    RC WritePage(int fd, PageNum pageNum, char *source);
    RC InitPageDesc(int fd, PageNum pageNum, int slot);

    std::unique_ptr<PF_BufPageDesc[]> bufTable;
    PF_HashTable hashTable;
    int numPages;
    int pageSize;
    int first;
    int last;
    int free;
};

#endif //PF_BUFFERMGR_H
