##
## EPITECH PROJECT, 2026
## Plazza
## File description:
## Makefile
##

NAME = plazza
TEST_NAME = unit_tests

SRC = src/main.cpp \
      $(wildcard src/*/*.cpp)
OBJ = $(SRC:.cpp=.o)

TEST_SRC = $(wildcard tests/*/*.cpp)

CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Iinclude -Wno-c2y-extensions
LDFLAGS = -lpthread

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

# Build the test binary directly to avoid contaminating standard build object (.o) files with coverage flags
$(TEST_NAME): $(TEST_SRC) $(filter-out src/main.cpp, $(SRC))
	$(CXX) -o $(TEST_NAME) $(filter-out src/main.cpp, $(SRC)) $(TEST_SRC) $(CXXFLAGS) --coverage $(LDFLAGS) -lCatch2Main -lCatch2

tests_run: $(TEST_NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
	rm -f $(TEST_NAME)
	rm -f $(wildcard *.gcda) $(wildcard *.gcno)
	rm -f $(wildcard tests/*/*.gcda) $(wildcard tests/*/*.gcno)

re: fclean all

.PHONY: all clean fclean re tests_run
