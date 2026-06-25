#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <string>

struct HttpResponse {
    int statusCode = 200;
    std::string statusText = "OK";
    std::string body;
};
#endif