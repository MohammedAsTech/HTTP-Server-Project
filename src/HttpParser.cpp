#include "HttpParser.h"
#include <sstream>

HttpRequest HttpParser::parse(const std::string& raw) {
    HttpRequest req;

    size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return req; // valid stays false
    }

    std::string header_block = raw.substr(0, header_end);
    req.body = raw.substr(header_end + 4);

    size_t line_end = header_block.find("\r\n");
    std::string request_line = header_block.substr(0, line_end);

    std::istringstream line_stream(request_line);
    if (!(line_stream >> req.method >> req.path >> req.version)) {
        return req; // malformed request line
    }

    size_t pos = (line_end == std::string::npos) ? header_block.size() : line_end + 2;
    while (pos < header_block.size()) {
        size_t next = header_block.find("\r\n", pos);
        std::string line = (next == std::string::npos)
            ? header_block.substr(pos)
            : header_block.substr(pos, next - pos);

        size_t sep = line.find(": ");
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 2);
            req.headers[key] = value;
        }

        if (next == std::string::npos) break;
        pos = next + 2;
    }

    req.valid = true;
    return req;
}