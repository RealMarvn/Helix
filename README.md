# ♟️ Helix — High‑Performance Chess Engine

**Helix** is a modern C++ chess engine that began as a university programming project (PK1) and evolved into a full,
research‑driven bachelor thesis at the University of Konstanz.  
It is designed as a clean, transparent, and academically rigorous engine that prioritizes correctness and clarity over
low‑level micro‑optimizations — making it an ideal platform for experimenting with search algorithms, evaluation
strategies, and engine architecture.

Rather than relying on complex bitboard tricks, Helix uses a refined **Mailbox representation**, providing exceptional
readability and debuggability while still supporting all essential chess mechanics.  
On top of this foundation, Helix implements a robust **NegaMax + Alpha‑Beta** search framework enhanced with iterative
deepening and move ordering techniques — allowing it to explore game trees efficiently while remaining easy to extend
and reason about.

Helix can be run directly via a simple CLI or integrated into chess GUIs through a basic **UCI protocol** implementation.

Helix supports both **CLI testing mode** and basic **UCI protocol** for integration with chess GUIs like CuteChess or
Banksia.

---

## 📌 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Technical Design](#technical-design)
- [Project Structure](#project-structure)
- [Build & Setup](#build--setup)
- [Common Meson Commands](#common-meson-commands)
- [Usage](#usage)
    - [CLI Mode](#cli-mode)
    - [UCI Mode](#uci-mode)
- [Prerequisites](#prerequisites)
- [Notes](#notes)

---

## Overview

Helix is designed to explore the computational complexity of chess decision‑making.  
It emphasizes correctness, clarity, and a research‑oriented design suitable for algorithmic experimentation and
performance evaluation.  
The engine integrates a modern alpha‑beta search enhanced with transposition tables and well‑established move‑ordering heuristics, allowing meaningful depth and performance experiments without sacrificing code readability.

---

## Features

### ✔️ Move Generation

- **Pseudolegal move generation** (correctness‑oriented; not bitboard‑based)
- Uses a **Mailbox representation** rather than Bitboards
- Full support for:
    - **Castling**
    - **En passant**
    - **Pawn promotion**
- Clear and maintainable data structures ideal for algorithmic experimentation

### ✔️ Search Algorithm

- **NegaMax** search formulation  
  (clean and elegant variant of Minimax for zero‑sum games)
- **Alpha‑Beta pruning** to drastically reduce the search tree
- **Iterative deepening** for stable move ordering and robust time management
- **Transposition Table (TT)** with depth‑sensitive bounds (Exact / Lower / Upper)
    - Avoids re‑searching previously evaluated positions
    - Stores best moves to guide future search (PV / TT move ordering)
- **Advanced move ordering heuristics** to maximize pruning efficiency:
    - **TT / PV move first** (principal variation prioritization)
    - **MVV‑LVA** ordering for captures
    - **Killer move heuristic** (per‑ply quiet moves causing beta cutoffs)
    - **History heuristic** (global quiet‑move statistics accumulated during search)

### ✔️ Engine Modes

- **CLI input** for fast manual testing
- **Basic UCI protocol** for interaction with external GUIs

---

## Technical Design

1. **Board Representation (Mailbox System)**
    - Classic **Mailbox (0×88‑style) representation** for clarity and ease of debugging
    - Tracks castling rights, en passant squares, move history, and full board state
    - Utility functions for FEN parsing and state inspection

2. **Move Generation**
    - Generates pseudolegal moves efficiently
    - Handles all non‑trivial move types (castling, promotions, en passant)

3. **Search System (NegaMax)**
    - Core of the engine built around a recursive NegaMax formulation
    - Enhanced with:
        - **Alpha‑Beta pruning**
        - **Iterative deepening**
        - **Transposition Table** for caching evaluated positions
        - **Heuristic‑driven move ordering**
    - Move ordering is implemented as a dedicated search‑heuristics module and includes:
        - TT / principal‑variation move prioritization
        - Capture ordering (MVV‑LVA)
        - Killer moves (quiet moves remembered per ply)
        - History heuristic (global quiet‑move success statistics)
    - Designed as a clean, modular system suitable for empirical evaluation of search behavior

4. **Evaluation Function**
    - Lightweight material‑based evaluation with piece‑value heuristics
    - Position-based evaluation (PESTO)
    - Easily extendable for future enhancements

---

## Project Structure

```
/engine/   → Core engine, board logic, move generation, search, exceptions
/tests/    → Unit tests for move generation and FEN/state validation
```

---

## Build & Setup

After cloning the repository, run:

```bash
./setup.sh
```

This script:

- Configures `build/debug` and `build/release`
- Builds the engine and (Release‑only) tests
- Installs clang‑format / clang‑tidy Git hooks when applicable

---

## Common Meson Commands

### Build the engine

```bash
meson compile -C build/debug
meson compile -C build/release
```

### Run the engine

```bash
meson compile -C build/debug helix && ./build/debug/helix
```

### Run tests (Release mode only)

```bash
meson test -C build/release
```

### Format and lint

```bash
meson compile -C build/debug format
meson compile -C build/debug format-check
meson compile -C build/debug tidy
```

---

## Usage

### CLI Mode

The CLI mode uses **UCI‑style coordinate notation**:

```
e2e4      # Pawn moves from e2 to e4
b1c3      # Knight moves from b1 to c3
e7e8q     # Pawn promotes to queen (promotion letter always lowercase)
```

- No piece letters required
- The engine infers the moving piece from board state

### UCI Mode

The engine automatically switches to UCI mode when the GUI sends:

```
uci
```

Minimal example:

```
uci
isready
ucinewgame
position startpos
go movetime 2000
bestmove e2e4
```

---

## Notes

- Meson uses out‑of‑tree builds in `build/debug` and `build/release`
- If you see:

  ```
  Error: not a Meson build directory
  ```

  ensure you are running:

  ```bash
  meson compile -C build/debug <target>
  ```

- Git hooks must be installed once by running `./setup.sh`
- The Git hooks automatically run **clang-format** and **clang-tidy** on every commit.  
  This enforces consistent style and static analysis across the entire codebase.

---

## Prerequisites

- **Meson ≥ 1.3** and **Ninja**
- **C++17 compiler** (Clang / GCC / MSVC)
- Important tooling used in this project:
    - **clang-format** (automatic formatting; required via Git hooks)
    - **clang-tidy** (static analysis and linting; required via Git hooks)