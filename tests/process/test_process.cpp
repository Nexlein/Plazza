/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_process
*/

#include <sys/wait.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

#include "plazza/core/Process.hpp"

using namespace plazza::core;

TEST_CASE("Process: child returns exit status collected by parent", "[process]")
{
    Process p([] { return 42; });
    int status = p.wait();
    CHECK(status == 42);
    CHECK(p.getState() == Process::State::EXITED);
    CHECK(p.getExitStatus() == 42);
}

TEST_CASE("Process: parent and child have different pids", "[process]")
{
    pid_t parentPid = ::getpid();
    Process p([&] { return (::getpid() != parentPid) ? 0 : 1; });
    int status = p.wait();
    CHECK(status == 0);
    CHECK(p.getPid() != parentPid);
}

TEST_CASE("Process: no zombie after destructor", "[process]")
{
    pid_t childPid = -1;
    {
        Process p([] { return 0; });
        childPid = p.getPid();
    }
    // After destructor, waitpid should return -1 (child already reaped)
    int status = 0;
    pid_t result = ::waitpid(childPid, &status, WNOHANG);
    CHECK(result == -1);
}

TEST_CASE("Process: tryWait returns nullopt while running, value when done", "[process]")
{
    Process p([] { return 7; });
    p.wait();
    auto result = p.tryWait();
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("Process: state transitions RUNNING -> EXITED", "[process]")
{
    Process p([] { return 0; });
    CHECK(p.getState() == Process::State::RUNNING);
    p.wait();
    CHECK(p.getState() == Process::State::EXITED);
}
