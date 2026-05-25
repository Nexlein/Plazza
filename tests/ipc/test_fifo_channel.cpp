/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_fifo_channel
*/

#include <numeric>

#include <catch2/catch_test_macros.hpp>

#include "plazza/core/Process.hpp"
#include "plazza/ipc/FifoChannel.hpp"

using namespace plazza::core;
using namespace plazza::ipc;

TEST_CASE("FifoChannel: round-trip frame across forked child", "[fifo]")
{
    const std::string base = "/tmp/plazza_test_fifo_rt";
    FifoChannel::create(base);

    std::vector<uint8_t> payload = {1, 2, 3, 42, 255};

    Process child([base, payload] {
        FifoChannel peer(base, FifoChannel::Role::Peer);
        peer.send(payload);
        return 0;
    });

    FifoChannel owner(base, FifoChannel::Role::Owner);
    auto received = owner.recv();

    REQUIRE(received.has_value());
    CHECK(*received == payload);
    CHECK(child.wait() == 0);
}

TEST_CASE("FifoChannel: closing write end surfaces EOF on reader", "[fifo]")
{
    const std::string base = "/tmp/plazza_test_fifo_eof";
    FifoChannel::create(base);

    Process child([base] {
        FifoChannel peer(base, FifoChannel::Role::Peer);
        peer.send({10, 20, 30});
        return 0;  // destructor closes write end → EOF on owner's read
    });

    FifoChannel owner(base, FifoChannel::Role::Owner);

    auto first = owner.recv();
    REQUIRE(first.has_value());
    CHECK(*first == std::vector<uint8_t>{10, 20, 30});

    child.wait();  // guarantees child FDs are closed before we read again
    auto eof = owner.recv();
    CHECK(!eof.has_value());
}

TEST_CASE("FifoChannel: large payload forces partial read/write loops", "[fifo]")
{
    const std::string base = "/tmp/plazza_test_fifo_large";
    FifoChannel::create(base);

    std::vector<uint8_t> big(65536);
    std::iota(big.begin(), big.end(), uint8_t(0));

    Process child([base, big] {
        FifoChannel peer(base, FifoChannel::Role::Peer);
        peer.send(big);
        return 0;
    });

    FifoChannel owner(base, FifoChannel::Role::Owner);
    auto received = owner.recv();

    REQUIRE(received.has_value());
    CHECK(*received == big);
    CHECK(child.wait() == 0);
}

TEST_CASE("FifoChannel: bidirectional exchange in same round-trip", "[fifo]")
{
    const std::string base = "/tmp/plazza_test_fifo_bidir";
    FifoChannel::create(base);

    std::vector<uint8_t> toChild = {0xAA, 0xBB};
    std::vector<uint8_t> toParent = {0xCC, 0xDD};

    Process child([base, toChild, toParent] {
        FifoChannel peer(base, FifoChannel::Role::Peer);
        auto got = peer.recv();
        if (!got.has_value() || *got != toChild) return 1;
        peer.send(toParent);
        return 0;
    });

    FifoChannel owner(base, FifoChannel::Role::Owner);
    owner.send(toChild);
    auto reply = owner.recv();

    REQUIRE(reply.has_value());
    CHECK(*reply == toParent);
    CHECK(child.wait() == 0);
}
