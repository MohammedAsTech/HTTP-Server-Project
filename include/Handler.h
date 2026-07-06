#ifndef HANDLER_H
#define HANDLER_H
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
#endif