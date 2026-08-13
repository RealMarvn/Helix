#!/usr/bin/env python3
"""Paired bootstrap confidence intervals for the numbers reported in the thesis.

The dominant source of variation in these experiments is the difference between
chess positions, not measurement noise, so uncertainty is quantified on the
paired differences rather than on the raw means. The resampling unit is the one
that is actually independent: positions for the suite experiments, whole games
for tree reuse, whose plies are clustered within games.

Seeds are fixed per cell so the printed intervals are reproducible.

Usage:
    python3 scripts/bootstrap_stats.py [--results results]
"""

import argparse
import csv
import glob
import io
import os
import random
import statistics as st
from collections import defaultdict

RESAMPLES = 20000  # lower this for a quick check


def load(path):
    """Read a bench CSV, skipping the leading comment lines."""
    with open(path) as f:
        rows = [line for line in f if not line.startswith("#")]
    return list(csv.DictReader(io.StringIO("".join(rows))))


def boot_ci(values, seed, resamples=RESAMPLES):
    """Percentile bootstrap CI of the mean, deterministic for a given seed."""
    rnd = random.Random(seed)
    n = len(values)
    means = sorted(
        st.mean([values[rnd.randrange(n)] for _ in range(n)]) for _ in range(resamples)
    )
    return st.mean(values), means[int(0.025 * resamples)], means[int(0.975 * resamples)]


def per_position(rows, key):
    """{budget: {position: mean over repetitions}}"""
    out = defaultdict(lambda: defaultdict(list))
    for row in rows:
        if row.get(key, "") not in ("", "nan"):
            out[int(row["budget_ms"])][int(row["fen_id"])].append(float(row[key]))
    return {b: {f: st.mean(v) for f, v in p.items()} for b, p in out.items()}


def nmp(results):
    scored = [load(f) for f in glob.glob(f"{results}/time_to_quality_*_scored.csv")]
    on = next(r for r in scored if r[0]["nmp"] == "on")
    off = next(r for r in scored if r[0]["nmp"] == "off")

    depth_on, depth_off = per_position(on, "completed_depth"), per_position(off, "completed_depth")
    cpl_on, cpl_off = per_position(on, "cpl"), per_position(off, "cpl")

    print("\n=== Null-move pruning, paired over positions (NMP minus no NMP) ===")
    print(f"{'budget':>7}  {'depth':>26}  {'centipawn loss':>26}")
    for budget in sorted(cpl_on):
        shared = set(cpl_on[budget]) & set(cpl_off[budget])
        d = boot_ci([depth_on[budget][f] - depth_off[budget][f] for f in shared], budget)
        c = boot_ci([cpl_on[budget][f] - cpl_off[budget][f] for f in shared], budget + 1)
        print(
            f"{budget:7d}  {d[0]:+6.2f} [{d[1]:+6.2f}, {d[2]:+6.2f}]  "
            f"{c[0]:+6.1f} [{c[1]:+6.1f}, {c[2]:+6.1f}]"
        )
    print("  depth: every interval excludes zero; cpl: none does")


def saturation(results):
    """Smallest budget from which saturation holds and keeps holding.

    Taking the first budget without a detectable improvement would not do: the
    criterion is not monotone, because very short budgets scatter so widely that
    nothing is detectable there either. t* is therefore the smallest budget such
    that it and every larger one show no further improvement.
    """
    scored = [load(f) for f in glob.glob(f"{results}/time_to_quality_*_scored.csv")]
    on = next(r for r in scored if r[0]["nmp"] == "on")
    cpl = per_position(on, "cpl")
    budgets = sorted(cpl)

    print("\n=== Saturation criterion t* ===")
    saturated = {}
    for i, b in enumerate(budgets):
        still = []
        for later in budgets[i + 1:]:
            shared = set(cpl[b]) & set(cpl[later])
            _, lo, hi = boot_ci([cpl[later][f] - cpl[b][f] for f in shared], b * 7 + later)
            if hi < 0:
                still.append(later)
        saturated[b] = not still
        print(f"  from {b:5d} ms: further improvement detectable at {still or 'no larger budget'}")

    t_star = next(
        (b for i, b in enumerate(budgets) if all(saturated[x] for x in budgets[i:])),
        None,
    )
    print(f"  -> t* = {t_star} ms (smallest budget from which saturation also holds for all larger ones)")


def tree_reuse(results):
    modes = {}
    for path in glob.glob(f"{results}/tree_reuse_*.csv"):
        rows = load(path)
        modes[rows[0]["mode"]] = rows
    if len(modes) < 2:
        print("\n=== Tree reuse: need both keep and clear runs ===")
        return

    print("\n=== Tree reuse, resampled over games (keep minus clear) ===")
    for key, label in [("completed_depth", "depth"), ("tt_hit_rate", "hit rate")]:
        keep = {(r["game_id"], r["ply"]): float(r[key]) for r in modes["keep"]}
        clear = {(r["game_id"], r["ply"]): float(r[key]) for r in modes["clear"]}
        shared = set(keep) & set(clear)

        per_game = defaultdict(list)
        for game, ply in shared:
            per_game[game].append(keep[(game, ply)] - clear[(game, ply)])
        game_means = [st.mean(v) for v in per_game.values()]

        m, lo, hi = boot_ci(game_means, 7)
        positive = sum(1 for v in game_means if v > 0)
        print(
            f"  {label:9s} {m:+.3f} [{lo:+.3f}, {hi:+.3f}]  "
            f"(n={len(game_means)} games, {positive} of them positive)"
        )


def main():
    parser = argparse.ArgumentParser(description="Bootstrap CIs for the thesis numbers.")
    parser.add_argument("--results", default="results", help="directory holding the bench CSVs")
    args = parser.parse_args()

    if not os.path.isdir(args.results):
        raise SystemExit(f"No such directory: {args.results}")

    nmp(args.results)
    saturation(args.results)
    tree_reuse(args.results)


if __name__ == "__main__":
    main()
