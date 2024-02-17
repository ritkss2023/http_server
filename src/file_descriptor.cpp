#include "file_descriptor.h"

#include "str_utils.h"

#include <fcntl.h>

void FileDescriptor::SetNonBlocking(bool non_blocking) {
    if (IsEmpty()) {
        throw FileDescriptorException{StrError("Empty File Descriptor")};
    }

    const int flags{fcntl(fd_, F_GETFL)};
    if (flags == -1) {
        throw FileDescriptorException{StrError("Failed to get File Descriptor flags")};
    }

    const int new_flags{non_blocking ? flags | O_NONBLOCK : flags & ~O_NONBLOCK};
    if (fcntl(fd_, F_SETFL, new_flags) == -1) {
        throw FileDescriptorException{StrError("Failed to set File Descriptor flags")};
    }
}
