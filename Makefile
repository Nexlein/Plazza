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
DEP = $(OBJ:.o=.d)

TEST_SRC = $(wildcard tests/*/*.cpp)

CXX = clang++

# libcaca (graphical bonus). Discovered via pkg-config so the build works on
# both macOS (Homebrew) and the Linux eval machines; falls back to a bare
# -lcaca if pkg-config has no entry.
CACA_CFLAGS := $(shell pkg-config --cflags caca 2>/dev/null)
CACA_LIBS := $(shell pkg-config --libs caca 2>/dev/null || echo -lcaca)

# -MMD -MP emit a .d file per object listing its header prerequisites, so an
# incremental `make` recompiles every translation unit that includes a changed
# header. Without this, editing a header (e.g. a struct layout in Frame.hpp)
# leaves stale objects compiled against the old layout, producing ABI
# mismatches that corrupt memory at runtime.
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Iinclude -Wno-c2y-extensions -MMD -MP $(CACA_CFLAGS)
LDFLAGS = -lpthread $(CACA_LIBS)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

# Build the test binary directly to avoid contaminating standard build object (.o) files with coverage flags
$(TEST_NAME): $(TEST_SRC) $(filter-out src/main.cpp, $(SRC))
	$(CXX) -o $(TEST_NAME) $(filter-out src/main.cpp, $(SRC)) $(TEST_SRC) $(CXXFLAGS) --coverage $(LDFLAGS) -lCatch2Main -lCatch2

tests_run: $(TEST_NAME)

clean:
	rm -f $(OBJ)
	rm -f $(DEP)

fclean: clean
	rm -f $(NAME)
	rm -f $(TEST_NAME)
	rm -f $(wildcard *.gcda) $(wildcard *.gcno)
	rm -f $(wildcard tests/*/*.gcda) $(wildcard tests/*/*.gcno)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re tests_run
