//
// Created by lenovo on 2025/4/17.
//

#include"rm.h"
#include"rm_rid.h"
#include"pf.h"
#include <unistd.h>
#include <sys/types.h>
#include "pf_internal.h"
#include "pf_buffermgr.h"

RM_FileHandle::RM_FileHandle  (){

}
RM_FileHandle::~RM_FileHandle () {
    //DO NOTHING

}
RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
    // 检查文件是否已打开
    if (!bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 获取页面号和槽号
    PageNum pageNum = rid.pageNum;
    SlotNum slotNum = rid.slotNum;

    // 检查页面号是否有效
    if (pageNum >= fileHeader.numPages) {
        return RM_INVALID_PAGE_NUM; // 如果页面号无效，返回错误代码
    }

    // 检查槽号是否有效
    if (slotNum >= fileHeader.numRecordsPerPage) {
        return RM_INVALID_SLOT_NUM; // 如果槽号无效，返回错误代码
    }

    // 固定页面
    PF_PageHandle pageHandle;
    RC rc = pfFileHandle.GetThisPage(pageNum, pageHandle);
    if (rc != 0) {
        return rc; // 如果固定页面失败，返回错误代码
    }

    // 获取页面数据指针
    char *data;
    rc = pageHandle.GetData(data);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 计算记录的偏移量
    int recordOffset = slotNum * fileHeader.recordSize;

    // 检查该槽是否为空
    if (data[recordOffset] == RM_RECORD_DELETED) {
        return RM_RECORD_NOT_FOUND; // 如果记录已删除，返回错误代码
    }

    // 复制记录数据到 RM_Record 对象
    rec.data = data + recordOffset;
    rec.rid = rid;
    rec.rmFileHandle = const_cast<RM_FileHandle*>(this);

    // 取消固定页面
    rc = pfFileHandle.UnpinPage(pageNum);
    if (rc != 0) {
        return rc; // 如果取消固定页面失败，返回错误代码
    }

    return 0; // 成功
}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid) {
    // 检查文件是否已打开
    if (!bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 检查 pData 是否为空
    if (pData == nullptr) {
        return RM_NULL_DATA; // 如果数据为空，返回错误代码
    }

    // 获取头页
    PF_PageHandle pageHandle;
    RC rc = pfFileHandle.GetThisPage(0, pageHandle);
    if (rc != 0) {
        return rc; // 如果获取头页失败，返回错误代码
    }

    // 获取页面数据指针
    char *headerData;
    rc = pageHandle.GetData(headerData);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 读取文件头信息
    RM_FileHeader fileHeader;
    memcpy(&fileHeader, headerData, sizeof(RM_FileHeader));

    // 取消固定头页
    rc = pfFileHandle.UnpinPage(0);
    if (rc != 0) {
        return rc; // 如果取消固定头页失败，返回错误代码
    }

    // 尝试在现有页面中找到空闲槽
    PageNum pageNum = fileHeader.firstFreePage;
    SlotNum slotNum = RM_SLOT_LIST_END;

    while (pageNum != PF_PAGE_LIST_END) {
        rc = pfFileHandle.GetThisPage(pageNum, pageHandle);
        if (rc != 0) {
            return rc; // 如果固定页面失败，返回错误代码
        }

        // 获取页面数据指针
        rc = pageHandle.GetData(headerData);
        if (rc != 0) {
            return rc; // 如果获取数据指针失败，返回错误代码
        }

        // 找到第一个空闲槽
        for (slotNum = 0; slotNum < fileHeader.numRecordsPerPage; slotNum++) {
            if (headerData[slotNum * fileHeader.recordSize] == RM_RECORD_DELETED) {
                break; // 找到空闲槽
            }
        }

        // 如果找到了空闲槽，跳出循环
        if (slotNum < fileHeader.numRecordsPerPage) {
            break;
        }

        // 如果当前页面没有空闲槽，获取下一个页面
        rc = pageHandle.GetNextPageNum(pageNum);
        if (rc != 0) {
            return rc; // 如果获取下一个页面号失败，返回错误代码
        }

        // 取消固定当前页面
        rc = pfFileHandle.UnpinPage(pageNum);
        if (rc != 0) {
            return rc; // 如果取消固定页面失败，返回错误代码
        }
    }

    // 如果没有找到空闲槽，创建新页面
    if (slotNum == fileHeader.numRecordsPerPage) {
        pageNum = fileHeader.numPages;
        rc = pfFileHandle.AllocatePage(pageHandle);
        if (rc != 0) {
            return rc; // 如果分配页面失败，返回错误代码
        }

        // 获取页面数据指针
        rc = pageHandle.GetData(headerData);
        if (rc != 0) {
            return rc; // 如果获取数据指针失败，返回错误代码
        }

        // 初始化新页面的槽位
        for (slotNum = 0; slotNum < fileHeader.numRecordsPerPage; slotNum++) {
            headerData[slotNum * fileHeader.recordSize] = RM_RECORD_AVAILABLE;
        }

        // 更新文件头信息
        fileHeader.numPages++;
        fileHeader.firstFreePage = pageNum;

        // 固定头页
        rc = pfFileHandle.GetThisPage(0, pageHandle);
        if (rc != 0) {
            return rc; // 如果固定头页失败，返回错误代码
        }

        // 获取页面数据指针
        rc = pageHandle.GetData(headerData);
        if (rc != 0) {
            return rc; // 如果获取数据指针失败，返回错误代码
        }

        // 更新头页中的文件头信息
        memcpy(headerData, &fileHeader, sizeof(RM_FileHeader));

        // 标记头页为脏页
        rc = pfFileHandle.MarkDirty(0);
        if (rc != 0) {
            return rc; // 如果标记脏页失败，返回错误代码
        }

        // 取消固定头页
        rc = pfFileHandle.UnpinPage(0);
        if (rc != 0) {
            return rc; // 如果取消固定头页失败，返回错误代码
        }

        // 设置第一个空闲槽
        slotNum = 0;
    }

    // 固定新页面
    rc = pfFileHandle.GetThisPage(pageNum, pageHandle);
    if (rc != 0) {
        return rc; // 如果固定页面失败，返回错误代码
    }

    // 获取页面数据指针
    rc = pageHandle.GetData(headerData);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 计算记录的偏移量
    int recordOffset = slotNum * fileHeader.recordSize;

    // 将数据插入到页面中
    memcpy(headerData + recordOffset, pData, fileHeader.recordSize);

    // 设置记录标识符
    rid.pageNum = pageNum;
    rid.slotNum = slotNum;

    // 标记新页面为脏页
    rc = pfFileHandle.MarkDirty(pageNum);
    if (rc != 0) {
        return rc; // 如果标记脏页失败，返回错误代码
    }

    // 取消固定页面
    rc = pfFileHandle.UnpinPage(pageNum);
    if (rc != 0) {
        return rc; // 如果取消固定页面失败，返回错误代码
    }

    return 0; // 成功
}

RC RM_FileHandle::DeleteRec(const RID &rid) {
    // 检查文件是否已打开
    if (!bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 获取页面号和槽号
    PageNum pageNum = rid.pageNum;
    SlotNum slotNum = rid.slotNum;

    // 检查页面号是否有效
    if (pageNum >= fileHeader.numPages) {
        return RM_INVALID_PAGE_NUM; // 如果页面号无效，返回错误代码
    }

    // 检查槽号是否有效
    if (slotNum >= fileHeader.numRecordsPerPage) {
        return RM_INVALID_SLOT_NUM; // 如果槽号无效，返回错误代码
    }

    // 固定页面
    PF_PageHandle pageHandle;
    RC rc = pfFileHandle.GetThisPage(pageNum, pageHandle);
    if (rc != 0) {
        return rc; // 如果固定页面失败，返回错误代码
    }

    // 获取页面数据指针
    char *data;
    rc = pageHandle.GetData(data);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 计算记录的偏移量
    int recordOffset = slotNum * fileHeader.recordSize;

    // 检查该槽是否为空
    if (data[recordOffset] == RM_RECORD_DELETED) {
        return RM_RECORD_NOT_FOUND; // 如果记录已删除，返回错误代码
    }

    // 标记记录为已删除
    data[recordOffset] = RM_RECORD_DELETED;

    // 标记页面为脏页
    rc = pfFileHandle.MarkDirty(pageNum);
    if (rc != 0) {
        return rc; // 如果标记脏页失败，返回错误代码
    }

    // 取消固定页面
    rc = pfFileHandle.UnpinPage(pageNum);
    if (rc != 0) {
        return rc; // 如果取消固定页面失败，返回错误代码
    }

    // 检查页面是否为空
    bool isEmpty = true;
    for (int i = 0; i < fileHeader.numRecordsPerPage; i++) {
        if (data[i * fileHeader.recordSize] != RM_RECORD_DELETED) {
            isEmpty = false;
            break;
        }
    }

    // 如果页面为空，可以选择丢弃页面
    if (isEmpty) {
        rc = pfFileHandle.DisposePage(pageNum);
        if (rc != 0) {
            return rc; // 如果丢弃页面失败，返回错误代码
        }

        // 更新文件头信息
        rc = pfFileHandle.GetThisPage(0, pageHandle);
        if (rc != 0) {
            return rc; // 如果固定头页失败，返回错误代码
        }

        rc = pageHandle.GetData(headerData);
        if (rc != 0) {
            return rc; // 如果获取数据指针失败，返回错误代码
        }

        // 读取文件头信息
        memcpy(&fileHeader, headerData, sizeof(RM_FileHeader));

        // 更新第一个空闲页面
        if (fileHeader.firstFreePage == pageNum) {
            fileHeader.firstFreePage = PF_PAGE_LIST_END;
        }

        // 将当前页面标记为第一个空闲页面
        fileHeader.firstFreePage = pageNum;

        // 更新头页中的文件头信息
        memcpy(headerData, &fileHeader, sizeof(RM_FileHeader));

        // 标记头页为脏页
        rc = pfFileHandle.MarkDirty(0);
        if (rc != 0) {
            return rc; // 如果标记脏页失败，返回错误代码
        }

        // 取消固定头页
        rc = pfFileHandle.UnpinPage(0);
        if (rc != 0) {
            return rc; // 如果取消固定头页失败，返回错误代码
        }
    }

    return 0; // 成功
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
    // 检查文件是否已打开
    if (!bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 获取记录的页面号和槽号
    PageNum pageNum = rec.rid.pageNum;
    SlotNum slotNum = rec.rid.slotNum;

    // 检查页面号是否有效
    if (pageNum >= fileHeader.numPages) {
        return RM_INVALID_PAGE_NUM; // 如果页面号无效，返回错误代码
    }

    // 检查槽号是否有效
    if (slotNum >= fileHeader.numRecordsPerPage) {
        return RM_INVALID_SLOT_NUM; // 如果槽号无效，返回错误代码
    }

    // 固定页面
    PF_PageHandle pageHandle;
    RC rc = pfFileHandle.GetThisPage(pageNum, pageHandle);
    if (rc != 0) {
        return rc; // 如果固定页面失败，返回错误代码
    }

    // 获取页面数据指针
    char *data;
    rc = pageHandle.GetData(data);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 计算记录的偏移量
    int recordOffset = slotNum * fileHeader.recordSize;

    // 检查该槽是否为空
    if (data[recordOffset] == RM_RECORD_DELETED) {
        return RM_RECORD_NOT_FOUND; // 如果记录已删除，返回错误代码
    }

    // 获取记录数据指针
    const char *recData = rec.data;

    // 将记录数据更新到页面中
    memcpy(data + recordOffset, recData, fileHeader.recordSize);

    // 标记页面为脏页
    rc = pfFileHandle.MarkDirty(pageNum);
    if (rc != 0) {
        return rc; // 如果标记脏页失败，返回错误代码
    }

    // 取消固定页面
    rc = pfFileHandle.UnpinPage(pageNum);
    if (rc != 0) {
        return rc; // 如果取消固定页面失败，返回错误代码
    }

    return 0; // 成功
}


RC RM_FileHandle::ForcePages(PageNum pageNum) const {
    // 检查文件是否已打开
    if (!bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 调用 PF_FileHandle 的 ForcePages 方法
    RC rc = pfFileHandle.ForcePages(pageNum);
    if (rc != 0) {
        return rc; // 如果 ForcePages 失败，返回错误代码
    }

    return 0; // 成功
}
