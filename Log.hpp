#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <windows.h>

class Log {
public:
    static void d(const std::string& tag, const std::string& message) {
        print("D", tag, message);
    }

    static void w(const std::string& tag, const std::string& message) {
        print("W", tag, message);
    }

    static void e(const std::string& tag, const std::string& message) {
        print("E", tag, message);
    }

    template <typename... Args>
    static void d(const std::string& tag, Args&&... args) {
        print("D", tag, buildMessage(std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void w(const std::string& tag, Args&&... args) {
        print("W", tag, buildMessage(std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void e(const std::string& tag, Args&&... args) {
        print("E", tag, buildMessage(std::forward<Args>(args)...));
    }

private:
    static void print(const std::string& level, const std::string& tag, const std::string& message) {
        std::ostringstream oss;
        oss << "[" << level << "/"
            << std::left << std::setw(18) << tag << "]  "  // 태그 너비 고정 정렬
            << std::setw(5) << GetCurrentProcessId() << ":"
            << std::setw(5) << GetCurrentThreadId() << "  "
            << currentTime() << "  |  " << message;
        std::cout << oss.str() << std::endl;
    }

    static std::string currentTime() {
        using namespace std::chrono;
        auto now = system_clock::now();
        std::time_t now_time = system_clock::to_time_t(now);
        std::tm local_tm;
#ifdef _WIN32
        localtime_s(&local_tm, &now_time);
#else
        localtime_r(&now_time, &local_tm);
#endif
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << local_tm.tm_hour << ":"
            << std::setfill('0') << std::setw(2) << local_tm.tm_min << ":"
            << std::setfill('0') << std::setw(2) << local_tm.tm_sec;
        return oss.str();
    }

    template <typename... Args>
    static std::string buildMessage(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        return oss.str();
    }
};
