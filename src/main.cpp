#include "server.h"

#include <iostream>

#include <netinet/in.h>

int main(int argc, char **argv) {
    HttpServer server;
    try {
        server.Run(INADDR_ANY, 4221);
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }
    return 0;
}