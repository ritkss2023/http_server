#include "server.h"

#include "http_parser.h"
#include "http_utils.h"
#include "utils.h"

#include <string>
#include <string_view>

#include <cstddef>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>

namespace {
constexpr std::string_view kHttpEchoPath{"/echo/"};
constexpr std::string_view kUserAgentPath{"/user-agent"};
constexpr std::string_view kHttpRootPath{"/"};
}

void HttpServer::Run(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port) {
    OpenListeningSocket(ipv4_address, port);
    Listen();

    //Waiting for a client to connect
    sockaddr_in client_addr;
    socklen_t client_addr_len{sizeof(client_addr)};
    FileDescriptor client_socket{accept(listening_socket_.Get(), (sockaddr*)&client_addr, &client_addr_len)};

    // Check if client successfully connected
    if (client_socket.Get() == -1) {
        return;
    }

    HttpParser parser;
    HttpParserState parser_state{HttpParserState::kStartLine};
    std::string buffer;
    while (parser_state != HttpParserState::kError && parser_state != HttpParserState::kFinished) {
        char read_buf[1024];
        const ssize_t bytes_read = read(client_socket.Get(), read_buf, sizeof(read_buf));
        if (bytes_read < 0) {
            parser_state = HttpParserState::kError;
            break;
        }

        buffer += std::string_view{read_buf, static_cast<size_t>(bytes_read)};
        parser_state = parser.Parse(buffer);
    }

    const HttpResponse response = parser_state == HttpParserState::kFinished
        ? HandleRequest(parser.GetRequest())
        : HttpResponse{.response_status = HttpResponseStatus::k400BadRequest};

    const std::string response_str{ToString(response)};
    write(client_socket.Get(), response_str.data(), response_str.size());
}

void HttpServer::OpenListeningSocket(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port) {
    const uint32_t addr = std::visit(overloaded{
        [](const std::string& address) {
            const uint32_t addr = inet_addr(address.c_str());
            if (addr == (uint32_t)(-1)) {
                throw std::invalid_argument("Invalid IP address: " + address);
            }
            return addr;
        },
        [](uint32_t address) { return htonl(address); }
    }, ipv4_address);
    const std::uint16_t network_port = htons(port);


    listening_socket_ = FileDescriptor{socket(AF_INET, SOCK_STREAM, 0)};
    if (listening_socket_.Get() < 0) {
        throw HttpServerException{"Failed to create server socket"};
    }

    // Since the tester restarts your program quite often, setting REUSE_PORT
    // ensures that we don't run into 'Address already in use' errors
    static constexpr int kReusePort{1};
    if (setsockopt(listening_socket_.Get(), SOL_SOCKET, SO_REUSEPORT, &kReusePort, sizeof(kReusePort)) < 0) {
        throw HttpServerException{"setsockopt failed"};
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = addr;
    server_addr.sin_port = network_port;

    if (bind(listening_socket_.Get(), (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        using namespace std::string_literals;
        throw HttpServerException{
            "Failed to bind to IP address"s + inet_ntoa(server_addr.sin_addr) + "on port " + std::to_string(port)};
    }
}

void HttpServer::Listen() {
    static constexpr int kConnectionBacklog{5};
    if (listen(listening_socket_.Get(), kConnectionBacklog) != 0) {
        throw HttpServerException{"listen failed"};
    }
}

HttpResponse HttpServer::HandleRequest(const HttpRequest& request) {
    using enum HttpMethod;
    using enum HttpResponseStatus;

    if (request.method == kGet && request.path.starts_with(kHttpEchoPath)) {
        return HttpResponse{
            .response_status = k200Ok,
            .headers = {{std::string{kHttpContentTypeHeader}, "text/plain"}},
            .body = request.path.substr(kHttpEchoPath.size())
        };
    } else if (request.method == kGet && request.path == kUserAgentPath) {
        auto user_agent_header_it{request.headers.find("User-Agent")};
        if (user_agent_header_it != request.headers.end()) {
            return HttpResponse{
                .response_status = k200Ok,
                .headers = {{std::string{kHttpContentTypeHeader}, "text/plain"}},
                .body = user_agent_header_it->second
            };
        }
    } else if (request.method == kGet && request.path == kHttpRootPath) {
        return HttpResponse{.response_status = HttpResponseStatus::k200Ok};
    }
    return HttpResponse{.response_status = k404NotFound};
}
