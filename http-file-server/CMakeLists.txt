cmake_minimum_required(VERSION 3.10)

project(HttpFileServer VERSION 1.0 DESCRIPTION "Simple HTTP File Server in C++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(server http_file_server.cpp)

target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 필요한 경우 추가 라이브러리 링크 (예: pthread)
# target_link_libraries(server PRIVATE pthread)