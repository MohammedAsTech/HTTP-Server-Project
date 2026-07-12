#ifndef HANDLER_H
#define HANDLER_H
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "HttpRequest.h"
#include "HttpResponse.h"

class Handler {
public:
    virtual ~Handler() = default;
    virtual void handle(const HttpRequest& req, HttpResponse& res) = 0;
};

class HelloHandler : public Handler {
public:
    void handle(const HttpRequest& req, HttpResponse& res) override {
        res.ok("Hello from handler");
    }
};

class NotFoundHandler : public Handler {
public:
    void handle(const HttpRequest& req, HttpResponse& res) override {
        res.notFound();
    }
};

class StaticFileHandler : public Handler {
public:
    explicit StaticFileHandler(const std::string& rootDir);
    void handle(const HttpRequest& req, HttpResponse& res) override;

private:
    std::string m_rootDir;
    std::string getMimeType(const std::string& ext);
};
class EchoHandler : public Handler {
public:
    void handle(const HttpRequest& req, HttpResponse& res) override {
        // mirror the content type back if the visitor specified one
        auto it = req.headers.find("Content-Type");
        std::string contentType = (it != req.headers.end())
            ? it->second
            : "text/plain";

        res.setStatus(200, "OK");
        res.setHeader("Content-Type", contentType);
        res.setBody(req.body);
    }
};

#endif