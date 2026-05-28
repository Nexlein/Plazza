##
## EPITECH PROJECT, 2026
## Plazza
## File description:
## Makefile
##

NAME = plazza

SRC = src/main.cpp \
      $(wildcard src/*/*.cpp)

OBJ = $(SRC:.cpp=.o)

CXX ?= g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS = -lpthread

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
	rm -f unit_tests
	rm -f *.gcda *.gcno

re: fclean all

tests_run: CXXFLAGS += --coverage
tests_run: LDFLAGS += -lCatch2Main -lCatch2
tests_run:
	@echo "Compiling and running unit tests..."
	$(CXX) -o unit_tests $(wildcard tests/*/*.cpp) $(filter-out src/main.cpp, $(SRC)) $(CXXFLAGS) $(LDFLAGS)
	./unit_tests

.PHONY: all clean fclean re tests_run
