# Change from 3.29 to 3.28
cmake_minimum_required(VERSION 3.28)
project(main)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(my_server
        src/main.cpp
        src/Server.cpp
        include/HttpRequest.h
        include/HttpParser.h
        src/HttpParser.cpp
        include/HttpResponse.h
        include/Handler.h
        include/Router.h
        src/Router.cpp
)