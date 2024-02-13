#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>

class FileDescriptor {
    int fd_ = -1;

public:
    FileDescriptor() = default;

    explicit FileDescriptor(int fd)
        : fd_(fd)
    {
    }

    ~FileDescriptor() {
        Close();
    }

    FileDescriptor(FileDescriptor&& other)
        : fd_(std::exchange(other.fd_, -1))
    {
    }

    FileDescriptor& operator=(FileDescriptor&& other) {
        Close();
        fd_ = std::exchange(other.fd_, -1);
        return *this;
    }

    FileDescriptor(const FileDescriptor&);
    FileDescriptor& operator=(const FileDescriptor&);

    void Close() {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
    }

    int Get() const { return fd_; }

};

constexpr std::string_view kOkEmptyResponse{"HTTP/1.1 200 OK\r\n\r\n"};

int main(int argc, char **argv) {
    FileDescriptor server_fd{socket(AF_INET, SOCK_STREAM, 0)};
    if (server_fd.Get() < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    // Since the tester restarts your program quite often, setting REUSE_PORT
    // ensures that we don't run into 'Address already in use' errors
    static constexpr int kReusePort = 1;
    if (setsockopt(server_fd.Get(), SOL_SOCKET, SO_REUSEPORT, &kReusePort, sizeof(kReusePort)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd.Get(), (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    static constexpr int kConnectionBacklog{5};
    if (listen(server_fd.Get(), kConnectionBacklog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    FileDescriptor client_fd{accept(server_fd.Get(), (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len)};
    if (client_fd.Get() == -1) {
        std::cerr << "Accept failed\n";
        return 1;
    }
    std::cout << "Client connected\n";

    char buffer[1024];
    const int bytes_read = read(client_fd.Get(), buffer, sizeof(buffer));
    if (bytes_read < 0) {
        std::cerr << "Read failed\n";
        return 1;
    }
    write(client_fd.Get(), kOkEmptyResponse.data(), kOkEmptyResponse.size());

    return 0;
}
