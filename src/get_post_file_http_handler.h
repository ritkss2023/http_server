#ifndef HTTP_SERVER_GET_POST_FILE_HTTP_HANDLER_H
#define HTTP_SERVER_GET_POST_FILE_HTTP_HANDLER_H

#include "http.h"
#include "http_handler_base.h"

#include <filesystem>

class GetFileHttpHandler : public HttpHandlerBase {
    std::filesystem::directory_entry directory_;
public:
    GetFileHttpHandler(std::filesystem::directory_entry directory);

    bool IsMyRequest(const HttpRequest& request) const override;
    HttpResponse HandleRequest(const HttpRequest& request) override;
};

class PostFileHttpHandler : public HttpHandlerBase {
    std::filesystem::directory_entry directory_;
public:
    PostFileHttpHandler(std::filesystem::directory_entry directory);

    bool IsMyRequest(const HttpRequest& request) const override;
    HttpResponse HandleRequest(const HttpRequest& request) override;
};

#endif //HTTP_SERVER_GET_POST_FILE_HTTP_HANDLER_H
