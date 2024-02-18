#include "get_root_http_handler.h"

#include <string_view>

namespace {
constexpr std::string_view kHttpRootPath{"/"};
}

bool GetRootHttpHandler::IsMyRequest(const HttpRequest& request) const {
    return request.method == HttpMethod::kGet && request.path == kHttpRootPath;
}

HttpResponse GetRootHttpHandler::HandleRequest(const HttpRequest& request) {
    return HttpResponse{.response_status = HttpResponseStatus::k200Ok};
}
