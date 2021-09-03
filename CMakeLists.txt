cmake_minimum_required(VERSION 3.20)
project(nastranToJson)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.65 REQUIRED)

include_directories(include)

set(SRCS src/pystring.cpp
        src/nastranToJson.cpp)

add_executable(nastranToJson main.cpp ${SRCS})