#ifndef HTTP_SERVER_STARTER_CPP_UTILS_H
#define HTTP_SERVER_STARTER_CPP_UTILS_H

// helper type for the std::variant visitor
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#endif //HTTP_SERVER_STARTER_CPP_UTILS_H
