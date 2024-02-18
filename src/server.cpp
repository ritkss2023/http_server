#include "server.h"

#include "http_utils.h"
#include "str_utils.h"
#include "utils.h"

#include <array>
#include <fstream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

#include <cerrno>
#include <cstddef>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/epoll.h>
#include <sys/socket.h>

namespace {
constexpr std::string_view kHttpEchoPath{"/echo/"};
constexpr std::string_view kHttpFilesPath{"/files/"};
constexpr std::string_view kUserAgentPath{"/user-agent"};
constexpr std::string_view kHttpRootPath{"/"};
}


HttpServer::HttpServer(std::filesystem::directory_entry directory)
    : directory_{std::move(directory)}
{
}


void HttpServer::Run(const std::variant<std::string, uint32_t>& ipv4_address, std::uint16_t port) {
    CreateEPoll();
    OpenListeningSocket(ipv4_address, port);
    Listen();
    RunEventLoop();
}

void HttpServer::CreateEPoll() {
    epoll_fd_ = FileDescriptor{epoll_create1(0)};
    if (epoll_fd_.IsEmpty()) {
        throw HttpServerException{StrError("epoll_create1 failed")};
    }
}


void HttpServer::AddFileDescriptorToEPoll(FileDescriptor& fd) {
    static constexpr auto kEPollEdgeTriggeredReadEvent{EPOLLIN | EPOLLET};
    epoll_event event;
    event.events = kEPollEdgeTriggeredReadEvent;
    event.data.fd = fd.Get();
    if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_ADD, fd.Get(), &event) == -1) {
        throw HttpServerException(StrError("EPOLL_CTL_ADD on File Descriptor " + std::to_string(fd.Get()) + " failed"));
    }
}

void HttpServer::RemoveFileDescriptorFromEPoll(FileDescriptor& fd) {
    if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_DEL, fd.Get(), static_cast<epoll_event*>(nullptr)) == -1) {
        throw HttpServerException(StrError("EPOLL_CTL_DEL on File Descriptor " + std::to_string(fd.Get()) + " failed"));
    }
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
    if (listening_socket_.IsEmpty()) {
        throw HttpServerException{StrError("Failed to create server socket")};
    }
    listening_socket_.SetNonBlocking(true);

    // Since the tester restarts your program quite often, setting REUSE_PORT
    // ensures that we don't run into 'Address already in use' errors
    static constexpr int kReusePort{1};
    if (setsockopt(listening_socket_.Get(), SOL_SOCKET, SO_REUSEPORT, &kReusePort, sizeof(kReusePort)) < 0) {
        throw HttpServerException{StrError("setsockopt failed")};
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = addr;
    server_addr.sin_port = network_port;

    if (bind(listening_socket_.Get(), (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        using namespace std::string_literals;
        throw HttpServerException{StrError(
            "Failed to bind to IP address"s + inet_ntoa(server_addr.sin_addr) + "on port " + std::to_string(port)
        )};
    }
}

void HttpServer::Listen() {
    static constexpr int kConnectionBacklog{5};
    if (listen(listening_socket_.Get(), kConnectionBacklog) == -1) {
        throw HttpServerException{StrError("listen failed")};
    }
}

void HttpServer::RunEventLoop() {
    static constexpr int kWaitIndefinitely{-1};
    static constexpr size_t kEPollMaxEvents = 16;

    AddFileDescriptorToEPoll(listening_socket_);

    std::array<epoll_event, kEPollMaxEvents> events;
    while (true) {
        const int events_count{
            epoll_wait(epoll_fd_.Get(), events.data(), static_cast<int>(events.size()), kWaitIndefinitely)};

        // ToDo: Handle signals, errors, etc. later
        if (events_count == -1) {
            continue;
        }

        for (const epoll_event& event : events | std::views::take(static_cast<size_t>(events_count))) {
            if (event.data.fd == listening_socket_.Get()) {
                AcceptNewConnections();
            } else {
                ProcessConnection(event.data.fd);
            }
        }
    }
}

void HttpServer::AcceptNewConnections() {
    sockaddr_in client_addr;
    socklen_t client_addr_len{sizeof(client_addr)};
    while (true) {
        FileDescriptor client_socket{accept(listening_socket_.Get(), (sockaddr*)&client_addr, &client_addr_len)};

        // Check if client successfully connected
        if (client_socket.IsEmpty()) {
            // No connections are present to be accepted
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                continue;
            }
        }

        client_socket.SetNonBlocking(true);
        AddFileDescriptorToEPoll(client_socket);
        connections_.try_emplace(client_socket.Get(), std::move(client_socket));
    }
}

void HttpServer::ProcessConnection(int socket_fd) {
    auto connection_state_it{connections_.find(socket_fd)};
    if (connection_state_it == connections_.end()) {
        return;
    }

    ConnectionState& connection_state{connection_state_it->second};
    HttpParserState parser_state{connection_state.http_parser.GetState()};

    static constexpr size_t kReadBufSize{1024};
    std::array<char, kReadBufSize> read_buf;
    bool not_enough_data{false};
    while (parser_state != HttpParserState::kError && parser_state != HttpParserState::kFinished) {

        const ssize_t bytes_read{read(connection_state.socket.Get(), read_buf.data(), read_buf.size())};

        // EOF
        if (bytes_read == 0) {
            break;
        } else if (bytes_read == -1) {
            // No data is present to be accepted
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                not_enough_data = true;
                break;
            } else {
                // ToDo: Handle unsuccessful read
                break;
            }
        } else {
            connection_state.buffer += std::string_view{read_buf.data(), static_cast<size_t>(bytes_read)};
            parser_state = connection_state.http_parser.Parse(connection_state.buffer);
        }
    }

    if (not_enough_data) {
        return;
    }

    const HttpResponse response{parser_state == HttpParserState::kFinished
                                ? HandleRequest(connection_state.http_parser.GetRequest())
                                : HttpResponse{.response_status = HttpResponseStatus::k400BadRequest}};

    const std::string response_str{ToString(response)};
    write(connection_state.socket.Get(), response_str.data(), response_str.size());
    RemoveFileDescriptorFromEPoll(connection_state.socket);
    connections_.erase(connection_state_it);
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
    } else if (request.method == kGet && request.path.starts_with(kHttpFilesPath)) {
        const std::string_view file{std::string_view{request.path}.substr(kHttpFilesPath.size())};
        if (file.empty() || !directory_.exists()) {
            return HttpResponse {.response_status = k404NotFound};
        }
        const std::filesystem::path file_path{directory_.path() / file};
        if (!std::filesystem::is_regular_file(file_path)) {
            return HttpResponse {.response_status = k404NotFound};
        }
        std::ifstream fs{file_path, std::ios_base::binary};
        if (!fs) {
            return HttpResponse {.response_status = k404NotFound};
        }
        return HttpResponse {
            .response_status = k200Ok,
            .headers = {{std::string{kHttpContentTypeHeader}, "application/octet-stream"}},
            .body = std::string{std::istreambuf_iterator<char>{fs}, std::istreambuf_iterator<char>{}}
        };
    } else if (request.method == kPost && request.path.starts_with(kHttpFilesPath)) {
        const std::string_view file{std::string_view{request.path}.substr(kHttpFilesPath.size())};
        if (file.empty() || !directory_.exists()) {
            return HttpResponse {.response_status = k404NotFound};
        }
        const std::filesystem::path file_path{directory_.path() / file};
        std::ofstream fs{file_path, std::ios_base::binary};
        if (!fs) {
            return HttpResponse{.response_status = k422UnprocessableContent};
        }
        fs.write(request.body.data(), request.body.size());
        return HttpResponse{.response_status = k201Created};
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
