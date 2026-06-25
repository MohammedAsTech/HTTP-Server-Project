#ifndef ROUTER_H
#define ROUTER_H
#include "Handler.h"
#include <map>
#include <string>
#include <memory>

class Router {
public:
    Router();
    void addRoute(const std::string& method, const std::string& path, std::shared_ptr<Handler> handler);
    Handler* dispatch(const HttpRequest& req);

private:
    std::map<std::pair<std::string, std::string>, std::shared_ptr<Handler>> m_routes;
    std::shared_ptr<Handler> m_notFoundHandler;
};
#endif