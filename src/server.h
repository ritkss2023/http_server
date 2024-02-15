#ifndef HTTP_SERVER_STARTER_CPP_SERVER_H
#define HTTP_SERVER_STARTER_CPP_SERVER_H

#include "file_descriptor.h"
#include "http.h"

#include <stdexcept>
#include <string>
#include <variant>

#include <cstdint>

struct HttpServerException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class HttpServer {
    FileDescriptor listening_socket_;

public:
    void Run(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port);

private:
    void OpenListeningSocket(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port);
    void Listen();

    HttpResponse HandleRequest(const HttpRequest& request);
};

#endif //HTTP_SERVER_STARTER_CPP_SERVER_H
