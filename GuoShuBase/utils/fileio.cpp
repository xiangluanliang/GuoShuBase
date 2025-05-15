//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/9 19:38
// @Project: GuoShuBase
//
//

#include "fileio.h"

#ifdef _WIN32
#define OPEN _open
#define CLOSE _close
#define READ _read
#define WRITE _write
#define SEEK _lseek
#define UNLINK _unlink
#define L_SET SEEK_SET
#else
#define OPEN open
#define CLOSE close
#define READ read
#define WRITE write
#define SEEK lseek
#define UNLINK unlink
#define L_SET SEEK_SET
#endif

namespace FileIO {
    int Open(const std::string& path, int flags, int mode) {
        return OPEN(path.c_str(), flags, mode);
    }

    int Close(int fd) {
        return CLOSE(fd);
    }

    int Read(int fd, void* buffer, int count) {
        return READ(fd, buffer, count);
    }

    int Write(int fd, const void* buffer, int count) {
        return WRITE(fd, buffer, count);
    }

    int Seek(int fd, int offset, int whence) {
        return SEEK(fd, offset, whence);
    }

    int Unlink(const std::string& path) {
        return UNLINK(path.c_str());
    }
}

