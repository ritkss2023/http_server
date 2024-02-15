cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard

add_executable(server)

target_sources(server
    PRIVATE
        src/file_descriptor.h
        src/file_descriptor.cpp
        src/http.h
        src/http.cpp
        src/http_parser.h
        src/http_parser.cpp
        src/http_utils.h
        src/http_utils.cpp
        src/server.h
        src/server.cpp
        src/str_utils.h
        src/str_utils.cpp
        src/utils.h
        src/utils.cpp
        src/main.cpp
)
