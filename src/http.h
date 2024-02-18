#ifndef HTTP_SERVER_STARTER_CPP_HTTP_H
#define HTTP_SERVER_STARTER_CPP_HTTP_H

#include <string>
#include <unordered_map>

enum class HttpMethod {
    kGet,
    kPost,
};

struct HttpRequest {
    HttpMethod method{HttpMethod::kGet};
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

enum class HttpResponseStatus {
    k200Ok = 200,
    k201Created = 201,
    k400BadRequest = 400,
    k404NotFound = 404,
    k422UnprocessableContent = 422,
};

struct HttpResponse {
    HttpResponseStatus response_status{HttpResponseStatus::k400BadRequest};
    std::unordered_map<std::string, std::string> headers;
    std::string body;

};

std::string ToString(const HttpResponse& response);

#endif //HTTP_SERVER_STARTER_CPP_HTTP_H
