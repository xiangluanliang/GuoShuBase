//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/17 23:10
// @Project: GuoShuBase
//
//
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "pf_buffermgr.h"

using namespace std;

const char* TEST_FILE = "buffer_test.data";;
const int TEST_PAGE = 1;
const char* TEST_DATA = "Hello Buffer Manager!";

int main() {
    PF_BufferMgr bufferMgr(10); // 初始化10页的缓冲池
    int fd;
    char* buffer;

    // 创建测试文件
    if ((fd = open(TEST_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
        cerr << "无法创建测试文件" << endl;
        return 1;
    }

    // 测试页面分配和写入
    RC rc = bufferMgr.AllocatePage(fd, TEST_PAGE, &buffer);
    if (rc != 0) {
        cerr << "页面分配失败: " << rc << endl;
        return 1;
    }
    strncpy(buffer, TEST_DATA, PF_PAGE_SIZE);
    bufferMgr.MarkDirty(fd, TEST_PAGE);

    // 测试页面读取
    char* readBuffer;
    rc = bufferMgr.GetPage(fd, TEST_PAGE, &readBuffer);
    if (rc != 0) {
        cerr << "页面读取失败: " << rc << endl;
        return 1;
    }
    bufferMgr.PrintBuffer();    // 验证数据
    if (strcmp(readBuffer, TEST_DATA) != 0) {
        cerr << "数据验证失败" << endl;
        cerr << "预期: " << TEST_DATA << endl;
        cerr << "实际: " << readBuffer << endl;
    } else {
        cout << readBuffer;
        cout << "数据验证成功!" << endl;
    }

    // 清理
    bufferMgr.UnpinPage(fd, TEST_PAGE);
    bufferMgr.FlushPages(fd);
//    close(fd);
    unlink(TEST_FILE);

    cout << "测试完成!" << endl;
    return 0;
}
