#include <unistd.h>
#include <sys/types.h>
#include "pf_internal.h"
#include "pf_buffermgr.h"
#include <iostream>
using namespace std;

//----------------------------------------------
// PF_FileHandle 构造函数
// 描述: 文件句柄对象的默认构造函数
//       文件对象提供对已打开文件的访问
//       用于分配、释放和获取页面
//       必须传递给PF_Manager::OpenFile()才能访问文件页面
//       应传递给PF_Manager::CloseFile()来关闭文件
//       包含指向文件表中文件数据的指针
//       将文件的UNIX文件描述符传递给缓冲区管理器以访问文件页面
//----------------------------------------------
PF_FileHandle::PF_FileHandle()
{
    // 初始化成员变量
    bFileOpen = FALSE;    // 文件未打开
    pBufferMgr = NULL;    // 缓冲区管理器指针为空
}

//----------------------------------------------
// PF_FileHandle 析构函数
// 描述: 销毁文件句柄对象
//       如果文件句柄引用已打开的文件，文件不会被关闭
//----------------------------------------------
PF_FileHandle::~PF_FileHandle()
{
    // 无需任何操作
}

//----------------------------------------------
// PF_FileHandle 拷贝构造函数
// 输入: fileHandle - 用于构造本对象的文件句柄
//----------------------------------------------
PF_FileHandle::PF_FileHandle(const PF_FileHandle &fileHandle)
{
    // 直接复制成员变量，不涉及内存分配
    this->pBufferMgr  = fileHandle.pBufferMgr;
    this->hdr         = fileHandle.hdr;
    this->bFileOpen   = fileHandle.bFileOpen;
    this->bHdrChanged = fileHandle.bHdrChanged;
    this->unixfd      = fileHandle.unixfd;
}

//----------------------------------------------
// operator= 赋值运算符重载
// 描述: 如果本文件句柄引用已打开文件，文件不会被关闭
// 输入: fileHandle - 要赋值的文件句柄对象
// 返回: 返回*this的引用
//----------------------------------------------
PF_FileHandle& PF_FileHandle::operator=(const PF_FileHandle &fileHandle)
{
    // 检查自赋值
    if (this != &fileHandle) {
        // 直接复制成员变量，不涉及内存分配
        this->pBufferMgr  = fileHandle.pBufferMgr;
        this->hdr         = fileHandle.hdr;
        this->bFileOpen   = fileHandle.bFileOpen;
        this->bHdrChanged = fileHandle.bHdrChanged;
        this->unixfd      = fileHandle.unixfd;
    }
    return (*this);
}

//----------------------------------------------
// 获取首页
// 描述: 获取文件的第一页
//       文件句柄必须引用已打开的文件
// 输出: pageHandle - 成为文件第一页的句柄
//       引用的页面在缓冲池中被固定
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::GetFirstPage(PF_PageHandle &pageHandle) const
{
    return (GetNextPage((PageNum)-1, pageHandle));
}

//----------------------------------------------
// 获取末页
// 描述: 获取文件的最后一页
//       文件句柄必须引用已打开的文件
// 输出: pageHandle - 成为文件最后一页的句柄
//       引用的页面在缓冲池中被固定
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::GetLastPage(PF_PageHandle &pageHandle) const
{
    return (GetPrevPage((PageNum)hdr.numPages, pageHandle));
}

//----------------------------------------------
// 获取下一页
// 描述: 获取当前页之后的下一页(有效页)
//       文件句柄必须引用已打开的文件
// 输入: current - 获取此页号之后的有效页
//       current可以引用已被释放的页面
// 输出: pageHandle - 成为文件下一页的句柄
//       引用的页面在缓冲池中被固定
// 返回: PF_EOF或其他PF返回码
//----------------------------------------------
RC PF_FileHandle::GetNextPage(PageNum current, PF_PageHandle &pageHandle) const
{
    int rc;  // 返回码

    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 验证页号(-1是可接受的)
    if (current != -1 && !IsValidPageNum(current))
        return (PF_INVALIDPAGE);

    // 扫描文件直到找到有效页
    for (current++; current < hdr.numPages; current++) {
        // 如果是有效页，返回成功
        if (!(rc = GetThisPage(current, pageHandle)))
            return (0);

        // 如果是意外错误，返回
        if (rc != PF_INVALIDPAGE)
            return (rc);
    }

    // 未找到有效页
    return (PF_EOF);
}

//----------------------------------------------
// 获取上一页
// 描述: 获取当前页之前的上一页(有效页)
//       文件句柄必须引用已打开的文件
// 输入: current - 获取此页号之前的有效页
//       current可以引用已被释放的页面
// 输出: pageHandle - 成为文件上一页的句柄
//       引用的页面在缓冲池中被固定
// 返回: PF_EOF或其他PF返回码
//----------------------------------------------
RC PF_FileHandle::GetPrevPage(PageNum current, PF_PageHandle &pageHandle) const
{
    int rc;  // 返回码

    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 验证页号(hdr.numPages是可接受的)
    if (current != hdr.numPages && !IsValidPageNum(current))
        return (PF_INVALIDPAGE);

    // 扫描文件直到找到有效页
    for (current--; current >= 0; current--) {
        // 如果是有效页，返回成功
        if (!(rc = GetThisPage(current, pageHandle)))
            return (0);

        // 如果是意外错误，返回
        if (rc != PF_INVALIDPAGE)
            return (rc);
    }

    // 未找到有效页
    return (PF_EOF);
}

//----------------------------------------------
// 获取指定页
// 描述: 获取文件中的特定页
//       文件句柄必须引用已打开的文件
// 输入: pageNum - 要获取的页号
// 输出: pageHandle - 成为该页的句柄
//       修改pageHandle中的局部变量
//       引用的页面在缓冲池中被固定
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const
{
    int rc;                // 返回码
    char *pPageBuf;        // 缓冲池中页面的地址

    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 验证页号
    if (!IsValidPageNum(pageNum))
        return (PF_INVALIDPAGE);

    // 从缓冲区管理器获取该页
    if ((rc = pBufferMgr->GetPage(unixfd, pageNum, &pPageBuf)))
        return (rc);

    // 如果是有效页，设置pageHandle并返回成功
    if (((PF_PageHdr*)pPageBuf)->nextFree == PF_PAGE_USED) {
        pageHandle.pageNum = pageNum;
        pageHandle.pPageData = pPageBuf + sizeof(PF_PageHdr);
        return (0);
    }

    // 如果是无效页，解固定并返回错误
    if ((rc = UnpinPage(pageNum)))
        return (rc);

    return (PF_INVALIDPAGE);
}

//----------------------------------------------
// 分配新页
// 描述: 在文件中分配新页(可能获取之前释放的页)
//       文件句柄必须引用已打开的文件
// 输出: pageHandle - 成为新分配页的句柄
//       修改pageHandle中的局部变量
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::AllocatePage(PF_PageHandle &pageHandle)
{
    int rc;                // 返回码
    int pageNum;           // 新页号
    char *pPageBuf;        // 缓冲池中页面的地址

    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 如果空闲列表不为空...
    if (hdr.firstFree != PF_PAGE_LIST_END) {
        pageNum = hdr.firstFree;

        // 获取第一个空闲页到缓冲区
        if ((rc = pBufferMgr->GetPage(unixfd, pageNum, &pPageBuf)))
            return (rc);

        // 设置第一个空闲页为列表中下一页
        hdr.firstFree = ((PF_PageHdr*)pPageBuf)->nextFree;
    }
    else {
        // 空闲列表为空...
        pageNum = hdr.numPages;

        // 在文件中分配新页
        if ((rc = pBufferMgr->AllocatePage(unixfd, pageNum, &pPageBuf)))
            return (rc);

        // 增加文件页数
        hdr.numPages++;
    }

    // 标记文件头已修改
    bHdrChanged = TRUE;

    // 标记该页为已使用
    ((PF_PageHdr *)pPageBuf)->nextFree = PF_PAGE_USED;

    // 清空页面数据
    memset(pPageBuf + sizeof(PF_PageHdr), 0, PF_PAGE_SIZE);

    // 标记页面为脏(因为修改了next指针)
    if ((rc = MarkDirty(pageNum)))
        return (rc);

    // 设置pageHandle
    pageHandle.pageNum = pageNum;
    pageHandle.pPageData = pPageBuf + sizeof(PF_PageHdr);

    return (0);
}

//----------------------------------------------
// 释放页面
// 描述: 释放一个页面
//       文件句柄必须引用已打开的文件
//       引用此页的PF_PageHandle对象在此调用后不应再使用
// 输入: pageNum - 要释放的页号
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::DisposePage(PageNum pageNum)
{
    int rc;                // 返回码
    char *pPageBuf;        // 缓冲池中页面的地址

    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 验证页号
    if (!IsValidPageNum(pageNum))
        return (PF_INVALIDPAGE);

    // 获取页面(如果已固定则不重新固定)
    if ((rc = pBufferMgr->GetPage(unixfd, pageNum, &pPageBuf, FALSE)))
        return (rc);

    // 页面必须是有效的(已使用)
    if (((PF_PageHdr *)pPageBuf)->nextFree != PF_PAGE_USED) {
        // 解固定页面
        if ((rc = UnpinPage(pageNum)))
            return (rc);
        return (PF_PAGEFREE);  // 页面已空闲
    }

    // 将此页放入空闲列表
    ((PF_PageHdr *)pPageBuf)->nextFree = hdr.firstFree;
    hdr.firstFree = pageNum;
    bHdrChanged = TRUE;

    // 标记页面为脏(因为修改了next指针)
    if ((rc = MarkDirty(pageNum)))
        return (rc);

    // 解固定页面
    if ((rc = UnpinPage(pageNum)))
        return (rc);

    return (0);
}

//----------------------------------------------
// 标记页面为脏
// 描述: 将页面标记为脏
//       页面在从页面缓冲区移除时将被写回磁盘
//       文件句柄必须引用已打开的文件
// 输入: pageNum - 要标记为脏的页号
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::MarkDirty(PageNum pageNum) const
{
    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 验证页号
    if (!IsValidPageNum(pageNum))
        return (PF_INVALIDPAGE);

    // 通知缓冲区管理器标记页面为脏
    return (pBufferMgr->MarkDirty(unixfd, pageNum));
}

//----------------------------------------------
// 解固定页面
// 描述: 从缓冲区管理器解固定页面
//       页面可以被写回磁盘
//       引用此页的PF_PageHandle对象在此调用后不应再使用
//       文件句柄必须引用已打开的文件
// 输入: pageNum - 要解固定的页号
// 返回: PF返回码
//----------------------------------------------
RC PF_FileHandle::UnpinPage(PageNum pageNum) const
{
    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 验证页号
    if (!IsValidPageNum(pageNum))
        return (PF_INVALIDPAGE);

    // 通知缓冲区管理器解固定页面
    return (pBufferMgr->UnpinPage(unixfd, pageNum));
}

//----------------------------------------------
// 刷新页面
// 描述: 刷新此文件所有脏的未固定页面
// 输入: 无
// 返回: 如果页面被固定则返回PF_PAGEFIXED警告或其他PF错误
//----------------------------------------------
RC PF_FileHandle::FlushPages() const
{
    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 如果文件头已修改，写回文件
    if (bHdrChanged) {
        // 定位到文件开头
        if (lseek(unixfd, 0, L_SET) < 0)
            return (PF_UNIX);

        // 写入文件头
        int numBytes = write(unixfd, (char *)&hdr, sizeof(PF_FileHdr));
        if (numBytes < 0)
            return (PF_UNIX);
        if (numBytes != sizeof(PF_FileHdr))
            return (PF_HDRWRITE);

        // 去除const属性修改bHdrChanged
        PF_FileHandle *dummy = (PF_FileHandle *)this;
        dummy->bHdrChanged = FALSE;
    }

    // 通知缓冲区管理器刷新页面
    return (pBufferMgr->FlushPages(unixfd));
}

//----------------------------------------------
// 强制写回页面
// 描述: 如果页面是脏的，强制将其从缓冲池写入磁盘
//       页面不会从缓冲池中移除
// 输入: 页号，默认ALL_PAGES将强制所有页面
// 返回: 标准PF错误
//----------------------------------------------
RC PF_FileHandle::ForcePages(PageNum pageNum) const
{
    // 文件必须已打开
    if (!bFileOpen)
        return (PF_CLOSEDFILE);

    // 如果文件头已修改，写回文件
    if (bHdrChanged) {
        // 定位到文件开头
        if (lseek(unixfd, 0, L_SET) < 0)
            return (PF_UNIX);

        // 写入文件头
        int numBytes = write(unixfd, (char *)&hdr, sizeof(PF_FileHdr));
        if (numBytes < 0)
            return (PF_UNIX);
        if (numBytes != sizeof(PF_FileHdr))
            return (PF_HDRWRITE);

        // 去除const属性修改bHdrChanged
        PF_FileHandle *dummy = (PF_FileHandle *)this;
        dummy->bHdrChanged = FALSE;
    }

    // 通知缓冲区管理器强制写回页面
    return (pBufferMgr->ForcePages(unixfd, pageNum));
}

//----------------------------------------------
// 验证页号有效性(内部方法)
// 描述: 如果pageNum是文件中的有效页号返回TRUE，否则FALSE
// 输入: pageNum - 要测试的页号
// 返回: TRUE或FALSE
//----------------------------------------------
int PF_FileHandle::IsValidPageNum(PageNum pageNum) const
{
    return (bFileOpen && pageNum >= 0 && pageNum < hdr.numPages);
}
