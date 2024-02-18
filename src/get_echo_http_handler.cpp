#include "get_echo_http_handler.h"

#include "http_utils.h"

#include <string_view>

namespace {
constexpr std::string_view kHttpEchoPath{"/echo/"};
}

bool GetEchoHttpHandler::IsMyRequest(const HttpRequest& request) const {
    return request.method == HttpMethod::kGet && request.path.starts_with(kHttpEchoPath);
}

HttpResponse GetEchoHttpHandler::HandleRequest(const HttpRequest& request) {
    return HttpResponse{
        .response_status = HttpResponseStatus::k200Ok,
        .headers = {{std::string{kHttpContentTypeHeader}, "text/plain"}},
        .body = request.path.substr(kHttpEchoPath.size())
    };
}
