#!/usr/bin/env python3
"""Score bench CSVs with Stockfish.

Takes a CSV produced by helix-bench (needs the columns 'fen' and 'bestmove')
and appends quality columns:

    sf_best        - Stockfish's best move for the position
    sf_eval_best   - eval of Stockfish's move (cp, from the side to move)
    sf_eval_played - eval of the engine's move (cp, from the side to move)
    cpl            - centipawn loss: max(0, sf_eval_best - sf_eval_played)
    agreement      - 1 if the engine picked Stockfish's move, else 0

Scoring is separate from measuring, so old runs can be re-scored without
re-running any search.

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

# Cap for mate scores, otherwise a single mate line dominates the averages.
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

    # Keep the '#' config header lines from the input file.
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

    # Cache per FEN (best move) and per FEN+move (played eval). The same
    # position shows up once per budget and rep.
    best_cache = {}
    played_cache = {}

    def blank_score(row):
        """Leave the row unscored but keep the columns."""
        row["sf_best"] = ""
        row["sf_eval_best"] = ""
        row["sf_eval_played"] = ""
        row["cpl"] = ""
        row["agreement"] = ""

    skipped = 0

    try:
        for i, row in enumerate(rows):
            fen = row["fen"]
            board = chess.Board(fen)

            # Mate/stalemate positions have no move to score.
            if board.is_game_over():
                blank_score(row)
                skipped += 1
                continue

            if fen not in best_cache:
                info = engine.analyse(board, chess.engine.Limit(depth=args.depth))
                pv = info.get("pv")
                if not pv:
                    best_cache[fen] = None
                else:
                    sf_eval_best = info["score"].pov(board.turn).score(mate_score=MATE_CAP_CP * 10)
                    best_cache[fen] = (pv[0], sf_eval_best)

            if best_cache[fen] is None:
                # Stockfish returned no PV for this position.
                blank_score(row)
                skipped += 1
                continue

            sf_best, sf_eval_best = best_cache[fen]

            # bestmove can be null ("0000") or illegal, skip those rows.
            try:
                played = chess.Move.from_uci(row["bestmove"])
            except ValueError:
                played = None

            if played is None or played not in board.legal_moves:
                row["sf_best"] = sf_best.uci()
                row["sf_eval_best"] = sf_eval_best
                row["sf_eval_played"] = ""
                row["cpl"] = ""
                row["agreement"] = 0
                skipped += 1
                continue

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

    if skipped:
        print(f"{skipped} rows skipped (terminal position or invalid bestmove)")

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
