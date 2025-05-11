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

PF_HashTable::PF_HashTable(int _numBuckets) : numBuckets(_numBuckets) {
    hashTable = new std::list<std::pair<PF_HashKey, int>>[numBuckets];
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
    PF_HashKey key{fd, pageNum};
    for (const auto &entry : hashTable[bucket]) {
        if (entry.first == key) {
            slot = entry.second;
            return 0;
        }
    }
    return PF_HASHNOTFOUND;
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
    PF_HashKey key{fd, pageNum};
    // 检查是否已存在
    for (const auto &entry : hashTable[bucket]) {
        if (entry.first == key) {
            return PF_HASHPAGEEXIST;
        }
    }
    // 插入到链表头部
    hashTable[bucket].emplace_front(key, slot);
    return 0;
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
    PF_HashKey key{fd, pageNum};
    hashTable[bucket].remove_if([&key](const auto &entry) {
        return entry.first == key;
    });
    return 0;
}



