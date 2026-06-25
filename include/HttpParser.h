#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include "HttpRequest.h"
#include <string>

class HttpParser {
public:
    static HttpRequest parse(const std::string& raw);
};
#endif