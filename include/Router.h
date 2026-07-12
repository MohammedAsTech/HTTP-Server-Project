#ifndef ROUTER_H
#define ROUTER_H
#include "Handler.h"
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <tuple>

class Router {
public:
    Router();
    void addRoute(const std::string& method, const std::string& path, std::shared_ptr<Handler> handler);
    void addPrefixRoute(const std::string& method, const std::string& prefix, std::shared_ptr<Handler> handler);
    Handler* dispatch(const HttpRequest& req);

private:
    std::map<std::pair<std::string, std::string>, std::shared_ptr<Handler>> m_routes;
    std::vector<std::tuple<std::string, std::string, std::shared_ptr<Handler>>> m_prefixRoutes;
    std::shared_ptr<Handler> m_notFoundHandler;
};
#endif