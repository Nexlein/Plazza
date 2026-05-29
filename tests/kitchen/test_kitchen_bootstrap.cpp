/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_kitchen_bootstrap
*/

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <future>
#include <string>

#include "plazza/protocol/Frame.hpp"
#include "plazza/reception/KitchenHandle.hpp"

using namespace plazza::kitchen;
using namespace plazza::protocol;
using namespace plazza::reception;
using namespace std::chrono_literals;

namespace {

    bool fifoExists(const std::string& path)
    {
        struct stat st;
        return ::stat(path.c_str(), &st) == 0;
    }

}  // namespace

TEST_CASE("KitchenHandle: spawn, ping/pong, clean shutdown", "[kitchen]")
{
    const std::string base = "/tmp/plazza_test_kitchen_boot";
    Kitchen::Config cfg{/*cookCount=*/1, /*cookingMultiplier=*/1.0F, /*restockMs=*/1000, base};

    std::promise<AckFrame> ackPromise;
    auto ackFuture = ackPromise.get_future();

    {
        KitchenHandle handle(cfg, /*id=*/0);
        CHECK(handle.pid() > 0);

        handle.onFrame([&](const AnyFrame& f) {
            if (std::holds_alternative<AckFrame>(f)) ackPromise.set_value(std::get<AckFrame>(f));
        });

        handle.sendFrame(HeartbeatFrame{/*token=*/0xDEADBEEFu});

        REQUIRE(ackFuture.wait_for(2s) == std::future_status::ready);
        AckFrame ack = ackFuture.get();
        CHECK(ack.token == 0xDEADBEEFu);

        handle.shutdown();
    }

    // No leftover FIFOs after Owner destruction.
    CHECK(!fifoExists(base + ".fwd"));
    CHECK(!fifoExists(base + ".bwd"));
}

TEST_CASE("KitchenHandle: idempotent shutdown via destructor", "[kitchen]")
{
    const std::string base = "/tmp/plazza_test_kitchen_dtor";
    Kitchen::Config cfg{/*cookCount=*/1, /*cookingMultiplier=*/1.0F, /*restockMs=*/1000, base};

    pid_t childPid = -1;
    {
        KitchenHandle handle(cfg, /*id=*/1);
        childPid = handle.pid();
        // No explicit shutdown — rely on destructor.
    }

    // Child reaped (waitpid returns -1 since process already gone).
    int status = 0;
    pid_t result = ::waitpid(childPid, &status, WNOHANG);
    CHECK(result == -1);

    CHECK(!fifoExists(base + ".fwd"));
    CHECK(!fifoExists(base + ".bwd"));
}
