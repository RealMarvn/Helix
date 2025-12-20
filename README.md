# ♟️ Helix — High‑Performance Chess Engine

**Helix** is a modern C++ chess engine that originally started as a **PK1 university programming project** and was later
extended and reworked into a **research‑driven bachelor thesis** at the University of Konstanz.

The engine is designed with a strong focus on **correctness, transparency, and architectural clarity**, making it an
ideal platform for experimenting with **search algorithms**, **evaluation functions**, and **engine instrumentation**.
Rather than relying on aggressive low‑level micro‑optimizations or bitboard‑specific tricks, Helix prioritizes clean
abstractions and well‑structured components that can be reasoned about, analyzed, and extended.

Helix deliberately uses a refined **Mailbox board representation**, trading raw speed for excellent debuggability and
readability while still supporting all essential chess mechanics.

On top of this foundation, Helix implements a robust **NegaMax + Alpha‑Beta** search framework with iterative deepening,
transposition tables, advanced move ordering heuristics, and full **time‑controlled UCI integration**.

Helix can be run either as a standalone CLI program or as a **UCI‑compatible engine** inside chess GUIs such as
CuteChess or Banksia.

---

## 📌 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Technical Design](#technical-design)
- [Project Structure](#project-structure)
- [Build & Setup](#build--setup)
- [Usage](#usage)
    - [CLI Mode](#cli-mode)
    - [UCI Mode](#uci-mode)
    - [Debugging & Instrumentation](#debugging--instrumentation)
- [Notes](#notes)
- [Prerequisites](#prerequisites)

---

## Overview

Helix explores the computational complexity of chess decision‑making with an emphasis on **search behavior analysis**,
**engine observability**, and **clean algorithmic design**.

The engine integrates a modern alpha‑beta search enhanced with:

- **Iterative deepening**
- **Transposition tables**
- **Heuristic‑driven move ordering**
- **Explicit time management and pondering**

Unlike highly optimized tournament engines, Helix is intentionally designed to remain **inspectable and explainable**,
making it suitable for empirical evaluation, debugging, and academic analysis.

---

## Features

### ✔️ Move Generation

- **Pseudolegal move generation** (correctness‑oriented)
- **Mailbox (0×88‑style) board representation**
- Full support for:
    - **Castling**
    - **En passant**
    - **Pawn promotion**
- Clear and maintainable data structures optimized for reasoning and experimentation

---

### ✔️ Search Algorithm

- **NegaMax** formulation (clean minimax variant for zero‑sum games)
- **Alpha‑Beta pruning** to reduce the explored game tree
- **Iterative deepening** for stable principal‑variation construction
- **Quiescence search** to mitigate horizon effects
- **Transposition Table (TT)** with depth‑sensitive bounds:
    - Exact
    - Lower bound
    - Upper bound
- **Advanced move ordering heuristics**:
    - TT / principal‑variation move first
    - MVV‑LVA ordering for captures
    - Killer move heuristic (per‑ply)
    - History heuristic (global quiet‑move statistics)

---

### ✔️ Engine Architecture

- **Dedicated worker thread for search**
    - Main thread remains responsive to UCI commands
    - Search can be safely interrupted via `stop`
- **Pondering support**
    - Engine can continue searching during the opponent’s thinking time
    - Ponder hit detection cleanly integrates into the main search loop
- Explicit separation between:
    - Engine control (UCI / CLI)
    - Search logic
    - Evaluation
    - Debug & instrumentation
- Architecture designed to support future extensions such as multi‑threaded search

---

### ✔️ Time Management

- Supports both:
    - **Fixed‑depth searches**
    - **Time‑controlled searches** via UCI (`movetime`, `wtime`, `btime`, increments)
- Explicit soft and hard time limits
- Clean search abortion and stop‑reason handling

---

### ✔️ Debugging & Instrumentation

Helix includes a **multi‑level debug and instrumentation system** designed to make the internal behavior of the search
engine observable without polluting performance‑critical code paths.

Debug output follows the UCI `info string` convention and can be enabled dynamically.

#### Debug Levels

Debugging can be configured via the UCI option:

```
setoption name Debug value <level>
```

Available levels:

| Level     | Description |
|-----------|-------------|
| `none`    | No debug output (default) |
| `basic`   | High‑level search information (nodes, time, score, stop reason) |
| `medium`  | Search health metrics (QSearch ratio, TT statistics, TT returns) |
| `verbose` | Root move ordering analysis and principal‑variation reconstruction |

Example:

```
setoption name Debug value verbose
```

Debug output includes:

- Search statistics (nodes, NPS, depth, selective depth)
- Quiescence search ratio
- Transposition table effectiveness
- Root move ordering snapshots
- Principal variation reconstruction via TT tracing

All debug logic is isolated from the core search and executed only at iteration boundaries.

---

## Technical Design

### 1. Board Representation

- Mailbox representation for clarity and correctness
- Full tracking of castling rights, en passant, and move history
- FEN parsing and validation utilities

### 2. Search System

- Recursive NegaMax with alpha‑beta pruning
- Iterative deepening driver with time control and pondering
- Transposition table with generation‑based aging
- Modular move ordering subsystem

### 3. Evaluation Function

- Lightweight material evaluation
- PESTO‑style piece‑square tables (midgame / endgame interpolation)
- Clean separation from search logic for extensibility

### 4. Debug & Observability

- Search instrumentation isolated in a dedicated debug module
- No debug logic inside hot search paths
- Designed for post‑hoc analysis and experimentation

---

## Project Structure

```
/engine/
├── core/        → Board, moves
├── eval/        → Evaluation
├── exceptions/  → Exceptions
├── movement/    → Move generation
├── search/      → NegaMax, TT, heuristics
├── search/time  → Time management
/tests/          → Unit tests for move generation and state validation
```

---

## Build & Setup

After cloning the repository:

```bash
./setup.sh
```

This script:

- Configures `build/debug` and `build/release`
- Builds the engine and tests
- Installs clang‑format and clang‑tidy Git hooks

---

## Usage

### CLI Mode

Uses UCI‑style coordinate notation:

```
e2e4
b1c3
e7e8q
```

---

### UCI Mode

Minimal example:

```
uci
isready
ucinewgame
position startpos
setoption name Debug value medium
go movetime 2000
bestmove e2e4
```

---

## Notes

- Uses out‑of‑tree Meson builds
- Git hooks enforce consistent formatting and static analysis
- Debug output is fully UCI‑compliant and GUI‑safe

---

## Prerequisites

- **Meson ≥ 1.3**
- **Ninja**
- **C++17 compiler**
- Required tools:
    - clang‑format
    - clang‑tidy