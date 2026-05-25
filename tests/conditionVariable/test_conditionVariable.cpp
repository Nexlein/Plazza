/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_conditionVariable
*/

#include "plazza/core/ConditionVariable.hpp"
#include "plazza/core/Mutex.hpp"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>

TEST_CASE("ConditionVariable - notify wakes waiting thread")
{
    plazza::core::Mutex mutex;
    plazza::core::ConditionVariable cv;
    bool ready = false;

    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        mutex.lock();
        ready = true;
        cv.notifyOne();
        mutex.unlock();
    });

    mutex.lock();
    while (!ready)
        cv.wait(mutex);
    mutex.unlock();

    REQUIRE(ready == true);
    producer.join();
}

TEST_CASE("ConditionVariable - timed wait returns on timeout")
{
    plazza::core::Mutex mutex;
    plazza::core::ConditionVariable cv;

    mutex.lock();
    auto start = std::chrono::steady_clock::now();
    cv.waitFor(mutex, std::chrono::milliseconds(200));
    auto elapsed = std::chrono::steady_clock::now() - start;
    mutex.unlock();

    REQUIRE(elapsed >= std::chrono::milliseconds(200));
    REQUIRE(elapsed < std::chrono::milliseconds(400));
}

TEST_CASE("ConditionVariable - notifyAll wakes all waiting threads")
{
    plazza::core::Mutex mutex;
    plazza::core::ConditionVariable cv;
    int woken = 0;

    auto waiter = [&]() {
        mutex.lock();
        cv.wait(mutex);
        woken++;
        mutex.unlock();
    };

    std::thread t1(waiter);
    std::thread t2(waiter);
    std::thread t3(waiter);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    cv.notifyAll();

    t1.join();
    t2.join();
    t3.join();

    REQUIRE(woken == 3);
}

TEST_CASE("ConditionVariable - producer consumer queue")
{
    plazza::core::Mutex mutex;
    plazza::core::ConditionVariable cv;
    std::vector<int> queue;
    bool done = false;
    int consumed = 0;

    std::thread producer([&]() {
        for (int i = 0; i < 5; i++) {
            mutex.lock();
            queue.push_back(i);
            cv.notifyOne();
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex.lock();
        done = true;
        cv.notifyOne();
        mutex.unlock();
    });

    std::thread consumer([&]() {
        while (true) {
            mutex.lock();
            while (queue.empty() && !done)
                cv.wait(mutex);
            while (!queue.empty()) {
                queue.pop_back();
                consumed++;
            }
            bool finished = done;
            mutex.unlock();
            if (finished)
                break;
        }
    });

    producer.join();
    consumer.join();

    REQUIRE(consumed == 5);
}