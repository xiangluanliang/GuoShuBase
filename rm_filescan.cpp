//
// Created by lenovo on 2025/4/17.
//
#include"rm.h"
#include"rm_rid.h "
#include"pf.h"
#include "pf_internal.h"
#include "pf_buffermgr.h"
#include "rm_filehandle.h"
#include "rm_scan.h"

#




#define RM_EOF 1
RM_FileScan :: RM_FileScan() {

}

RM_FileScan :: ~RM_FileScan() {

}


RC RC_OpenScan(const RM_FileHandle &fileHandle, AttrType attrType, int attrLength, int attrOffset, CompOp compOp, void *value, ClientHint pinHint) {
    // 检查文件是否已打开
    if (!fileHandle.bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 检查参数的有效性
    if (attrOffset < 0 || attrOffset + attrLength > fileHandle.fileHeader.recordSize) {
        return RM_INVALID_ATTR_OFFSET; // 如果属性偏移量无效，返回错误代码
    }

    // 检查属性长度是否有效
    if (attrLength < 1 || (attrType == STRING && attrLength > MAXSTRINGLEN)) {
        return RM_INVALID_ATTR_LENGTH; // 如果属性长度无效，返回错误代码
    }

    // 检查比较操作符的有效性
    if (compOp < NO_OP || compOp > NE_OP) {
        return RM_INVALID_COMP_OP; // 如果比较操作符无效，返回错误代码
    }

    // 检查 value 的有效性
    if (value != nullptr && attrType == STRING && attrLength != strlen(value)) {
        return RM_INVALID_STR_LENGTH; // 如果字符串长度不匹配，返回错误代码
    }

    // 创建一个新的 RM_ScanHandle 对象
    RM_ScanHandle *scanHandle = new RM_ScanHandle();

    // 初始化 RM_ScanHandle 对象
    scanHandle->rmFileHandle = &fileHandle;
    scanHandle->attrType = attrType;
    scanHandle->attrLength = attrLength;
    scanHandle->attrOffset = attrOffset;
    scanHandle->compOp = compOp;
    scanHandle->value = value;

    // 根据 attrType 分配和复制 value
    if (value != nullptr) {
        if (attrType == INT) {
            scanHandle->valueInt = *static_cast<int*>(value);
        } else if (attrType == FLOAT) {
            scanHandle->valueFloat = *static_cast<float*>(value);
        } else if (attrType == STRING) {
            scanHandle->valueString = new char[attrLength];
            memcpy(scanHandle->valueString, value, attrLength);
        } else {
            return RM_INVALID_ATTR_TYPE; // 如果属性类型无效，返回错误代码
        }
    } else {
        // 如果没有条件，设置 value 为默认值
        scanHandle->valueInt = 0;
        scanHandle->valueFloat = 0.0f;
        scanHandle->valueString = nullptr;
    }

    // 初始化扫描的起始页面和槽号
    scanHandle->currentPageNum = 0;
    scanHandle->currentSlotNum = 0;

    // 返回扫描句柄
    return (RC)scanHandle;
}


RC RM_ScanHandle::GetNextRec(RM_Record &rec) const {
    // 检查文件是否已打开
    if (rmFileHandle == nullptr || !rmFileHandle->bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    PF_PageHandle pageHandle;
    char *data;
    RC rc;

    // 当前页面和槽的偏移量
    PageNum currentPageNum = this->currentPageNum;
    SlotNum currentSlotNum = this->currentSlotNum;

    // 遍历文件中的每个页面
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

        // 遍历当前页面中的每个槽
        for (; currentSlotNum < rmFileHandle->fileHeader.numRecordsPerPage; currentSlotNum++) {
            int recordOffset = currentSlotNum * rmFileHandle->fileHeader.recordSize;

            // 检查该槽是否为空（已被删除）
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

                    // 更新扫描状态
                    this->currentPageNum = currentPageNum;
                    this->currentSlotNum = currentSlotNum + 1;

                    return 0; // 成功
                }
            }
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

    // 如果没有找到符合条件的记录，返回 RM_EOF
    return RM_EOF;
}
RC RM_ScanHandle::GetRid(RID &rid) const {
    // 检查是否有当前记录
    if (currentPageNum >= rmFileHandle->fileHeader.numPages) {
        return RM_EOF; // 如果没有当前记录，返回 RM_EOF
    }

    // 设置 RID
    rid = RID(currentPageNum, currentSlotNum - 1);

    return 0; // 成功
}