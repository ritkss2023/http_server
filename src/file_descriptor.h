#ifndef HTTP_SERVER_STARTER_CPP_FILE_DESCRIPTOR_H
#define HTTP_SERVER_STARTER_CPP_FILE_DESCRIPTOR_H

#include <utility>

#include <unistd.h>

class FileDescriptor {
    int fd_{-1};

public:
    FileDescriptor() = default;

    explicit FileDescriptor(int fd)
        : fd_{fd}
    {
    }

    ~FileDescriptor() {
        Close();
    }

    FileDescriptor(FileDescriptor&& other) noexcept
        : fd_{std::exchange(other.fd_, -1)}
    {
    }

    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        Close();
        fd_ = std::exchange(other.fd_, -1);
        return *this;
    }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    void Close() noexcept {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
    }

    int Get() { return fd_; }

};

#endif //HTTP_SERVER_STARTER_CPP_FILE_DESCRIPTOR_H
