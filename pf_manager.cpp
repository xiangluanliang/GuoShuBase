#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pf_internal.h"
#include "pf_buffermgr.h"
#include "pf.h"

//----------------------------------------------
// PF_Manager 构造函数
// 描述: 在程序开始时调用一次
//       负责文件的创建、删除、打开和关闭操作
//       关联一个管理页面缓冲区和执行页面替换策略的 PF_BufferMgr
//----------------------------------------------
PF_Manager::PF_Manager()
{
    // 创建缓冲区管理器
    pBufferMgr = new PF_BufferMgr(PF_BUFFER_SIZE);
}

//----------------------------------------------
// PF_Manager 析构函数
// 描述: 在程序结束时调用一次
//       销毁缓冲区管理器
//       调用此方法时所有文件应已关闭
//----------------------------------------------
PF_Manager::~PF_Manager()
{
    // 销毁缓冲区管理器对象
    delete pBufferMgr;
}

//----------------------------------------------
// 创建文件
// 描述: 创建名为 fileName 的新 PF 文件
// 输入: fileName - 要创建的文件名
// 返回: PF 返回码
//----------------------------------------------
RC PF_Manager::CreateFile(const char *fileName)
{
    int fd;            // UNIX 文件描述符
    int numBytes;      // 写入系统调用的返回码

    // 创建独占使用的文件
    if ((fd = open(fileName,
#ifdef PC
            O_BINARY |   // Windows平台需要二进制模式
#endif
                   O_CREAT | O_EXCL | O_WRONLY,  // 创建新文件，独占写入
                   CREATION_MASK)) < 0)          // 文件权限掩码
        return (PF_UNIX);  // 返回UNIX错误

    // 初始化文件头：必须在内存中保留PF_FILE_HDR_SIZE字节
    // 尽管实际FileHdr的大小更小
    char hdrBuf[PF_FILE_HDR_SIZE];

    // 清除内存以避免Purify警告
    memset(hdrBuf, 0, PF_FILE_HDR_SIZE);

    // 设置文件头信息
    PF_FileHdr *hdr = (PF_FileHdr*)hdrBuf;
    hdr->firstFree = PF_PAGE_LIST_END;  // 初始空闲页列表结束
    hdr->numPages = 0;                  // 初始页数为0

    // 将文件头写入文件
    if((numBytes = write(fd, hdrBuf, PF_FILE_HDR_SIZE))
       != PF_FILE_HDR_SIZE) {

        // 写入错误：关闭并删除文件
        close(fd);
        unlink(fileName);

        // 返回错误码
        if(numBytes < 0)
            return (PF_UNIX);    // UNIX系统错误
        else
            return (PF_HDRWRITE); // 文件头写入不完整
    }

    // 关闭文件
    if(close(fd) < 0)
        return (PF_UNIX);

    // 返回成功
    return (0);
}

//----------------------------------------------
// 销毁文件
// 描述: 删除名为 fileName 的 PF 文件
//       (文件必须存在且未被打开)
// 输入: fileName - 要删除的文件名
// 返回: PF 返回码
//----------------------------------------------
RC PF_Manager::DestroyFile(const char *fileName)
{
    // 删除文件
    if (unlink(fileName) < 0)
        return (PF_UNIX);

    // 返回成功
    return (0);
}

//----------------------------------------------
// 打开文件
// 描述: 打开名为 "fileName" 的分页文件
//       注意：多次打开同一文件会被视为不同文件(不同文件描述符和缓冲区)
//       多次打开同一文件进行写入可能导致文件损坏
//       即使只有一个实例进行写入，也可能导致读取不一致问题
// 输入: fileName - 要打开的文件名
// 输出: fileHandle - 引用打开的文件
//                   此函数修改fileHandle中的局部变量
//                   指向文件表中的文件数据和缓冲区管理器对象
// 返回: PF_FILEOPEN 或其他 PF 返回码
//----------------------------------------------
RC PF_Manager::OpenFile(const char *fileName, PF_FileHandle &fileHandle)
{
    RC rc;  // 返回码

    // 确保文件未被打开
    if (fileHandle.bFileOpen)
        return (PF_FILEOPEN);

    // 打开文件
    if ((fileHandle.unixfd = open(fileName,
#ifdef PC
            O_BINARY |  // Windows平台需要二进制模式
#endif
                                  O_RDWR)) < 0)  // 读写模式
        return (PF_UNIX);

    // 读取文件头
    {
        int numBytes = read(fileHandle.unixfd, (char *)&fileHandle.hdr,
                            sizeof(PF_FileHdr));
        if (numBytes != sizeof(PF_FileHdr)) {
            rc = (numBytes < 0) ? PF_UNIX : PF_HDRREAD;
            goto err;
        }
    }

    // 标记文件头未修改
    fileHandle.bHdrChanged = FALSE;

    // 设置文件句柄中的局部变量引用打开的文件
    fileHandle.pBufferMgr = pBufferMgr;
    fileHandle.bFileOpen = TRUE;

    // 返回成功
    return 0;

    err:
    // 错误处理：关闭文件
    close(fileHandle.unixfd);
    fileHandle.bFileOpen = FALSE;

    // 返回错误
    return (rc);
}

//----------------------------------------------
// 关闭文件
// 描述: 关闭与 fileHandle 关联的文件
//       文件应已通过 OpenFile() 打开
//       同时刷新文件的所有页面缓冲区
//       在缓冲区中仍有固定页面时关闭文件是错误的
// 输入: fileHandle - 要关闭的文件句柄
// 输出: fileHandle - 不再引用打开的文件
//                   此函数修改fileHandle中的局部变量
// 返回: PF 返回码
//----------------------------------------------
RC PF_Manager::CloseFile(PF_FileHandle &fileHandle)
{
    RC rc;

    // 确保fileHandle引用已打开的文件
    if (!fileHandle.bFileOpen)
        return (PF_CLOSEDFILE);

    // 刷新此文件的所有缓冲区并写出文件头
    if ((rc = fileHandle.FlushPages()))
        return (rc);

    // 关闭文件
    if (close(fileHandle.unixfd) < 0)
        return (PF_UNIX);
    fileHandle.bFileOpen = FALSE;

    // 重置文件句柄中的缓冲区管理器指针
    fileHandle.pBufferMgr = NULL;

    // 返回成功
    return 0;
}

//----------------------------------------------
// 清空缓冲区
// 描述: 从缓冲区管理器中移除所有条目
//       此例程将通过系统命令调用
//       主要用于用户希望从干净缓冲区开始进行性能比较
// 输入: 无
// 输出: 无
// 返回: PF_BufferMgr::ClearBuffer 的结果
//       0 表示成功，其他值表示 PF 错误
//----------------------------------------------
RC PF_Manager::ClearBuffer()
{
    return pBufferMgr->ClearBuffer();
}

//----------------------------------------------
// 打印缓冲区
// 描述: 显示缓冲区中的所有页面
//       此例程将通过系统命令调用
// 输入: 无
// 输出: 无
// 返回: PF_BufferMgr::PrintBuffer 的结果
//       0 表示成功，其他值表示 PF 错误
//----------------------------------------------
RC PF_Manager::PrintBuffer()
{
    return pBufferMgr->PrintBuffer();
}

//----------------------------------------------
// 调整缓冲区大小
// 描述: 将缓冲区管理器调整为指定大小
//       此例程将通过系统命令调用
// 输入: iNewSize - 新缓冲区大小
// 输出: 无
// 返回: PF_BufferMgr::ResizeBuffer 的结果
//       0 表示成功，PF_TOOSMALL 表示iNewSize太小
//----------------------------------------------
RC PF_Manager::ResizeBuffer(int iNewSize)
{
    return pBufferMgr->ResizeBuffer(iNewSize);
}

//------------------------------------------------------------------------------
// 原始内存缓冲区操作方法
// 这些内存位置由缓冲区管理器处理，但不与特定文件关联
// 如果您想要受缓冲池大小限制的内存，应使用这些方法
//
// PF_Manager 只是将这些调用传递给缓冲区管理器
//------------------------------------------------------------------------------

// 获取块大小
RC PF_Manager::GetBlockSize(int &length) const
{
    return pBufferMgr->GetBlockSize(length);
}

// 分配块
RC PF_Manager::AllocateBlock(char *&buffer)
{
    return pBufferMgr->AllocateBlock(buffer);
}

// 释放块
RC PF_Manager::DisposeBlock(char *buffer)
{
    return pBufferMgr->DisposeBlock(buffer);
}
