cmake_minimum_required(VERSION 3.13)
project(http-server)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(server)

target_sources(server
    PRIVATE
        src/file_descriptor.cpp
        src/file_descriptor.h
        src/get_echo_http_handler.cpp
        src/get_echo_http_handler.h
        src/get_post_file_http_handler.cpp
        src/get_post_file_http_handler.h
        src/get_root_http_handler.cpp
        src/get_root_http_handler.h
        src/get_user_agent_http_handler.cpp
        src/get_user_agent_http_handler.h
        src/http.cpp
        src/http.h
        src/http_handler_base.cpp
        src/http_handler_base.h
        src/http_parser.cpp
        src/http_parser.h
        src/http_utils.cpp
        src/http_utils.h
        src/main.cpp
        src/server.cpp
        src/server.h
        src/str_utils.cpp
        src/str_utils.h
        src/utils.cpp
        src/utils.h
)
