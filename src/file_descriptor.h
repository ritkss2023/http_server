#ifndef HTTP_SERVER_STARTER_CPP_FILE_DESCRIPTOR_H
#define HTTP_SERVER_STARTER_CPP_FILE_DESCRIPTOR_H

#include <stdexcept>
#include <utility>

#include <unistd.h>

struct FileDescriptorException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

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

    void SetNonBlocking(bool non_blocking);

    void Close() noexcept {
        if (!IsEmpty()) {
            close(fd_);
            fd_ = -1;
        }
    }

    int Get() noexcept { return fd_; }
    bool IsEmpty() const noexcept { return fd_ == -1; }
};

#endif //HTTP_SERVER_STARTER_CPP_FILE_DESCRIPTOR_H
