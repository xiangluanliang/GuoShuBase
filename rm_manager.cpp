//
// Created by lenovo on 2025/4/17.
//
#include"rm.h"
#include"rm_rid.h"
#include"pf.h"


RM_Manager::RM_Manager(PF_Manager  &pf_manager) {
    this->pBufferMgr = pf_manager.GetBufferMgr(); // 获取 PF_BufferMgr 实例

}
RM_Manager::~RM_Manager()
{
    delete pBufferMgr; // 释放 PF_BufferMgr 实例

}

RC CreateFile(const char *fileName, int recordSize) {
    PF_Manager &pf_manager = PF_Manager::Instance(); // 获取 PF_Manager 实例
    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;
    char *data;
    int pageSize = pf_manager.GetPageSize(); // 获取页面大小

    // 检查 recordSize 是否大于 pageSize
    if (recordSize > pageSize) {
        return RM_RECORD_SIZE_TOO_LARGE; // 返回一个非零错误代码
    }

    // 创建文件
    RC rc = pf_manager.CreateFile(fileName);
    if (rc != 0) {
        return rc; // 如果文件创建失败，返回错误代码
    }

    // 打开文件
    rc = pf_manager.OpenFile(fileName, fileHandle);
    if (rc != 0) {
        return rc; // 如果文件打开失败，返回错误代码
    }

    // 获取第一页（头页）
    rc = fileHandle.GetFirstPage(pageHandle);
    if (rc != 0) {
        return rc; // 如果获取第一页失败，返回错误代码
    }

    // 获取页面数据指针
    rc = pageHandle.GetData(data);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 初始化 RM 文件头
    RM_FileHeader fileHeader;
    fileHeader.recordSize = recordSize;
    fileHeader.pageSize = pageSize;
    fileHeader.numRecordsPerPage = pageSize / recordSize;
    fileHeader.firstFreePage = PF_PAGE_LIST_END;
    fileHeader.firstFreeSlot = RM_SLOT_LIST_END;
    fileHeader.numPages = 1;

    // 将文件头写入头页
    memcpy(data, &fileHeader, sizeof(RM_FileHeader));

    // 标记第一页为脏页，以便后续写入磁盘
    rc = fileHandle.MarkDirty(PF_PAGE_LIST_END);
    if (rc != 0) {
        return rc; // 如果标记脏页失败，返回错误代码
    }

    // 强制将第一页写入磁盘
    rc = fileHandle.ForcePage(PF_PAGE_LIST_END);
    if (rc != 0) {
        return rc; // 如果强制写入磁盘失败，返回错误代码
    }

    // 关闭文件
    rc = pf_manager.CloseFile(fileHandle);
    if (rc != 0) {
        return rc; // 如果关闭文件失败，返回错误代码
    }

    return 0; // 成功
}

RC DestroyFile(const char *fileName) {
    PF_Manager &pf_manager = PF_Manager::Instance(); // 获取 PF_Manager 实例

    // 调用 PF_Manager 的 DestroyFile 方法来销毁文件
    RC rc = pf_manager.DestroyFile(fileName);
    if (rc != 0) {
        return rc; // 如果销毁文件失败，返回错误代码
    }

    return 0; // 成功
}

RC OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    PF_Manager &pf_manager = PF_Manager::Instance(); // 获取 PF_Manager 实例
    PF_FileHandle pfFileHandle;

    // 打开文件
    RC rc = pf_manager.OpenFile(fileName, pfFileHandle);
    if (rc != 0) {
        return rc; // 如果打开文件失败，返回错误代码
    }

    // 初始化 RM_FileHandle
    fileHandle.pfFileHandle = pfFileHandle;
    fileHandle.bFileOpen = true;

    // 获取头页
    PF_PageHandle pageHandle;
    rc = pfFileHandle.GetFirstPage(pageHandle);
    if (rc != 0) {
        return rc; // 如果获取头页失败，返回错误代码
    }

    // 获取页面数据指针
    char *data;
    rc = pageHandle.GetData(data);
    if (rc != 0) {
        return rc; // 如果获取数据指针失败，返回错误代码
    }

    // 从头页读取文件头信息
    memcpy(&fileHandle.fileHeader, data, sizeof(RM_FileHeader));

    // 关闭头页
    rc = pfFileHandle.UnpinPage(PF_PAGE_LIST_END);
    if (rc != 0) {
        return rc; // 如果取消固定头页失败，返回错误代码
    }

    return 0; // 成功
}


RC CloseFile(RM_FileHandle &fileHandle) {
    PF_Manager &pf_manager = PF_Manager::Instance(); // 获取 PF_Manager 实例

    // 检查文件是否已打开
    if (!fileHandle.bFileOpen) {
        return RM_FILE_NOT_OPEN; // 如果文件未打开，返回错误代码
    }

    // 关闭文件
    RC rc = pf_manager.CloseFile(fileHandle.pfFileHandle);
    if (rc != 0) {
        return rc; // 如果关闭文件失败，返回错误代码
    }

    // 重置 RM_FileHandle 对象的状态
    fileHandle.bFileOpen = false;
    fileHandle.pfFileHandle = PF_FileHandle();

    return 0; // 成功
}
