#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <string>
#include <map>

class HttpResponse {
public:
    HttpResponse();
    void setStatus(int code, const std::string& text);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    std::string build();

    HttpResponse& ok(const std::string& body = "");
    HttpResponse& notFound();
    HttpResponse& badRequest();
    HttpResponse& serverError();

private:
    int m_statusCode;
    std::string m_statusText;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
};
#endif