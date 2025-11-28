# ♟️ Helix | ChessBot

**Helix** is a chess bot developed as part of my first programming project (PK1) and my bachelor project at the
University of Konstanz. This project dives into the complexities of chess move generation, including pseudolegal moves,
castling, en passant, and iterative deepening for move depth evaluation.

## 🚀 Features

- **Pseudolegal Move Generation**: Generates moves without considering check, enhancing speed.
- **Castling and En Passant**: Supports special moves, providing a more complete set of chess rules.
- **Alpha-Beta Pruning**: Optimization of depth evaluation for faster computation.
- **Iterative Deepening**: Uses progressive depth evaluation to find the best move efficiently.

## 🏫 Project Background

This project was created as an introduction to programming concepts and problem-solving strategies. It combines classic
chess rules with computational methods to give a practical application to theoretical programming knowledge from the
University of Konstanz’s PK1 course. Later this project was used for my bachelor project with more advanced improvements
like iterative deepening and other optimizations.

## 🔍 How It Works

ChessBot uses the following strategies:

1. **Pseudolegal Moves**: Generates potential moves quickly by not checking for attacks on the king at first.
2. **Special Move Support**: Includes special moves like castling and en passant for a realistic move set including most
   of the classic chess rules.
3. **Iterative Deepening**: Allows the bot to make decisions under time constraints, prioritizing moves as it explores
   deeper levels.

## 🏗️ Project Structure

- Under ``/engine/`` you can find the whole bot with board logic and much more.
- Under ``/tests/`` you can find all tests I used to prove the MoveMaking is correct.

## ⚙️ Build & Setup

After cloning the repository, run the setup script **once**:

```bash
./setup.sh
```

This script will:

- Configure a Debug build in `build/debug`
- Build the engine and tests
- Configure a Release build in `build/release`
- Install a `pre-commit` Git hook (if this is a Git repository)

---

## ▶️ Common Commands

All commands should be executed from the **project root directory**.

### Build only the engine

```bash
# To build the optimized release version:
cmake --build build/release --target chess-engine

# Build only the engine
cmake --build build/debug --target chess-engine

# Build and run the engine
cmake --build build/debug --target run

# Run all tests
cmake --build build/debug --target test_all

# Format all source files
cmake --build build/debug --target format

# Check formatting only (no changes)
cmake --build build/debug --target format-check

# Run clang-tidy
cmake --build build/debug --target tidy
```

---

## 🛠️ Notes

- All commands use the CMake build directory (`build/debug` or `build/release`).
- If you see:

  ```
  Error: not a CMake build directory
  ```

  make sure you are calling:

  ```bash
  cmake --build build/debug --target <command>
  ```

- Git hooks are local to your machine and must be installed once via `./setup.sh`.

## 🕹️ Usage

### 🧑‍💻 CLI Mode (local manual testing)

The CLI mode now uses **UCI-style move notation** for manual input.
The engine automatically switches to CLI mode when the GUI sends the `classic` command.
This means moves are entered the same way as in UCI:

```
e2e4      # Pawn from e2 to e4
b1c3      # Knight from b1 to c3
e7e8q     # Pawn promotes to a queen (promotion always lowercase)
```

- No piece letters (`P, N, B, R, Q, K`) are required.
- Promotion uses **lowercase** letters, identical to UCI.
- The engine determines the moving piece from the board state.

### ♟️ UCI Mode (for chess GUIs)

The engine automatically switches to UCI mode when the GUI sends the `uci` command.

Minimal example interaction:

```
uci
isready
ucinewgame
position startpos
go movetime 2000
bestmove e2e4
```

In UCI mode, moves follow standard UCI notation:

```
e2e4
b1c3
e7e8q    # promotion (always lowercase)
```

UCI moves **do not contain piece letters**; the engine determines the moving piece from the board state.

## Prerequisites

Make sure the following tools are installed:

- **CMake ≥ 3.27**
- **C++17-compatible compiler** (clang, GCC, or MSVC)
- (Recommended)
   - `clang-format` for code formatting
   - `clang-tidy` for static analysis

GoogleTest is downloaded automatically by CMake via `FetchContent`. No manual installation is required.