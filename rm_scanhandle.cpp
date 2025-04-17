//
// Created by lenovo on 2025/4/18.
//

#include "rm_scanhandle.h"
#include "pf.h"
#include "rm.h"
#include "pf_internal.h"
#include "rm.h"
#include "rm_filehandle.h"
#include "rm_scan.h"

RM_ScanHandle::RM_ScanHandle(){

}

RM_ScanHandle::~RM_ScanHandle(){

}

RC RM_ScanHandle::GetNextRec(RM_Record &rec) const {
    // 检查文件是否已打开
    if (rmFileHandle == nullptr || !rmFileHandle->bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    PF_PageHandle pageHandle;
    char *data;
    RC rc;

    while (currentPageNum < rmFileHandle->fileHeader.numPages) {
        // 固定当前页面
        rc = rmFileHandle->pfFileHandle.GetThisPage(currentPageNum, pageHandle);
        if (rc != 0) {
            return rc; // 如果固定页面失败，返回错误代码
        }

        // 获取页面数据指针
        rc = pageHandle.GetData(data);
        if (rc != 0) {
            return rc; // 如果获取数据指针失败，返回错误代码
        }

        // 检查当前槽号是否在有效范围内
        while (currentSlotNum < rmFileHandle->fileHeader.numRecordsPerPage) {
            int recordOffset = currentSlotNum * rmFileHandle->fileHeader.recordSize;

            // 检查该槽是否为空
            if (data[recordOffset] != RM_RECORD_DELETED) {
                // 获取记录数据指针
                const char *recordData = data + recordOffset;

                // 根据 compOp 进行比较
                bool match = false;

                switch (compOp) {
                    case NO_OP:
                        match = true;
                        break;
                    case EQ_OP:
                        if (attrType == INT) {
                            match = (*reinterpret_cast<const int*>(recordData + attrOffset) == valueInt);
                        } else if (attrType == FLOAT) {
                            match = (*reinterpret_cast<const float*>(recordData + attrOffset) == valueFloat);
                        } else if (attrType == STRING) {
                            match = (memcmp(recordData + attrOffset, valueString, attrLength) == 0);
                        }
                        break;
                    case LT_OP:
                        if (attrType == INT) {
                            match = (*reinterpret_cast<const int*>(recordData + attrOffset) < valueInt);
                        } else if (attrType == FLOAT) {
                            match = (*reinterpret_cast<const float*>(recordData + attrOffset) < valueFloat);
                        } else if (attrType == STRING) {
                            match = (memcmp(recordData + attrOffset, valueString, attrLength) < 0);
                        }
                        break;
                    case GT_OP:
                        if (attrType == INT) {
                            match = (*reinterpret_cast<const int*>(recordData + attrOffset) > valueInt);
                        } else if (attrType == FLOAT) {
                            match = (*reinterpret_cast<const float*>(recordData + attrOffset) > valueFloat);
                        } else if (attrType == STRING) {
                            match = (memcmp(recordData + attrOffset, valueString, attrLength) > 0);
                        }
                        break;
                    case LE_OP:
                        if (attrType == INT) {
                            match = (*reinterpret_cast<const int*>(recordData + attrOffset) <= valueInt);
                        } else if (attrType == FLOAT) {
                            match = (*reinterpret_cast<const float*>(recordData + attrOffset) <= valueFloat);
                        } else if (attrType == STRING) {
                            match = (memcmp(recordData + attrOffset, valueString, attrLength) <= 0);
                        }
                        break;
                    case GE_OP:
                        if (attrType == INT) {
                            match = (*reinterpret_cast<const int*>(recordData + attrOffset) >= valueInt);
                        } else if (attrType == FLOAT) {
                            match = (*reinterpret_cast<const float*>(recordData + attrOffset) >= valueFloat);
                        } else if (attrType == STRING) {
                            match = (memcmp(recordData + attrOffset, valueString, attrLength) >= 0);
                        }
                        break;
                    case NE_OP:
                        if (attrType == INT) {
                            match = (*reinterpret_cast<const int*>(recordData + attrOffset) != valueInt);
                        } else if (attrType == FLOAT) {
                            match = (*reinterpret_cast<const float*>(recordData + attrOffset) != valueFloat);
                        } else if (attrType == STRING) {
                            match = (memcmp(recordData + attrOffset, valueString, attrLength) != 0);
                        }
                        break;
                }

                if (match) {
                    // 设置 RM_Record 对象
                    rec.data = recordData;
                    rec.rid.pageNum = currentPageNum;
                    rec.rid.slotNum = currentSlotNum;
                    rec.rmFileHandle = const_cast<RM_FileHandle*>(rmFileHandle);

                    // 取消固定页面
                    rc = rmFileHandle->pfFileHandle.UnpinPage(currentPageNum);
                    if (rc != 0) {
                        return rc; // 如果取消固定页面失败，返回错误代码
                    }

                    // 移动到下一个槽
                    currentSlotNum++;
                    return 0; // 成功
                }
            }

            // 移动到下一个槽
            currentSlotNum++;
        }

        // 取消固定当前页面
        rc = rmFileHandle->pfFileHandle.UnpinPage(currentPageNum);
        if (rc != 0) {
            return rc; // 如果取消固定页面失败，返回错误代码
        }

        // 移动到下一页
        currentPageNum++;
        currentSlotNum = 0;
    }

    return RM_EOF; // 如果没有更多记录，返回 RM_EOF 表示文件结束
}
