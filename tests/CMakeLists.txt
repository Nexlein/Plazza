cmake_minimum_required(VERSION 3.20)

include(FetchContent)
fetchcontent_declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
fetchcontent_makeavailable(Catch2)

add_executable(test_pizza_recipe pizza/test_recipe.cpp)
target_link_libraries(test_pizza_recipe PRIVATE plazza_core Catch2::Catch2WithMain)

add_executable(test_serial_buffer serialBuffer/test_serialBuffer.cpp)
target_link_libraries(test_serial_buffer PRIVATE plazza_core Catch2::Catch2WithMain)

add_executable(test_process process/test_process.cpp)
target_link_libraries(test_process PRIVATE plazza_core Catch2::Catch2WithMain)

add_executable(test_conditionVariable conditionVariable/test_conditionVariable.cpp)
target_link_libraries(test_conditionVariable PRIVATE plazza_core Catch2::Catch2WithMain)

add_executable(test_fifo_channel ipc/test_fifo_channel.cpp)
target_link_libraries(test_fifo_channel PRIVATE plazza_core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test_pizza_recipe)
catch_discover_tests(test_serial_buffer)
catch_discover_tests(test_process)
catch_discover_tests(test_conditionVariable)
catch_discover_tests(test_fifo_channel)
