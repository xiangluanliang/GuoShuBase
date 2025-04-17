//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/27 18:05
// @Project: GuoShuBase
//

#include "pf_internal.h"
#include "pf_hashtable.h"

//
// PF_HashTable
//
// 描述: PF_HashTable 对象的构造函数，支持查找、插入和删除哈希表条目。
// 输入: numBuckets - 哈希表桶的数量
//

PF_HashTable::PF_HashTable(int _numBuckets) {
    this->numBuckets = _numBuckets;
    hashTable = new PF_HashEntry *[numBuckets];
    for (int i = 0; i < numBuckets; i++)
        hashTable[i] = nullptr;
}
//
// ~PF_HashTable
//
// 描述: 析构函数
//
PF_HashTable::~PF_HashTable() {
    for (int i = 0; i < numBuckets; i++) {
        PF_HashEntry *entry = hashTable[i];
        while (entry != nullptr) {
            PF_HashEntry *next = entry->next;
            delete entry;
            entry = next;
        }
    }
    delete[] hashTable;
}

//
// Find
//
// 描述: 查找哈希表条目。
// 输入: fd - 文件描述符
//       pageNum - 页号
// 输出: slot - 设为与 fd 和 pageNum 关联的槽位
// 返回: PF 返回码
//
RC PF_HashTable::Find(int fd, PageNum pageNum, int &slot) {
    int bucket = Hash(fd, pageNum);
    if (bucket < 0)
        return (PF_HASHNOTFOUND);
    for (PF_HashEntry *entry = hashTable[bucket];
         entry != nullptr;
         entry = entry->next) {
        if (entry->fd == fd && entry->pageNum == pageNum) {
            slot = entry->slot;
            return (0);
        }
    }
    return (PF_HASHNOTFOUND);
}


//
// Insert
//
// 描述: 插入哈希表条目。
// 输入: fd - 文件描述符
//       pageNum - 页号
//       slot - 与 fd 和 pageNum 关联的槽位
// 返回: PF 返回码
//
RC PF_HashTable::Insert(int fd, PageNum pageNum, int slot) {
    int bucket = Hash(fd, pageNum);
    PF_HashEntry *entry;
    for (entry = hashTable[bucket];
         entry != nullptr;
         entry = entry->next) {
        if (entry->fd == fd && entry->pageNum == pageNum)
            return (PF_HASHPAGEEXIST);
    }
    if ((entry = new PF_HashEntry) == nullptr) //这种情况都要考虑到吗……
        return (PF_NOMEM);
    entry->fd = fd;
    entry->pageNum = pageNum;
    entry->slot = slot;
    entry->next = hashTable[bucket];
    entry->prev = nullptr;
    if (hashTable[bucket] != nullptr)
        hashTable[bucket]->prev = entry;
    hashTable[bucket] = entry;
    return (0);
}

//
// Delete
//
// 描述: 删除哈希表条目。
// 输入: fd - 文件描述符
//       pageNum - 页号
// 返回: PF 返回码
//
RC PF_HashTable::Delete(int fd, PageNum pageNum) {
    int bucket = Hash(fd, pageNum);
    PF_HashEntry *entry;
    for (entry = hashTable[bucket];
         entry != nullptr;
         entry = entry->next) {
        if (entry->fd == fd && entry->pageNum == pageNum)
            break;
    }
    if (entry == nullptr)
        return (PF_HASHNOTFOUND);
    if (entry == hashTable[bucket])
        hashTable[bucket] = entry->next;
    if (entry->prev != nullptr)
        entry->prev->next = entry->next;
    if (entry->next != nullptr)
        entry->next->prev = entry->prev;
    delete entry;
    return (0);
}


