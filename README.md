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
- **IPC Coordination:** Uses pipes/message queues for the Reception to communicate with Kitchens.
- **Thread Safety:** Implements mutexes and condition variables to manage shared ingredient stocks safely.

## Installation

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

Once running, type your pizza orders into the reception prompt (e.g., `margarita XXL 2; regina M 1`) and watch the kitchens coordinate to bake them!
