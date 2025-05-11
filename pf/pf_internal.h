//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/13 18:20
// @Project: GuoShuBase
//
// 这是PF组件内部声明的定义头，任何其他PF组件的实现只需要include这个头即可
// “那我问你，为什么不把这些定义和pf.h写在一起呢？”
// 答案是，pf.h定义了很多外部类，负责被其他组件（例如RM）调用，
// 一旦需要修改参数，就要重新编译所有使用pf.h的文件，
// 而本文件的数据只供PF组件内部使用，修改这些参数时只用重新编译这个头即可，能避免很多不必要的麻烦
// 因此，写在这个头中的所有定义都应该只被PF组件内部使用，而这样的参数也都应该写在这里

#ifndef PF_INTERNAL_H
#define PF_INTERNAL_H

#include <cstdlib>
#include <string>
#include "pf.h"

const int PF_BUFFER_SIZE = 40;     // Number of pages in the buffer
const int PF_HASH_TBL_SIZE = 20;   // Size of hash table

#define CREATION_MASK      0600
#define PF_PAGE_LIST_END  -1       // end of list of free pages
#define PF_PAGE_USED      -2       // page is being used

#ifndef SEEK_SET
#define SEEK_SET              0
#endif

struct PF_PageHdr {
    int nextFree;       // 可以是如下值:
    //  - 后续空闲页面数
    //  - 当是最后一页空闲时取值 PF_PAGE_LIST_END (-1)
    //  - 当没有后续空闲页时取值 PF_PAGE_USED (-2)
};

// 调整文件头使之与页边界对齐
// 如果不太懂，去看看pf.h 中对PF_PAGE_SIZE的定义原文，那里提到了PF_PageHdr的事
const int PF_FILE_HDR_SIZE = PF_PAGE_SIZE + sizeof(PF_PageHdr);

#endif //PF_INTERNAL_H
