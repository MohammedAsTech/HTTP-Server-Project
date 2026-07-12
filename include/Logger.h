#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <mutex>
#include <iostream>
#include <chrono>

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void log(const std::string& method,
             const std::string& path,
             int statusCode,
             long long latencyMs) {
        std::string color = colorFor(statusCode);
        std::string reset = "\033[0m";

        std::unique_lock<std::mutex> lock(m_mutex);
        std::cout << color
                  << "[" << method << "] "
                  << path << " -> "
                  << statusCode << " ("
                  << latencyMs << "ms)"
                  << reset << "\n";
    }

    void error(const std::string& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::cout << "\033[31m[ERROR] " << message << "\033[0m\n";
    }

private:
    Logger() = default;
    std::mutex m_mutex;

    std::string colorFor(int statusCode) {
        if (statusCode >= 500) return "\033[31m"; // red
        if (statusCode >= 400) return "\033[33m"; // yellow
        if (statusCode >= 300) return "\033[36m"; // cyan
        return "\033[32m";                         // green
    }
};
#endif