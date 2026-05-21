/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Logger
*/

#include "plazza/reception/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace plazza::reception {

    namespace {

        std::string timestamp()
        {
            using clock = std::chrono::system_clock;
            const auto now = clock::now();
            const auto t = clock::to_time_t(now);
            const auto millis =
                std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) %
                1000;
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &t);
#else
            localtime_r(&t, &tm);
#endif
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3)
                << std::setfill('0') << millis.count();
            return oss.str();
        }

    }  // namespace

    Logger::Logger(const std::string& path, bool mirrorStdout)
        : _file(path, std::ios::out | std::ios::app), _mirrorStdout(mirrorStdout)
    {
    }

    Logger::~Logger() = default;

    void Logger::info(std::string_view message) { write("info", message); }
    void Logger::warn(std::string_view message) { write("warn", message); }
    void Logger::error(std::string_view message) { write("error", message); }

    void Logger::write(std::string_view level, std::string_view message)
    {
        std::ostringstream line;
        line << '[' << timestamp() << "] [" << level << "] " << message << '\n';
        const std::string text = line.str();
        if (_file.is_open()) {
            _file << text;
            _file.flush();
        }
        if (_mirrorStdout) {
            std::cout << text;
            std::cout.flush();
        }
    }

}  // namespace plazza::reception
