//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/9 19:38
// @Project: GuoShuBase
//
//

#ifndef FILEIO_H
#define FILEIO_H

#pragma once

#include <string>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

namespace FileIO {
    int Open(const std::string& path, int flags, int mode = 0644);
    int Close(int fd);
    int Read(int fd, void* buffer, int count);
    int Write(int fd, const void* buffer, int count);
    int Seek(int fd, int offset, int whence);
    int Unlink(const std::string& path);
}



#endif //FILEIO_H
