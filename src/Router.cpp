#include "Router.h"

Router::Router() : m_notFoundHandler(std::make_shared<NotFoundHandler>()) {}

void Router::addRoute(const std::string& method, const std::string& path, std::shared_ptr<Handler> handler) {
    m_routes[{method, path}] = handler;
}

void Router::addPrefixRoute(const std::string& method, const std::string& prefix, std::shared_ptr<Handler> handler) {
    m_prefixRoutes.emplace_back(method, prefix, handler);
}

Handler* Router::dispatch(const HttpRequest& req) {
    // exact match first
    auto it = m_routes.find({req.method, req.path});
    if (it != m_routes.end()) return it->second.get();

    // HEAD falls back to GET handler per HTTP/1.1 spec
    if (req.method == "HEAD") {
        auto git = m_routes.find({"GET", req.path});
        if (git != m_routes.end()) return git->second.get();
    }

    // prefix match
    for (const auto& [method, prefix, handler] : m_prefixRoutes) {
        if (req.method == method && req.path.substr(0, prefix.size()) == prefix)
            return handler.get();
        if (req.method == "HEAD" && method == "GET" && req.path.substr(0, prefix.size()) == prefix)
            return handler.get();
    }

    return m_notFoundHandler.get();
}