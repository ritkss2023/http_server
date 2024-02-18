#include "get_echo_http_handler.h"
#include "get_post_file_http_handler.h"
#include "get_root_http_handler.h"
#include "get_user_agent_http_handler.h"
#include "server.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

#include <netinet/in.h>

namespace {
struct CommandLine {
    std::filesystem::directory_entry dir_entry;
};

std::optional<CommandLine> ParseArgs(int argc, char** argv) {
    if (argc == 1) {
        return CommandLine{};
    }
    if (argc == 3 && std::string_view{argv[1]} == "--directory") {
        std::filesystem::directory_entry dir_entry{std::string_view{argv[2]}};
        if (dir_entry.exists()) {
            return CommandLine{.dir_entry = std::move(dir_entry)};
        }
    }
    return std::nullopt;
}
}

int main(int argc, char **argv) {
    auto command_line = ParseArgs(argc, argv);
    if (!command_line) {
        std::cout << "Usage: server [--directory <path-to-directory>]\n";
        return 1;
    }

    HttpServer server;
    server.AddHandler(std::make_unique<GetRootHttpHandler>());
    server.AddHandler(std::make_unique<GetEchoHttpHandler>());
    server.AddHandler(std::make_unique<GetUserAgentHttpHandler>());
    if (!command_line->dir_entry.path().empty()) {
        server.AddHandler(std::make_unique<GetFileHttpHandler>(command_line->dir_entry));
        server.AddHandler(std::make_unique<PostFileHttpHandler>(std::move(command_line->dir_entry)));
    }
    try {
        server.Run(INADDR_ANY, 4221);
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }
    return 0;
}