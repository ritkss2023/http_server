#ifndef HTTP_SERVER_STARTER_CPP_HTTP_UTILS_H
#define HTTP_SERVER_STARTER_CPP_HTTP_UTILS_H

#include "http.h"

#include <optional>
#include <string_view>

inline constexpr std::string_view kHttpLineTerminator{"\r\n"};
inline constexpr std::string_view kHttpVersion{"HTTP/1.1"};
inline constexpr std::string_view kHttpContentLengthHeader{"Content-Length"};
inline constexpr std::string_view kHttpContentTypeHeader{"Content-Type"};

constexpr std::optional<HttpMethod> ToHttpMethod(std::string_view method) noexcept {
    using enum HttpMethod;
    if (method == "GET") {
        return kGet;
    }
    return std::nullopt;
}

#endif //HTTP_SERVER_STARTER_CPP_HTTP_UTILS_H
