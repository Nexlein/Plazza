<div align="center">
  <h1>🍕 Plazza</h1>
  <p><strong>A highly concurrent pizzeria simulation in C++</strong></p>
</div>

## Overview

Plazza is a multi-process, multi-threaded simulation of a bustling pizzeria. It was built to explore concurrent architectures, inter-process communication (IPC), and mutex coordination under heavy load.

A main "Reception" process takes pizza orders and distributes them across multiple "Kitchen" processes. Inside each kitchen, individual "Cook" threads work simultaneously to bake the pizzas, managing shared resources like ovens and ingredient stocks without causing race conditions or deadlocks.

## Features

- **Multi-Processing:** Kitchens are spun up dynamically as separate processes using `fork()`.
- **Multi-Threading:** Cooks are represented as concurrent threads within their kitchen.
- **IPC Coordination:** The Reception talks to each Kitchen over a pair of named pipes (FIFOs created with `mkfifo`), one per direction.
- **Thread Safety:** Implements mutexes and condition variables to manage shared ingredient stocks safely.
- **Graphical Mode (optional):** A live terminal view of every kitchen built on libcaca (see [docs/GRAPHIC.md](docs/GRAPHIC.md)).

## Installation

### Requirements

- A C++20 compiler (the Makefile uses `clang++`).
- [libcaca](https://caca.zoy.org/wiki/libcaca) development files, used by the graphical mode and linked in every build: `sudo apt install libcaca-dev pkg-config` on Debian/Ubuntu, `brew install libcaca pkg-config` on macOS.
- For the CMake build: CMake 3.20 or newer.
- For the tests: [Catch2](https://github.com/catchorg/Catch2) v3 (`sudo apt install catch2`). The CMake build downloads it if it is not installed.

The project includes a standard Makefile for fast compilation.

```bash
# Compile the project
make
```

*(Alternatively, you can use `cmake -B build && cmake --build build` if you prefer).*

## How to Run

Launch the pizzeria by defining the rules of the simulation via command line arguments:

```bash
./plazza <multiplier> <cooks_per_kitchen> <time_restock>
```

- **`multiplier`**: Multiplier for the baking time of the pizzas.
- **`cooks_per_kitchen`**: The number of cooks available in a single kitchen.
- **`time_restock`**: The time (in milliseconds) it takes to restock ingredients.

**Example:**

```bash
./plazza 2 5 2000
```

Once running, type your pizza orders into the reception prompt (e.g., `margarita XXL x2; regina M x1`) and watch the kitchens coordinate to bake them!

- **Order format:** `TYPE SIZE xNUMBER`, several orders separated by `;`. Types: `regina`, `margarita`, `americana`, `fantasia`. Sizes: `S`, `M`, `L`, `XL`, `XXL`.
- **`status`:** shows each kitchen's cooks and ingredient stock.
- **`exit`:** closes the reception.

Add `-g` (or `--graphic`) as a fourth argument to open the graphical view instead of the text prompt; see [docs/GRAPHIC.md](docs/GRAPHIC.md).

## Tests

The unit tests use Catch2:

```bash
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure
```

With the Makefile, `make tests_run` builds a single `unit_tests` binary (with coverage flags); run it with `./unit_tests`.
