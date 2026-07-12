cmake_minimum_required(VERSION 3.28)
project(main)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(my_server
        src/main.cpp
        src/Server.cpp
        src/HttpParser.cpp
        src/Router.cpp
        src/HttpResponse.cpp
        src/StaticFileHandler.cpp
        src/ThreadPool.cpp
        include/HttpRequest.h
        include/HttpParser.h
        include/HttpResponse.h
        include/Handler.h
        include/Router.h
        include/ThreadPool.h
        include/Logger.h
)

target_link_libraries(my_server pthread)