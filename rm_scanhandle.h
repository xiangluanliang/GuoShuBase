//
// Created by lenovo on 2025/4/18.
//

#ifndef GUOSHUBASE_RM_SCANHANDLE_H
#define GUOSHUBASE_RM_SCANHANDLE_H

#ifndef RM_SCAN_H
#define RM_SCAN_H

#include "pf.h"
#include "rm.h"

class RM_FileHandle;

class RM_ScanHandle {
public:
    RM_ScanHandle();
    ~RM_ScanHandle();

    RC GetNextRec(RM_Record &rec) const;

private:
    friend class RM_FileHandle;

    const RM_FileHandle *rmFileHandle;
    AttrType attrType;
    int attrLength;
    int attrOffset;
    CompOp compOp;
    const void *value;

    mutable PageNum currentPageNum;
    mutable SlotNum currentSlotNum;

    mutable int valueInt;
    mutable float valueFloat;
    mutable char *valueString;
};

#endif // RM_SCAN_H



#endif //GUOSHUBASE_RM_SCANHANDLE_H
