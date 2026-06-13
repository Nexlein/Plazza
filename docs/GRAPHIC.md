# Graphical mode (libcaca bonus)

Plazza ships an optional real-time graphical view of the pizzeria, built on
[libcaca](https://caca.zoy.org/wiki/libcaca). It is **opt-in**: the default
three-argument invocation keeps the original text shell, so existing scripts and
tests are unaffected.

## Running

```
./plazza <multiplier> <cooks_per_kitchen> <restock_ms>          # text shell (default)
./plazza <multiplier> <cooks_per_kitchen> <restock_ms> -g       # graphical mode
./plazza <multiplier> <cooks_per_kitchen> <restock_ms> --graphic
```

## Controls

| Key            | Action                                             |
|----------------|----------------------------------------------------|
| printable keys | type a pizza order into the `order>` line          |
| `Enter`        | submit the order (same grammar as the text shell)  |
| `Backspace`    | edit the order line                                |
| `ESC`          | quit (also closing the window)                     |

`q` is **not** a quit key here so it can be typed in an order; use `ESC`.
Typing `exit`/`quit` + `Enter` also quits.

## What it shows

```
 PLAZZA  live pizzeria                              ESC quit | type order + Enter

,--------------------------------. ,--------------------------------.
|Kitchen #0  PID 78873           | |Kitchen #1  PID 78874           |
|cooks 3/3 busy   queue 3        | |cooks 1/3 busy   queue 0        |
|c0 regina XL    [######----] 55%| |c0 margarita M  [#####-----] 50%|
|c1 regina XL    [######----] 55%| |c1 idle                         |
|c2 regina XL    [######----] 55%| |c2 idle                         |
|stk Do3 To3 Gr3 Ha3 Mu3 St6 Eg6 | |stk Do4 To4 Gr4 Ha5 Mu5 St5 Eg5 |
`--------------------------------' `--------------------------------'

-- events ------------------------------------------------------------------------
 accepted 5 pizza(s): regina XL x5
 order #1: regina XL done (1/5)
order> _
```

- One panel per kitchen (PID, busy/total cooks, queue depth).
- One line per cook: the pizza it is baking and a live progress bar (green while
  baking, yellow while waiting on ingredients, grey when idle).
- Ingredient stock (`Do To Gr Ha Mu St Eg Gc Cl`), shown red when depleted.
- A scrolling event log fed by pizza/order completions.

## How it works

The renderer is a thin view layer; it adds no business logic.

- `Reception::pollStatus()` queries every kitchen (the existing
  `StatusQuery`/`StatusReply` IPC, extended with per-cook state) **under the
  registry lock**, so the reaper cannot free a kitchen mid-query, and returns a
  plain `KitchenStatus` snapshot — no live handles cross threads.
- A background poller thread refreshes that snapshot ~4×/s; the main thread
  redraws ~30 fps and interpolates each bake bar locally between polls, so the
  bars move smoothly even at a low poll rate. All `caca_*` calls stay on the
  main thread.
- Orders typed in the canvas are parsed by the same `Parser` and submitted
  through the same queue the text shell uses.

## Dependency

libcaca is required to build and run:

- macOS: `brew install libcaca`
- Fedora: `sudo dnf install libcaca-devel`
- Debian/Ubuntu: `sudo apt install libcaca-dev`

The build discovers it via `pkg-config` (`Makefile` and `CMakeLists.txt`), with a
plain `-lcaca` fallback. libcaca auto-selects a terminal driver (ncurses/slang);
set `CACA_DRIVER` to override.
