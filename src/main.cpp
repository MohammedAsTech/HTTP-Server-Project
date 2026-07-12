#include "Server.h"
#include <iostream>
#include <csignal>

Server* g_server = nullptr;

void handleSignal(int signal) {
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::string port = "8080";
    if (argc >= 2) {
        port = argv[1];
    }

    Server server(port);
    g_server = &server;
    std::signal(SIGINT, handleSignal);

    server.start();
    return 0;
}