cmake_minimum_required(VERSION 3.10)
project(packet_checker)

set(CMAKE_CXX_STANDARD 14)

add_executable(client client.cpp)
add_executable(server server.cpp)
add_executable(proxy1 proxy_forward_to_server.cpp)
add_executable(proxy2 proxy_modify_payload.cpp)