#include "server.h"

#include <filesystem>
#include <iostream>
#include <string_view>

#include <netinet/in.h>

namespace {
std::filesystem::directory_entry ParseArgs(int argc, char** argv) {
    if (argc != 3 || std::string_view{argv[1]} != "--directory") {
        return {};
    }
    return std::filesystem::directory_entry{std::string_view{argv[2]}};
}
}

int main(int argc, char **argv) {
    std::filesystem::directory_entry dir_entry{ParseArgs(argc, argv)};
    if (argc > 1 && !dir_entry.exists()) {
        std::cout << "Usage: server --directory <path-to-directory>\n";
        return 1;
    }

    HttpServer server{std::move(dir_entry)};
    try {
        server.Run(INADDR_ANY, 4221);
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }
    return 0;
}