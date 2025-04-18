#include "pf_internal.h"
//----------------------------------------------
// 常量定义
//----------------------------------------------
#define INVALID_PAGE   (-1)  // 无效页面标识

//----------------------------------------------
// PF_PageHandle 构造函数
// 描述: 页面句柄对象的默认构造函数
//       页面句柄对象提供对页面内容和页面号的访问
//       虽然在此构造页面句柄对象，但必须将其传递给PF_FileHandle的方法之一
//       才能使其引用已固定的页面，然后才能用于访问页面内容
//       注意：访问完成后记得调用PF_FileHandle::UnpinPage()解固定页面
//----------------------------------------------
PF_PageHandle::PF_PageHandle()
{
    pageNum = INVALID_PAGE;  // 初始化为无效页面号
    pPageData = NULL;        // 初始化为空指针
}

//----------------------------------------------
// PF_PageHandle 析构函数
// 描述: 销毁页面句柄对象
//       如果页面句柄对象引用已固定的页面，该页面不会被解固定
//----------------------------------------------
PF_PageHandle::~PF_PageHandle()
{
    // 无需任何操作
}

//----------------------------------------------
// PF_PageHandle 拷贝构造函数
// 描述: 拷贝构造函数
//       如果传入的页面句柄对象引用已固定的页面，该页面不会被再次固定
// 输入: pageHandle - 用于构造本对象的页面句柄对象
//----------------------------------------------
PF_PageHandle::PF_PageHandle(const PF_PageHandle &pageHandle)
{
    // 只需复制局部变量，不涉及内存分配
    this->pageNum = pageHandle.pageNum;
    this->pPageData = pageHandle.pPageData;
}

//----------------------------------------------
// operator= 赋值运算符重载
// 描述: 重载赋值运算符
//       如果右侧的页面句柄对象引用已固定的页面，该页面不会被再次固定
// 输入: pageHandle - 要赋值的页面句柄对象
// 返回: 返回*this的引用
//----------------------------------------------
PF_PageHandle& PF_PageHandle::operator= (const PF_PageHandle &pageHandle)
{
    // 检查自赋值情况
    if (this != &pageHandle) {
        // 只需复制指针，不涉及内存分配
        this->pageNum = pageHandle.pageNum;
        this->pPageData = pageHandle.pPageData;
    }

    // 返回本对象的引用
    return (*this);
}

//----------------------------------------------
// 获取页面数据
// 描述: 访问页面内容。页面句柄对象必须引用已固定的页面
// 输出: pData - 设置为指向页面内容的指针
// 返回: PF 返回码
//----------------------------------------------
RC PF_PageHandle::GetData(char *&pData) const
{

    // 页面必须引用已固定的页面
    if (pPageData == NULL)
        return (PF_PAGEUNPINNED);  // 页面未固定错误

    // 设置pData指向页面内容(PF头之后)
    pData = pPageData;

    // 返回成功
    return (0);
}

//----------------------------------------------
// 获取页面号
// 描述: 访问页面号。页面句柄对象必须引用已固定的页面
// 输出: _pageNum - 包含页面号
// 返回: PF 返回码
//----------------------------------------------
RC PF_PageHandle::GetPageNum(PageNum &_pageNum) const
{
    // 页面必须引用已固定的页面
    if (pPageData == NULL)
        return (PF_PAGEUNPINNED);  // 页面未固定错误

    // 设置页面号
    _pageNum = this->pageNum;

    // 返回成功
    return (0);
}
