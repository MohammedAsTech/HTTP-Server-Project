#include "Router.h"

Router::Router() : m_notFoundHandler(std::make_shared<NotFoundHandler>()) {}

void Router::addRoute(const std::string& method, const std::string& path, std::shared_ptr<Handler> handler) {
    m_routes[{method, path}] = handler;
}

Handler* Router::dispatch(const HttpRequest& req) {
    auto it = m_routes.find({req.method, req.path});
    if (it != m_routes.end()) {
        return it->second.get();
    }
    return m_notFoundHandler.get();
}