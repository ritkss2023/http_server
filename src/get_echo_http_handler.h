#ifndef HTTP_SERVER_GET_ECHO_HTTP_HANDLER_H
#define HTTP_SERVER_GET_ECHO_HTTP_HANDLER_H

#include "http.h"
#include "http_handler_base.h"

class GetEchoHttpHandler : public HttpHandlerBase {
public:
    bool IsMyRequest(const HttpRequest& request) const override;
    HttpResponse HandleRequest(const HttpRequest& request) override;
};

#endif //HTTP_SERVER_GET_ECHO_HTTP_HANDLER_H
