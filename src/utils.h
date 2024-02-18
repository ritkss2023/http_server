#ifndef HTTP_SERVER_UTILS_H
#define HTTP_SERVER_UTILS_H

#include <memory>
#include <utility>

// helper type for the std::variant visitor
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

struct ToAddress {
    template <typename Ptr>
    constexpr decltype(auto) operator()(Ptr&& ptr) const noexcept {
        return std::to_address(std::forward<Ptr>(ptr));
    }
};

#endif //HTTP_SERVER_UTILS_H
