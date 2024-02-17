#include "str_utils.h"

#include <algorithm>
#include <charconv>

#include <cerrno>
#include <cstring>

void SkipWs(std::string_view& str) noexcept {
    while (!str.empty() && IsWs(str.front())) {
        str.remove_prefix(1);
    }
}

std::string_view Strip(std::string_view str) noexcept {
    SkipWs(str);
    while (!str.empty() && IsWs(str.back())) {
        str.remove_suffix(1);
    }
    return str;
}

std::string_view ReadWord(std::string_view& str) noexcept {
    SkipWs(str);
    auto word_last{std::ranges::find_if(str, &IsWs)};
    const size_t word_length{static_cast<size_t>(std::ranges::distance(std::ranges::begin(str), word_last))};
    auto word{str.substr(0, word_length)};
    str.remove_prefix(word_length);
    return word;
}

std::optional<size_t> TryParseSizeT(std::string_view str) noexcept {
    size_t result{0};
    const auto [ptr, ec]{std::from_chars(str.data(), str.data() + str.size(), result)};
    if (ec == std::errc{} && ptr == str.data() + str.size()) {
        return result;
    }
    return std::nullopt;
}

std::string StrError(std::string_view str) {
    using namespace std::string_literals;
    std::string result;
    result += str;
    result += ": ";
    result += strerror(errno);
    return result;
}
