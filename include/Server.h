#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <memory>
#include "Router.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "Handler.h"
#include "ThreadPool.h"

class Server {
public:
    Server(const std::string& port);
    ~Server();
    void start();

private:
    std::string m_port;
    int m_server_fd;
    Router m_router;
    ThreadPool m_threadPool;
    void setupSocket();
    void acceptLoop();
    void handleClient(int client_fd);
};
#endif