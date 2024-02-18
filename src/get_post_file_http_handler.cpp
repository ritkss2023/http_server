#include "get_post_file_http_handler.h"

#include "http_utils.h"

#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

namespace {
constexpr std::string_view kHttpFilesPath{"/files/"};
}

GetFileHttpHandler::GetFileHttpHandler(std::filesystem::directory_entry directory)
    : directory_{std::move(directory)}
{
}

bool GetFileHttpHandler::IsMyRequest(const HttpRequest& request) const {
    return request.method == HttpMethod::kGet
        && request.path.starts_with(kHttpFilesPath)
        && request.path.size() != kHttpFilesPath.size();
}

HttpResponse GetFileHttpHandler::HandleRequest(const HttpRequest& request) {
    if (!directory_.exists()) {
        return HttpResponse {.response_status = HttpResponseStatus::k404NotFound};
    }
    const std::string_view file{std::string_view{request.path}.substr(kHttpFilesPath.size())};
    const std::filesystem::path file_path{directory_.path() / file};
    if (!std::filesystem::is_regular_file(file_path)) {
        return HttpResponse {.response_status = HttpResponseStatus::k404NotFound};
    }
    std::ifstream fs{file_path, std::ios_base::binary};
    if (!fs) {
        return HttpResponse {.response_status = HttpResponseStatus::k404NotFound};
    }
    return HttpResponse {
        .response_status = HttpResponseStatus::k200Ok,
        .headers = {{std::string{kHttpContentTypeHeader}, "application/octet-stream"}},
        .body = std::string{std::istreambuf_iterator<char>{fs}, std::istreambuf_iterator<char>{}}
    };
}

PostFileHttpHandler::PostFileHttpHandler(std::filesystem::directory_entry directory)
    : directory_{std::move(directory)}
{
}

bool PostFileHttpHandler::IsMyRequest(const HttpRequest& request) const {
    return request.method == HttpMethod::kPost
        && request.path.starts_with(kHttpFilesPath)
        && request.path.size() != kHttpFilesPath.size();
}

HttpResponse PostFileHttpHandler::HandleRequest(const HttpRequest& request) {
    if (!directory_.exists()) {
        return HttpResponse {.response_status = HttpResponseStatus::k422UnprocessableContent};
    }
    const std::string_view file{std::string_view{request.path}.substr(kHttpFilesPath.size())};
    const std::filesystem::path file_path{directory_.path() / file};
    std::ofstream fs{file_path, std::ios_base::binary};
    if (!fs) {
        return HttpResponse{.response_status = HttpResponseStatus::k422UnprocessableContent};
    }
    fs.write(request.body.data(), request.body.size());
    return HttpResponse{.response_status = HttpResponseStatus::k201Created};
}
