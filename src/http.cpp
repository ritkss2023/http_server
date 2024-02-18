#include "http.h"

#include "http_utils.h"

#include <string_view>

namespace {
constexpr std::string_view ToString(HttpResponseStatus status) noexcept {
    switch (status) {
    case HttpResponseStatus::k200Ok:
        return "200 OK";
    case HttpResponseStatus::k201Created:
        return "201 Created";
    case HttpResponseStatus::k400BadRequest:
        return "400 Bad Request";
    case HttpResponseStatus::k404NotFound:
        return "404 Not Found";
    case HttpResponseStatus::k422UnprocessableContent:
        return "422 Unprocessable Content";
    }
}
}

std::string ToString(const HttpResponse& response) {
    std::string res;
    res += kHttpVersion;
    res += ' ';
    res += ToString(response.response_status);
    res += kHttpLineTerminator;

    for (const auto& [header, value] : response.headers) {
        res += header;
        res += ": ";
        res += value;
        res += kHttpLineTerminator;
    }
    res += kHttpContentLengthHeader;
    res += ": ";
    res += std::to_string(response.body.size());
    res += kHttpLineTerminator;


    res += kHttpLineTerminator;

    res += response.body;

    return res;
}

