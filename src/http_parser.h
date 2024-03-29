#ifndef HTTP_SERVER_HTTP_PARSER_H
#define HTTP_SERVER_HTTP_PARSER_H

#include "http.h"

#include <string>
#include <string_view>

enum class HttpParserState {
    kStartLine,
    kHeaders,
    kBody,
    kFinished,
    kError,
};


class HttpParser {
    HttpParserState state_ = HttpParserState::kStartLine;
    HttpRequest request_;

public:
    HttpParserState GetState() const noexcept { return state_; }
    HttpParserState Parse(std::string& buffer);
    HttpRequest GetRequest();

private:
    bool ParseStartLine(std::string_view& buffer);
    bool ParseHeaders(std::string_view& buffer);
    bool ParseBody(std::string_view& buffer);
};


#endif //HTTP_SERVER_HTTP_PARSER_H
