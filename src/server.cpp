#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#include <cctype>
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

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    void Close() {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
    }

    int Get() const { return fd_; }

};

namespace {
    constexpr std::string_view kHttpLineTerminator{"\r\n"};

    constexpr std::string_view kHttpGetMethod{"GET"};
    constexpr std::string_view kHttpRootPath{"/"};
    constexpr std::string_view kHttpVersion{"HTTP/1.1"};

    constexpr std::string_view kOkEmptyResponse{"HTTP/1.1 200 OK\r\n\r\n"};
    constexpr std::string_view kNotFoundEmptyResponse{"HTTP/1.1 404 Not Found\r\n\r\n"};

    bool IsWs(char ch) noexcept {
        return static_cast<bool>(isspace(ch));
    }

    void SkipWs(std::string_view &str) {
        while (!str.empty() && IsWs(str.front())) {
            str.remove_prefix(1);
        }
    }

    std::string_view ReadWord(std::string_view &str) {
        SkipWs(str);
        auto word_last = std::ranges::find_if(str, &IsWs);
        const size_t word_length = std::distance(std::ranges::begin(str), word_last);
        auto word = str.substr(0, word_length);
        str.remove_prefix(word_length);
        return word;
    }

    std::string_view ParseHttpGetRequestStartline(std::string_view start_line) {
        const auto method = ReadWord(start_line);
        auto path = ReadWord(start_line);
        const auto http_version = ReadWord(start_line);
        SkipWs(start_line);
        if (method == kHttpGetMethod && http_version == kHttpVersion && start_line.empty()) {
            return path;
        }
        return {};
    }
}


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

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd.Get(), (sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    static constexpr int kConnectionBacklog{5};
    if (listen(server_fd.Get(), kConnectionBacklog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    FileDescriptor client_fd{accept(server_fd.Get(), (sockaddr *) &client_addr, &client_addr_len)};
    if (client_fd.Get() == -1) {
        std::cerr << "Accept failed\n";
        return 1;
    }
    std::cout << "Client connected\n";

    std::string http_request_start_line;
    while (true) {
        char buffer[1024];
        const ssize_t bytes_read = read(client_fd.Get(), buffer, sizeof(buffer));
        if (bytes_read < 0) {
            std::cerr << "Read failed\n";
            return 1;
        }
        const char* start_line_last = std::search(buffer, buffer + bytes_read,
          std::ranges::begin(kHttpLineTerminator), std::ranges::end(kHttpLineTerminator));

        const std::string_view current_bytes{buffer, start_line_last};
        http_request_start_line += current_bytes;

        // We've read http start line
        if (current_bytes.size() != bytes_read) {
            break;
        }
    }

    const std::string_view request_path = ParseHttpGetRequestStartline(http_request_start_line);
    if (request_path == kHttpRootPath) {
        write(client_fd.Get(), kOkEmptyResponse.data(), kOkEmptyResponse.size());
    } else {
        write(client_fd.Get(), kNotFoundEmptyResponse.data(), kNotFoundEmptyResponse.size());
    }

    return 0;
}
