#include "get_user_agent_http_handler.h"

#include "http_utils.h"

#include <string_view>

namespace {
constexpr std::string_view kUserAgentPath{"/user-agent"};
}

bool GetUserAgentHttpHandler::IsMyRequest(const HttpRequest& request) const {
    return request.method == HttpMethod::kGet && request.path == kUserAgentPath;
}

HttpResponse GetUserAgentHttpHandler::HandleRequest(const HttpRequest& request) {
    auto user_agent_header_it{request.headers.find("User-Agent")};
    if (user_agent_header_it != request.headers.end()) {
        return HttpResponse{
            .response_status = HttpResponseStatus::k200Ok,
            .headers = {{std::string{kHttpContentTypeHeader}, "text/plain"}},
            .body = user_agent_header_it->second
        };
    }
    return HttpResponse{.response_status = HttpResponseStatus::k404NotFound};
}