#include "HttpResponse.h"

HttpResponse::HttpResponse()
    : m_statusCode(200), m_statusText("OK") {}

void HttpResponse::setStatus(int code, const std::string& text) {
    m_statusCode = code;
    m_statusText = text;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    m_headers[key] = value;
}

void HttpResponse::setBody(const std::string& body) {
    m_body = body;
}

std::string HttpResponse::build() {
    if (m_headers.find("Content-Length") == m_headers.end()) {
        m_headers["Content-Length"] = std::to_string(m_body.size());
    }

    std::string response = "HTTP/1.1 " + std::to_string(m_statusCode) +
                           " " + m_statusText + "\r\n";
    for (const auto& [key, value] : m_headers) {
        response += key + ": " + value + "\r\n";
    }
    response += "\r\n";
    response += m_body;
    return response;
}

HttpResponse& HttpResponse::ok(const std::string& body) {
    setStatus(200, "OK");
    setBody(body);
    return *this;
}

HttpResponse& HttpResponse::notFound() {
    setStatus(404, "Not Found");
    setBody("404 Not Found");
    return *this;
}

HttpResponse& HttpResponse::badRequest() {
    setStatus(400, "Bad Request");
    setBody("400 Bad Request");
    return *this;
}

HttpResponse& HttpResponse::serverError() {
    setStatus(500, "Internal Server Error");
    setBody("500 Internal Server Error");
    return *this;
}