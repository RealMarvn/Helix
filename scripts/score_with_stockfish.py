#!/usr/bin/env python3
"""Score bench CSVs with Stockfish.

Takes a CSV produced by helix-bench (needs the columns 'fen' and 'bestmove')
and appends quality columns:

    sf_best        - Stockfish's best move for the position
    sf_eval_best   - eval of Stockfish's move (cp, from the side to move)
    sf_eval_played - eval of the engine's move (cp, from the side to move)
    cpl            - centipawn loss: max(0, sf_eval_best - sf_eval_played)
    agreement      - 1 if the engine picked Stockfish's move, else 0

Scoring is separate from measuring on purpose: you can re-score old runs
without re-running any search.

Usage:
    pip install python-chess
    python3 scripts/score_with_stockfish.py results/time_to_quality_X.csv \
        --stockfish /path/to/stockfish [--depth 20] [--out results/scored.csv]
"""

import argparse
import csv
import sys

import chess
import chess.engine

# Mate scores get capped so a single mate line does not dominate the averages.
MATE_CAP_CP = 1000


def eval_after_move(engine, board, move, depth):
    """Eval of a position after playing one move, from the mover's perspective."""
    board.push(move)
    info = engine.analyse(board, chess.engine.Limit(depth=depth))
    score = info["score"].pov(not board.turn)  # perspective of the side that just moved
    board.pop()
    return score.score(mate_score=MATE_CAP_CP * 10)


def main():
    parser = argparse.ArgumentParser(description="Score bench CSVs with Stockfish.")
    parser.add_argument("csv_in", help="CSV produced by helix-bench (needs fen + bestmove)")
    parser.add_argument("--stockfish", required=True, help="path to the Stockfish binary")
    parser.add_argument("--depth", type=int, default=20, help="Stockfish search depth")
    parser.add_argument("--out", help="output CSV (default: <input>_scored.csv)")
    args = parser.parse_args()

    out_path = args.out or args.csv_in.replace(".csv", "_scored.csv")

    # Keep the '#' config header lines so the scored file stays self-documenting.
    header_lines = []
    with open(args.csv_in) as f:
        for line in f:
            if line.startswith("#"):
                header_lines.append(line.rstrip("\n"))
            else:
                break

    with open(args.csv_in) as f:
        rows = list(csv.DictReader(line for line in f if not line.startswith("#")))

    if not rows or "fen" not in rows[0] or "bestmove" not in rows[0]:
        sys.exit("Input CSV needs 'fen' and 'bestmove' columns.")

    engine = chess.engine.SimpleEngine.popen_uci(args.stockfish)
    engine.configure({"Threads": 1})

    # Cache per FEN (best move) and per FEN+move (played eval), the same
    # position shows up once per budget and rep.
    best_cache = {}
    played_cache = {}

    try:
        for i, row in enumerate(rows):
            fen = row["fen"]
            board = chess.Board(fen)

            if fen not in best_cache:
                info = engine.analyse(board, chess.engine.Limit(depth=args.depth))
                sf_best = info["pv"][0]
                sf_eval_best = info["score"].pov(board.turn).score(mate_score=MATE_CAP_CP * 10)
                best_cache[fen] = (sf_best, sf_eval_best)

            sf_best, sf_eval_best = best_cache[fen]
            played = chess.Move.from_uci(row["bestmove"])

            key = (fen, row["bestmove"])
            if key not in played_cache:
                if played == sf_best:
                    played_cache[key] = sf_eval_best
                else:
                    played_cache[key] = eval_after_move(engine, board, played, args.depth)

            sf_eval_played = played_cache[key]
            cpl = max(0, sf_eval_best - sf_eval_played)
            cpl = min(cpl, MATE_CAP_CP)

            row["sf_best"] = sf_best.uci()
            row["sf_eval_best"] = sf_eval_best
            row["sf_eval_played"] = sf_eval_played
            row["cpl"] = cpl
            row["agreement"] = int(played == sf_best)

            if (i + 1) % 50 == 0:
                print(f"{i + 1}/{len(rows)} rows scored")
    finally:
        engine.quit()

    with open(out_path, "w", newline="") as f:
        for line in header_lines:
            f.write(line + "\n")
        f.write(f"# scored with stockfish depth={args.depth}\n")
        writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
