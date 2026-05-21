/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Logger
*/

#pragma once

#include <fstream>
#include <string>
#include <string_view>

namespace plazza::reception {

    class Logger {
        public:
        Logger(const std::string& path, bool mirrorStdout = true);
        ~Logger();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void info(std::string_view message);
        void warn(std::string_view message);
        void error(std::string_view message);

        private:
        std::ofstream _file;
        bool _mirrorStdout;

        void write(std::string_view level, std::string_view message);
    };

}  // namespace plazza::reception
