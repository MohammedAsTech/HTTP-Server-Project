#include "Handler.h"
#include <filesystem>
#include <fstream>
#include <sstream>

StaticFileHandler::StaticFileHandler(const std::string& rootDir) : m_rootDir(rootDir) {}

void StaticFileHandler::handle(const HttpRequest& req, HttpResponse& res) {
    if (req.path.find("..") != std::string::npos) {
        res.setStatus(403, "Forbidden");
        res.setBody("403 Forbidden");
        return;
    }

    std::filesystem::path filePath = std::filesystem::path(m_rootDir) / req.path.substr(1);

    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
        res.notFound();
        return;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        res.serverError();
        return;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string body = ss.str();

    std::string ext = filePath.extension().string();
    res.setStatus(200, "OK");
    res.setHeader("Content-Type", getMimeType(ext));
    res.setBody(body);
}

std::string StaticFileHandler::getMimeType(const std::string& ext) {
    if (ext == ".html") return "text/html";
    if (ext == ".css")  return "text/css";
    if (ext == ".js")   return "application/javascript";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")  return "image/gif";
    if (ext == ".txt")  return "text/plain";
    if (ext == ".json") return "application/json";
    return "application/octet-stream";
}