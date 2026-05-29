/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** shell
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <sstream>

#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Shell.hpp"

using plazza::reception::Logger;
using plazza::reception::Shell;

namespace {

std::filesystem::path tmpLog(const std::string& name)
{
    return std::filesystem::temp_directory_path() / ("plazza_shell_" + name + ".log");
}

}  // namespace

TEST(Shell, ExitsOnEof)
{
    const auto path = tmpLog("eof");
    std::filesystem::remove(path);
    Logger logger(path.string(), false);
    std::istringstream in("");
    std::ostringstream out;
    Shell shell(logger, in, out);
    EXPECT_EQ(shell.run(), 0);
    std::filesystem::remove(path);
}

TEST(Shell, ExitsOnExitCommand)
{
    const auto path = tmpLog("exit");
    std::filesystem::remove(path);
    Logger logger(path.string(), false);
    std::istringstream in("exit\n");
    std::ostringstream out;
    Shell shell(logger, in, out);
    EXPECT_EQ(shell.run(), 0);
    std::filesystem::remove(path);
}

TEST(Shell, AcceptsValidOrder)
{
    const auto path = tmpLog("order");
    std::filesystem::remove(path);
    Logger logger(path.string(), false);
    std::istringstream in("regina XXL x2; fantasia M x3\nexit\n");
    std::ostringstream out;
    Shell shell(logger, in, out);
    EXPECT_EQ(shell.run(), 0);
    const auto output = out.str();
    EXPECT_NE(output.find("5 pizza"), std::string::npos);
    std::filesystem::remove(path);
}

TEST(Shell, RejectsBadOrderWithoutCrash)
{
    const auto path = tmpLog("bad");
    std::filesystem::remove(path);
    Logger logger(path.string(), false);
    std::istringstream in("regina XXL x0\nexit\n");
    std::ostringstream out;
    Shell shell(logger, in, out);
    EXPECT_EQ(shell.run(), 0);
    const auto output = out.str();
    EXPECT_NE(output.find("error"), std::string::npos);
    std::filesystem::remove(path);
}

TEST(Shell, StatusCommand)
{
    const auto path = tmpLog("status");
    std::filesystem::remove(path);
    Logger logger(path.string(), false);
    std::istringstream in("status\nexit\n");
    std::ostringstream out;
    Shell shell(logger, in, out);
    EXPECT_EQ(shell.run(), 0);
    const auto output = out.str();
    EXPECT_NE(output.find("no kitchens"), std::string::npos);
    std::filesystem::remove(path);
}

TEST(Shell, EmptyLineIgnored)
{
    const auto path = tmpLog("empty");
    std::filesystem::remove(path);
    Logger logger(path.string(), false);
    std::istringstream in("\n\nexit\n");
    std::ostringstream out;
    Shell shell(logger, in, out);
    EXPECT_EQ(shell.run(), 0);
    const auto output = out.str();
    EXPECT_EQ(output.find("error"), std::string::npos);
    std::filesystem::remove(path);
}
