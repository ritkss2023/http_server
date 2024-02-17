#ifndef HTTP_SERVER_STARTER_CPP_SERVER_H
#define HTTP_SERVER_STARTER_CPP_SERVER_H

#include "file_descriptor.h"
#include "http.h"
#include "http_parser.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <variant>
#include <unordered_map>

#include <cstdint>

struct HttpServerException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class HttpServer {
    struct ConnectionState {
        FileDescriptor socket;
        HttpParser http_parser;
        std::string buffer;
    };

    std::filesystem::directory_entry directory_;
    FileDescriptor epoll_fd_;
    FileDescriptor listening_socket_;
    std::unordered_map<int, ConnectionState> connections_;

public:
    HttpServer(std::filesystem::directory_entry directory);

    void Run(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port);

private:
    void CreateEPoll();
    void AddFileDescriptorToEPoll(FileDescriptor& fd);
    void RemoveFileDescriptorFromEPoll(FileDescriptor& fd);

    void OpenListeningSocket(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port);
    void Listen();

    void RunEventLoop();
    void AcceptNewConnections();
    void ProcessConnection(int socket_fd);

    HttpResponse HandleRequest(const HttpRequest& request);
};

#endif //HTTP_SERVER_STARTER_CPP_SERVER_H
