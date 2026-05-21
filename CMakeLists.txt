cmake_minimum_required(VERSION 3.20)

project(plazza LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

file(GLOB_RECURSE PLAZZA_SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(FILTER PLAZZA_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(plazza_core STATIC ${PLAZZA_SOURCES})
target_include_directories(plazza_core PUBLIC include)
target_link_libraries(plazza_core PUBLIC pthread)

add_executable(plazza src/main.cpp)
target_link_libraries(plazza PRIVATE plazza_core)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()
