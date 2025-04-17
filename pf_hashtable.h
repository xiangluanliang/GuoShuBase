//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/27 17:41
// @Project: GuoShuBase
//
// 啊，第一个类，类目
// 这个哈希表类提供方法以插入、查找和删除哈希表条目

#ifndef GUOSHUBASE_PF_HASHTABLE_H
#define GUOSHUBASE_PF_HASHTABLE_H

#include "pf_internal.h"

struct PF_HashEntry {
    PF_HashEntry *next;
    PF_HashEntry *prev;
    int fd;      // file descriptor 文件描述符，表示页面属于哪个文件
    PageNum pageNum; // 页面编号，表示该条目对应的页面
    int slot;    // 页面在缓冲池中的槽位，标记该页面所在的缓存位置
};

class PF_HashTable {
public:
    PF_HashTable(int numBuckets);           // 构造器，numBuckets指定哈希表的桶数
    ~PF_HashTable();                         // 析构
    RC Find(int fd, PageNum pageNum, int &slot);

    // 通过 fd 和 pageNum
    // 查找哈希表条目的槽位信息
    RC Insert(int fd, PageNum pageNum, int slot);

    // 插入条目
    RC Delete(int fd, PageNum pageNum);  // 删除条目

private:
    int Hash(int fd, PageNum pageNum) const { return ((fd + pageNum) % numBuckets); }   // 内部哈希函数
    int numBuckets;                               // 桶数
    PF_HashEntry **hashTable;                     // 指向哈希表的指针，存储多个 PF_HashEntry 的指针数组
};

#endif //GUOSHUBASE_PF_HASHTABLE_H
