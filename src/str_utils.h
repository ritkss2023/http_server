#ifndef HTTP_SERVER_STR_UTILS_H
#define HTTP_SERVER_STR_UTILS_H

#include <optional>
#include <string>
#include <string_view>

#include <cctype>

inline bool IsWs(char ch) noexcept {
    return static_cast<bool>(isspace(ch));
}

void SkipWs(std::string_view& str) noexcept;

std::string_view Strip(std::string_view str) noexcept;

std::string_view ReadWord(std::string_view& str) noexcept;

std::optional<size_t> TryParseSizeT(std::string_view str) noexcept;

std::string StrError(std::string_view str);

#endif //HTTP_SERVER_STR_UTILS_H
