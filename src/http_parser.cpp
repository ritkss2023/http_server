#include "http_parser.h"

#include "http.h"
#include "http_utils.h"
#include "str_utils.h"

#include <algorithm>
#include <optional>
#include <string>
#include <utility>

#include <cassert>

namespace {
constexpr auto FindHttpLineTerminator(std::string_view str) noexcept {
    return std::search(str.begin(), str.end(), kHttpLineTerminator.begin(), kHttpLineTerminator.end());
}
}

HttpParserState HttpParser::Parse(std::string& buffer) {
    std::string_view current_buffer{buffer};
    bool keep_parsing{true};
    while (keep_parsing) {
        switch (state_) {
        case HttpParserState::kStartLine:
            keep_parsing = ParseStartLine(current_buffer);
            break;
        case HttpParserState::kHeaders:
            keep_parsing = ParseHeaders(current_buffer);
            break;
        case HttpParserState::kBody:
            keep_parsing = ParseBody(current_buffer);
            break;
        case HttpParserState::kFinished:
        case HttpParserState::kError:
            // Should not enter here
            assert(false);
            break;
        }
    }
    buffer.erase(buffer.begin(), buffer.end() - current_buffer.size());
    return state_;
}

HttpRequest HttpParser::GetRequest() {
    assert(state_ == HttpParserState::kFinished);
    auto request{std::exchange(request_, HttpRequest{})};
    state_ = HttpParserState::kStartLine;
    return request;
}

bool HttpParser::ParseStartLine(std::string_view& buffer) {
    assert(state_ == HttpParserState::kStartLine);

    auto start_line_last_it{FindHttpLineTerminator(buffer)};
    if (start_line_last_it == buffer.end()) {
        // Start line not yet fully recieved
        return false;
    }

    std::string_view start_line{buffer.begin(), start_line_last_it};
    const std::string_view method_str{ReadWord(start_line)};
    const std::string_view path{ReadWord(start_line)};
    const std::string_view http_version{ReadWord(start_line)};
    SkipWs(start_line);
    const std::optional<HttpMethod> method{ToHttpMethod(method_str)};

    // Invalid Http/1.1 start-line provided
    if (method == std::nullopt || path.empty() || http_version != kHttpVersion || !start_line.empty()) {
        state_ = HttpParserState::kError;
        return false;
    }

    request_.method = *method;
    request_.path = path;
    buffer.remove_prefix(std::distance(buffer.begin(), start_line_last_it) + kHttpLineTerminator.size());
    state_ = HttpParserState::kHeaders;
    return !buffer.empty();
}

bool HttpParser::ParseHeaders(std::string_view& buffer) {
    assert(state_ == HttpParserState::kHeaders);

    auto header_last_it{buffer.end()};
    while ((header_last_it = FindHttpLineTerminator(buffer)) != buffer.end()) {
        const std::string_view header{buffer.begin(), header_last_it};

        // End of headers
        if (std::ranges::all_of(header, &IsWs)) {
            buffer.remove_prefix(header.size() + kHttpLineTerminator.size());
            const bool may_have_body{request_.headers.contains(std::string(kHttpContentLengthHeader))};
            state_ = may_have_body ? HttpParserState::kBody : HttpParserState::kFinished;
            return may_have_body && !buffer.empty();
        }

        auto header_key_last = std::find(header.begin(), header.end(), ':');

        const std::string_view key{Strip(std::string_view{header.begin(), header_key_last})};
        const std::string_view value{Strip(
            std::string_view{
                header_key_last != header.end() ? std::next(header_key_last) : header.end(),
                header.end()
            }
        )};

        // Invalid Header key-value pair
        if (key.empty() || value.empty()) {
            state_ = HttpParserState::kError;
            return false;
        }

        request_.headers.insert_or_assign(std::string(key), std::string(value));
        buffer.remove_prefix(std::distance(buffer.begin(), header_last_it) + kHttpLineTerminator.size());
    }

    // Complete header yet to be received
    return false;
}

bool HttpParser::ParseBody(std::string_view& buffer) {
    assert(state_ == HttpParserState::kBody);

    size_t body_size{0};
    auto content_length_it{request_.headers.find(std::string{kHttpContentLengthHeader})};
    if (content_length_it != request_.headers.end()) {
        if (const auto size{TryParseSizeT(content_length_it->second)}) {
            body_size = *size;
        } else {
            // Failed to read Content-Length number
            state_ = HttpParserState::kError;
            return false;
        }
    }


    const size_t to_read_size{std::min(body_size - request_.body.size(), buffer.size())};
    request_.body += buffer.substr(0, to_read_size);
    buffer.remove_prefix(to_read_size);

    if (request_.body.size() == body_size) {
        state_ = HttpParserState::kFinished;
    }

    // Since kBody is the last state we either read full body or wait for data to arrive
    return false;
}
