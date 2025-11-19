# ♟️ ChessBot

**ChessBot** is a chess bot developed as part of my first programming project (PK1) and my bachelor project at the
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

## 📝 Libaries

- Gtest from Google for automated tests