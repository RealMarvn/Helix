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
    - [Compiling](#compiling)
    - [Running the Tests](#running-the-tests)
    - [Running a Benchmark](#running-a-benchmark)
- [Usage](#usage)
    - [CLI Mode](#cli-mode)
    - [UCI Mode](#uci-mode)
    - [Debugging & Instrumentation](#debugging--instrumentation)
    - [Search Tuning (PVS)](#search-tuning-pvs)
    - [Search Tuning (Null Move Pruning)](#search-tuning-null-move-pruning)
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
- **Principal Variation Search (PVS)** with tunable null‑window scouting
- **Null Move Pruning (NMP)** with zugzwang guard and tunable reduction
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

### Search Tuning (PVS)

Helix uses **Principal Variation Search (PVS)** as a refinement of plain alpha‑beta: the first
(best‑ordered) move is searched with a full window, while every later move is first probed with a
cheap null‑window scout and only re‑searched with the full window if it unexpectedly beats `alpha`.

Two UCI options expose the PVS behavior so the scouting trade‑off can be measured without
recompiling:

```
setoption name PvsMinDepth value <int>
setoption name PvsScoutAfterMove value <int>
```

| Option              | Default | Description |
|---------------------|---------|-------------|
| `PvsMinDepth`       | `2`     | Minimum remaining depth at which null‑window scouting is used. Below this depth every move is searched with a full window, since the re‑search overhead would outweigh the savings at shallow nodes. |
| `PvsScoutAfterMove` | `1`     | Number of leading (best‑ordered) moves searched with a full window before scouting begins. The remaining moves are probed with a null window first. |

With the defaults (`PvsMinDepth = 2`, `PvsScoutAfterMove = 1`) the engine searches the first move
per node with a full window and scouts the rest — the standard PVS configuration. Raising
`PvsMinDepth` to a large value effectively disables scouting, which is convenient for an
on/off ablation against plain alpha‑beta.

Example:

```
setoption name PvsMinDepth value 3
setoption name PvsScoutAfterMove value 2
```

---

### Search Tuning (Null Move Pruning)

Helix implements **Null Move Pruning (NMP)**: before searching the real moves at a node, the engine
hands the opponent a free move and runs a reduced null‑window search around `beta`. If the position
is still strong enough that even this "do nothing" search fails high, the real moves can only be
better and the whole node is cut immediately.

The pruning is guarded to stay sound:

- skipped when the side to move is **in check** (passing would be illegal),
- skipped when only **pawns and king** remain (zugzwang positions, where passing is often best),
- skipped near **mate scores** (a fake move must not distort mate distances),
- never applied **twice in a row** and never at the root.

Three UCI options expose the NMP behavior so the trade‑off can be measured without recompiling:

```
setoption name NullMove value <true|false>
setoption name NullMoveMinDepth value <int>
setoption name NullMoveReduction value <int>
```

| Option              | Default | Description |
|---------------------|---------|-------------|
| `NullMove`          | `true`  | Enables or disables null move pruning entirely. Convenient for an on/off ablation. |
| `NullMoveMinDepth`  | `3`     | Minimum remaining depth at which a null move is tried. Below this depth the reduced search would land straight in quiescence and prove nothing. |
| `NullMoveReduction` | `2`     | Depth reduction `R` applied to the null move search (`depth − 1 − R`). Larger values prune more aggressively but risk overlooking deep threats. |

Successful null move cutoffs are reported as `nullcuts` in the debug output and as the
`null_cutoffs` column in the bench CSVs (`depth-vs-time` and `time-to-quality` both accept
`--nmp on|off` for A/B measurements).

Example:

```
setoption name NullMove value true
setoption name NullMoveReduction value 3
```

---

## Technical Design

### 1. Board Representation

- Mailbox representation for clarity and correctness
- Full tracking of castling rights, en passant, and move history
- FEN parsing and validation utilities

### 2. Search System

- Recursive NegaMax with alpha‑beta pruning
- Null move pruning with zugzwang and mate‑score guards
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

### Compiling

After the initial setup, rebuilding is a single Meson call into the desired build directory:

```bash
meson compile -C build/release        # optimized build (engine, tests, bench)
meson compile -C build/debug          # debug build (engine only)
```

The engine binary is `build/release/helix` (or `build/debug/helix`). Note that the **tests and the
bench harness are only built in the release configuration** — the debug directory contains just the
engine.

### Running the Tests

The unit tests (move generation via Perft, FEN round‑trips, move parsing, null move make/unmake)
run through Meson:

```bash
meson test -C build/release
```

For a single test or more detailed output, call the GTest binary directly:

```bash
./build/release/chess-tests --gtest_filter=Board.NullMoveMakeUnmake
```

### Running a Benchmark

The bench harness `helix-bench` links the engine as a library and writes one timestamped CSV per
run into `results/`. Run it without arguments to get the full usage overview:

```bash
./build/release/helix-bench
```

A typical A/B measurement (here: null move pruning on vs. off) looks like this:

```bash
./build/release/helix-bench depth-vs-time --suite tests/data/stockfish-defaults.epd --nmp off
./build/release/helix-bench depth-vs-time --suite tests/data/stockfish-defaults.epd --nmp on
python3 scripts/analyze_bench.py --depth-time results/depth_vs_time_<timestamp>.csv
```

For the quality side, `time-to-quality` records the chosen moves and
`scripts/score_with_stockfish.py` adds the centipawn loss against Stockfish afterwards:

```bash
./build/release/helix-bench time-to-quality --suite tests/data/stockfish-defaults.epd
python3 scripts/score_with_stockfish.py results/time_to_quality_<timestamp>.csv \
    --stockfish /path/to/stockfish
python3 scripts/analyze_bench.py --quality results/time_to_quality_<timestamp>_scored.csv
```

The transposition table size can be swept the same way (`tt-sweep`), recording reached depth,
hit rate and replacement pressure per table size:

```bash
./build/release/helix-bench tt-sweep --suite tests/data/stockfish-defaults.epd --mb 1,4,16,64,256
```

At runtime the table size is controlled via the standard UCI option `Hash` (in MB, default 32):

```
setoption name Hash value 128
```

The Python scripts need `pandas` (plots additionally `matplotlib`, scoring `python-chess`).

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