//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/13 18:16
// @Project: GuoShuBase
//
//

#ifndef GUOSHUBASE_RM_RID_H
#define GUOSHUBASE_RM_RID_H
//
// rm_rid.h
//
//   The Record Id interface
//

// We separate the interface of RID from the rest of RM because some
// components will require the use of RID but not the rest of RM.

#include "guoshubase.h"

//
// PageNum: uniquely identifies a page in a file
//
typedef int PageNum;

//
// SlotNum: uniquely identifies a record in a page
//
typedef int SlotNum;

//
// RID: Record id interface
//
class RID {
public:
    RID();                                         // Default constructor
    RID(PageNum pageNum, SlotNum slotNum);
    ~RID();                                        // Destructor

    RC GetPageNum(PageNum &pageNum) const;         // Return page number
    RC GetSlotNum(SlotNum &slotNum) const;         // Return slot number

private:
};
#endif //GUOSHUBASE_RM_RID_H
