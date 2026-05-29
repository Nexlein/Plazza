/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** logger
*/

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "plazza/reception/Logger.hpp"

using plazza::reception::Logger;

namespace {

std::string readFile(const std::filesystem::path& p)
{
    std::ifstream in(p);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

class LoggerTest : public ::testing::Test {
    protected:
    std::filesystem::path logPath;

    void SetUp() override
    {
        logPath = std::filesystem::temp_directory_path() /
                  ("plazza_test_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                   "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".log");
        std::filesystem::remove(logPath);
    }

    void TearDown() override { std::filesystem::remove(logPath); }
};

}  // namespace

TEST_F(LoggerTest, WritesInfoLineToFile)
{
    {
        Logger logger(logPath.string(), /*mirrorStdout=*/false);
        logger.info("hello");
    }
    const auto contents = readFile(logPath);
    EXPECT_NE(contents.find("hello"), std::string::npos);
    EXPECT_NE(contents.find("[info]"), std::string::npos);
}

TEST_F(LoggerTest, TagsLevels)
{
    {
        Logger logger(logPath.string(), /*mirrorStdout=*/false);
        logger.info("i");
        logger.warn("w");
        logger.error("e");
    }
    const auto contents = readFile(logPath);
    EXPECT_NE(contents.find("[info]"), std::string::npos);
    EXPECT_NE(contents.find("[warn]"), std::string::npos);
    EXPECT_NE(contents.find("[error]"), std::string::npos);
}

TEST_F(LoggerTest, TimestampPrefixed)
{
    {
        Logger logger(logPath.string(), /*mirrorStdout=*/false);
        logger.info("payload");
    }
    const auto contents = readFile(logPath);
    ASSERT_FALSE(contents.empty());
    EXPECT_EQ(contents.front(), '[');
    const auto closeBracket = contents.find(']');
    ASSERT_NE(closeBracket, std::string::npos);
    EXPECT_GT(closeBracket, 10u);
}
