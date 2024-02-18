#ifndef HTTP_SERVER_HTTP_HANDLER_BASE_H
#define HTTP_SERVER_HTTP_HANDLER_BASE_H

#include "http.h"


class HttpHandlerBase {
public:
    virtual ~HttpHandlerBase() = default;

    virtual bool IsMyRequest(const HttpRequest& request) const = 0;
    virtual HttpResponse HandleRequest(const HttpRequest& request) = 0;
};

#endif //HTTP_SERVER_HTTP_HANDLER_BASE_H
