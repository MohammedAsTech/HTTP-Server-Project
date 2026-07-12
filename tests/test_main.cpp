#include <cassert>
#include <iostream>
#include <string>
#include "../include/HttpParser.h"
#include "../include/HttpResponse.h"
#include "../include/Router.h"
#include "../include/Handler.h"

int PASS = 0;
int FAIL = 0;

void check(const std::string& description, bool condition) {
    if (condition) {
        std::cout << "\033[32mPASS\033[0m " << description << "\n";
        PASS++;
    } else {
        std::cout << "\033[31mFAIL\033[0m " << description << "\n";
        FAIL++;
    }
}

// ─── parser tests ─────────────────────────────────────────────────────────────

void test_parser_get_basic() {
    HttpRequest req = HttpParser::parse("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    check("parser: GET / valid", req.valid);
    check("parser: GET / method", req.method == "GET");
    check("parser: GET / path", req.path == "/");
    check("parser: GET / version", req.version == "HTTP/1.1");
}

void test_parser_get_with_path() {
    HttpRequest req = HttpParser::parse("GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n");
    check("parser: GET /index.html valid", req.valid);
    check("parser: GET /index.html method", req.method == "GET");
    check("parser: GET /index.html path", req.path == "/index.html");
    check("parser: GET /index.html version", req.version == "HTTP/1.1");
}

void test_parser_post_with_body() {
    HttpRequest req = HttpParser::parse("POST /echo HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello");
    check("parser: POST valid", req.valid);
    check("parser: POST method", req.method == "POST");
    check("parser: POST path", req.path == "/echo");
    check("parser: POST body", req.body == "hello");
}

void test_parser_headers_parsed() {
    HttpRequest req = HttpParser::parse("GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test\r\nAccept: */*\r\n\r\n");
    check("parser: headers Host present", req.headers.count("Host") > 0);
    check("parser: headers Host value", req.headers["Host"] == "localhost");
    check("parser: headers User-Agent present", req.headers.count("User-Agent") > 0);
    check("parser: headers User-Agent value", req.headers["User-Agent"] == "test");
    check("parser: headers Accept present", req.headers.count("Accept") > 0);
}

void test_parser_content_length_trim() {
    HttpRequest req = HttpParser::parse("POST /echo HTTP/1.1\r\nContent-Length: 3\r\n\r\nhelloextra");
    check("parser: Content-Length trim", req.body == "hel");
}

void test_parser_empty_body() {
    HttpRequest req = HttpParser::parse("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    check("parser: empty body is empty string", req.body.empty());
}

void test_parser_malformed_no_blank_line() {
    HttpRequest req = HttpParser::parse("GET / HTTP/1.1\r\nHost: localhost");
    check("parser: malformed no blank line invalid", !req.valid);
}

void test_parser_malformed_garbage() {
    HttpRequest req = HttpParser::parse("GARBAGE");
    check("parser: garbage invalid", !req.valid);
}

void test_parser_malformed_empty() {
    HttpRequest req = HttpParser::parse("");
    check("parser: empty string invalid", !req.valid);
}

void test_parser_malformed_partial_request_line() {
    HttpRequest req = HttpParser::parse("GET\r\n\r\n");
    check("parser: partial request line invalid", !req.valid);
}

void test_parser_post_no_body() {
    HttpRequest req = HttpParser::parse("POST /echo HTTP/1.1\r\nHost: localhost\r\n\r\n");
    check("parser: POST no body valid", req.valid);
    check("parser: POST no body empty", req.body.empty());
}

void test_parser_multiple_headers() {
    HttpRequest req = HttpParser::parse("GET / HTTP/1.1\r\nHost: localhost\r\nX-A: 1\r\nX-B: 2\r\nX-C: 3\r\n\r\n");
    check("parser: multiple headers X-A", req.headers["X-A"] == "1");
    check("parser: multiple headers X-B", req.headers["X-B"] == "2");
    check("parser: multiple headers X-C", req.headers["X-C"] == "3");
}

// ─── response builder tests ───────────────────────────────────────────────────

void test_response_200_ok() {
    HttpResponse res;
    res.ok("Hello");
    std::string built = res.build();
    check("response: 200 OK status line", built.find("200 OK") != std::string::npos);
    check("response: 200 OK body", built.find("Hello") != std::string::npos);
    check("response: 200 OK Content-Length", built.find("Content-Length: 5") != std::string::npos);
}

void test_response_404_not_found() {
    HttpResponse res;
    res.notFound();
    std::string built = res.build();
    check("response: 404 Not Found status line", built.find("404 Not Found") != std::string::npos);
    check("response: 404 Not Found body", built.find("404 Not Found") != std::string::npos);
}

void test_response_400_bad_request() {
    HttpResponse res;
    res.badRequest();
    std::string built = res.build();
    check("response: 400 Bad Request status line", built.find("400 Bad Request") != std::string::npos);
}

void test_response_500_server_error() {
    HttpResponse res;
    res.serverError();
    std::string built = res.build();
    check("response: 500 Internal Server Error", built.find("500 Internal Server Error") != std::string::npos);
}

void test_response_custom_status() {
    HttpResponse res;
    res.setStatus(403, "Forbidden");
    res.setBody("403 Forbidden");
    std::string built = res.build();
    check("response: 403 Forbidden status", built.find("403 Forbidden") != std::string::npos);
}

void test_response_custom_header() {
    HttpResponse res;
    res.ok("body");
    res.setHeader("Content-Type", "text/html");
    std::string built = res.build();
    check("response: custom Content-Type header", built.find("Content-Type: text/html") != std::string::npos);
}

void test_response_auto_content_length() {
    HttpResponse res;
    res.ok("Hello World");
    std::string built = res.build();
    check("response: auto Content-Length correct", built.find("Content-Length: 11") != std::string::npos);
}

void test_response_empty_body() {
    HttpResponse res;
    res.ok("");
    std::string built = res.build();
    check("response: empty body Content-Length 0", built.find("Content-Length: 0") != std::string::npos);
}

void test_response_blank_line_separator() {
    HttpResponse res;
    res.ok("body");
    std::string built = res.build();
    check("response: blank line separator present", built.find("\r\n\r\n") != std::string::npos);
}

void test_response_http_version() {
    HttpResponse res;
    res.ok("x");
    std::string built = res.build();
    check("response: HTTP/1.1 version", built.find("HTTP/1.1") != std::string::npos);
}

// ─── router tests ─────────────────────────────────────────────────────────────

void test_router_dispatch_hello() {
    Router router;
    router.addRoute("GET", "/", std::make_shared<HelloHandler>());
    HttpRequest req;
    req.method = "GET";
    req.path = "/";
    req.valid = true;
    Handler* handler = router.dispatch(req);
    HttpResponse res;
    handler->handle(req, res);
    check("router: GET / dispatches to HelloHandler", res.build().find("200") != std::string::npos);
    check("router: GET / body correct", res.build().find("Hello from handler") != std::string::npos);
}

void test_router_dispatch_not_found() {
    Router router;
    HttpRequest req;
    req.method = "GET";
    req.path = "/nonexistent";
    req.valid = true;
    Handler* handler = router.dispatch(req);
    HttpResponse res;
    handler->handle(req, res);
    check("router: unknown path returns 404", res.build().find("404") != std::string::npos);
}

void test_router_method_mismatch() {
    Router router;
    router.addRoute("GET", "/", std::make_shared<HelloHandler>());
    HttpRequest req;
    req.method = "POST";
    req.path = "/";
    req.valid = true;
    Handler* handler = router.dispatch(req);
    HttpResponse res;
    handler->handle(req, res);
    check("router: POST on GET route returns 404", res.build().find("404") != std::string::npos);
}

void test_router_multiple_routes() {
    Router router;
    router.addRoute("GET", "/", std::make_shared<HelloHandler>());
    router.addRoute("POST", "/echo", std::make_shared<EchoHandler>());

    HttpRequest req1;
    req1.method = "GET";
    req1.path = "/";
    req1.valid = true;
    HttpResponse res1;
    router.dispatch(req1)->handle(req1, res1);
    check("router: multiple routes GET /", res1.build().find("200") != std::string::npos);

    HttpRequest req2;
    req2.method = "POST";
    req2.path = "/echo";
    req2.body = "test";
    req2.valid = true;
    HttpResponse res2;
    router.dispatch(req2)->handle(req2, res2);
    check("router: multiple routes POST /echo", res2.build().find("200") != std::string::npos);
}

// ─── handler tests ────────────────────────────────────────────────────────────

void test_echo_handler_mirrors_body() {
    HttpRequest req;
    req.method = "POST";
    req.path = "/echo";
    req.body = "hello world";
    req.valid = true;
    HttpResponse res;
    EchoHandler handler;
    handler.handle(req, res);
    check("echo: body mirrored", res.build().find("hello world") != std::string::npos);
}

void test_echo_handler_mirrors_content_type() {
    HttpRequest req;
    req.method = "POST";
    req.path = "/echo";
    req.body = "{}";
    req.headers["Content-Type"] = "application/json";
    req.valid = true;
    HttpResponse res;
    EchoHandler handler;
    handler.handle(req, res);
    check("echo: Content-Type mirrored", res.build().find("application/json") != std::string::npos);
}

void test_echo_handler_empty_body() {
    HttpRequest req;
    req.method = "POST";
    req.path = "/echo";
    req.body = "";
    req.valid = true;
    HttpResponse res;
    EchoHandler handler;
    handler.handle(req, res);
    check("echo: empty body returns 200", res.build().find("200") != std::string::npos);
}

void test_not_found_handler() {
    HttpRequest req;
    HttpResponse res;
    NotFoundHandler handler;
    handler.handle(req, res);
    check("not found handler: returns 404", res.build().find("404 Not Found") != std::string::npos);
}

void test_hello_handler() {
    HttpRequest req;
    HttpResponse res;
    HelloHandler handler;
    handler.handle(req, res);
    check("hello handler: returns 200", res.build().find("200") != std::string::npos);
    check("hello handler: body correct", res.build().find("Hello from handler") != std::string::npos);
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << " HTTP Parser Tests\n";
    std::cout << "══════════════════════════════════════════════════\n";
    test_parser_get_basic();
    test_parser_get_with_path();
    test_parser_post_with_body();
    test_parser_headers_parsed();
    test_parser_content_length_trim();
    test_parser_empty_body();
    test_parser_malformed_no_blank_line();
    test_parser_malformed_garbage();
    test_parser_malformed_empty();
    test_parser_malformed_partial_request_line();
    test_parser_post_no_body();
    test_parser_multiple_headers();

    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << " HttpResponse Builder Tests\n";
    std::cout << "══════════════════════════════════════════════════\n";
    test_response_200_ok();
    test_response_404_not_found();
    test_response_400_bad_request();
    test_response_500_server_error();
    test_response_custom_status();
    test_response_custom_header();
    test_response_auto_content_length();
    test_response_empty_body();
    test_response_blank_line_separator();
    test_response_http_version();

    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << " Router Tests\n";
    std::cout << "══════════════════════════════════════════════════\n";
    test_router_dispatch_hello();
    test_router_dispatch_not_found();
    test_router_method_mismatch();
    test_router_multiple_routes();

    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << " Handler Tests\n";
    std::cout << "══════════════════════════════════════════════════\n";
    test_echo_handler_mirrors_body();
    test_echo_handler_mirrors_content_type();
    test_echo_handler_empty_body();
    test_not_found_handler();
    test_hello_handler();

    std::cout << "\n══════════════════════════════════════════════════\n";
    std::cout << " RESULTS\n";
    std::cout << "══════════════════════════════════════════════════\n";
    std::cout << "Total:  " << (PASS + FAIL) << "\n";
    std::cout << "\033[32mPassed: " << PASS << "\033[0m\n";
    std::cout << "\033[31mFailed: " << FAIL << "\033[0m\n\n";

    if (FAIL == 0) {
        std::cout << "\033[32mAll unit tests passed.\033[0m\n\n";
        return 0;
    } else {
        std::cout << "\033[31m" << FAIL << " test(s) failed.\033[0m\n\n";
        return 1;
    }
}