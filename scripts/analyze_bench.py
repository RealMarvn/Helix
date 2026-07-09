#!/usr/bin/env python3
"""Summarize bench CSVs into per-budget tables.

Takes the CSVs produced by helix-bench and prints one table per input:

  --quality      scored time_to_quality CSV (needs the 'cpl' column from
                 scripts/score_with_stockfish.py): mean centipawn loss,
                 reached depth and move agreement per time budget.
  --depth-time   depth_vs_time CSV straight from the bench: NPS, depth,
                 nodes and TT hit rate per time budget, split by the
                 pvs/nmp flags if present.
  --tt-sweep     tt_sweep CSV: depth, hit rate and replacement pressure
                 per table size.
  --pvs-sweep    pvs_sweep CSV: depth, nodes, re-searches and hit rate
                 per PVS parameter combination.

Usage:
    pip install pandas matplotlib
    python3 scripts/analyze_bench.py \
        --quality results/time_to_quality_X_scored.csv \
        --depth-time results/depth_vs_time_Y.csv \
        [--plots results/plots]

Both inputs are optional; pass whichever you have. With --plots a couple of
PNGs are written, otherwise the script only prints the tables.
"""

import argparse

import pandas as pd

# Per-column formats for the console tables. Everything not listed here
# falls back to two decimals.
COLUMN_FORMATS = {
    "budget_ms": lambda v: f"{v:d}",
    "tt_mb": lambda v: f"{v:d}",
    "min_depth": lambda v: f"{v:d}",
    "scout_after": lambda v: f"{v:d}",
    "positions": lambda v: f"{v:d}",
    "replace_rate": lambda v: f"{v:.3f}",
    "cpl": lambda v: f"{v:.1f}",
    "depth": lambda v: f"{v:.1f}",
    "completed_depth": lambda v: f"{v:.1f}",
    "agreement": lambda v: f"{v:.2f}",
    "nps": lambda v: f"{v:,.0f}",
    "nodes": lambda v: f"{v:,.0f}",
    "researches": lambda v: f"{v:.1f}",
    "null_cutoffs": lambda v: f"{v:,.0f}",
    "tt_hit_rate": lambda v: f"{v:.3f}",
}


def load_csv(path):
    """Read a bench CSV, skipping the leading '#' config/header comment lines."""
    return pd.read_csv(path, comment="#")


def print_table(title, df):
    """Print one summary table with per-column formatting."""
    formatters = {col: COLUMN_FORMATS[col] for col in df.columns if col in COLUMN_FORMATS}
    print(f"== {title} ==")
    print(df.to_string(index=False, formatters=formatters, float_format=lambda v: f"{v:.2f}"))
    print()


def summarize_quality(df):
    """Mean centipawn loss, depth and agreement per time budget."""
    needed = {"budget_ms", "cpl", "completed_depth"}
    missing = needed - set(df.columns)
    if missing:
        raise ValueError(
            f"quality CSV is missing columns {sorted(missing)} "
            f"- did you run score_with_stockfish.py first?"
        )

    agg = {
        "cpl": ("cpl", "mean"),
        "depth": ("completed_depth", "mean"),
        "positions": ("cpl", "size"),
    }
    if "agreement" in df.columns:
        agg["agreement"] = ("agreement", "mean")

    group_cols = ["budget_ms"]
    if "nmp" in df.columns:
        group_cols.append("nmp")

    return df.groupby(group_cols).agg(**agg).reset_index().sort_values(group_cols)


def summarize_search(df):
    """NPS, depth and TT hit rate per time budget from a depth_vs_time CSV."""
    if "budget_ms" not in df.columns:
        raise ValueError("depth_vs_time CSV has no 'budget_ms' column.")

    df = df.copy()
    # NPS per run; guard against the zero-time rows of the tiniest budgets.
    df["nps"] = df.apply(
        lambda r: (r["nodes"] * 1000.0 / r["time_ms"]) if r["time_ms"] > 0 else float("nan"),
        axis=1,
    )

    agg = {
        "completed_depth": ("completed_depth", "mean"),
        "nps": ("nps", "mean"),
        "nodes": ("nodes", "mean"),
        "researches": ("researches", "mean"),
    }
    if "null_cutoffs" in df.columns:
        agg["null_cutoffs"] = ("null_cutoffs", "mean")
    if "tt_hit_rate" in df.columns:
        agg["tt_hit_rate"] = ("tt_hit_rate", "mean")

    group_cols = ["budget_ms"]
    if "pvs" in df.columns:
        group_cols.append("pvs")
    if "nmp" in df.columns:
        group_cols.append("nmp")

    return df.groupby(group_cols).agg(**agg).reset_index().sort_values(group_cols)


def summarize_tt_sweep(df):
    """Depth, hit rate and replacement pressure per TT size from a tt_sweep CSV."""
    if "tt_mb" not in df.columns:
        raise ValueError("tt_sweep CSV has no 'tt_mb' column.")

    df = df.copy()
    # Replacement pressure: what fraction of stores had to evict an entry.
    df["replace_rate"] = df.apply(
        lambda r: (r["tt_replaces"] / r["tt_stores"]) if r["tt_stores"] > 0 else 0.0,
        axis=1,
    )

    agg = {
        "completed_depth": ("completed_depth", "mean"),
        "nodes": ("nodes", "mean"),
        "tt_hit_rate": ("tt_hit_rate", "mean"),
        "replace_rate": ("replace_rate", "mean"),
    }

    return df.groupby("tt_mb").agg(**agg).reset_index().sort_values("tt_mb")


def summarize_pvs_sweep(df):
    """Depth, nodes, re-searches and hit rate per PVS parameter combination."""
    needed = {"min_depth", "scout_after"}
    missing = needed - set(df.columns)
    if missing:
        raise ValueError(f"pvs_sweep CSV is missing columns {sorted(missing)}.")

    agg = {
        "completed_depth": ("completed_depth", "mean"),
        "nodes": ("nodes", "mean"),
        "researches": ("researches", "mean"),
    }
    if "tt_hit_rate" in df.columns:
        agg["tt_hit_rate"] = ("tt_hit_rate", "mean")

    group_cols = ["min_depth", "scout_after"]
    return df.groupby(group_cols).agg(**agg).reset_index().sort_values(group_cols)


def make_plots(quality, search, out_dir):
    """Write the two diagnostic plots. Imported lazily so the text report works
    even without matplotlib installed."""
    import os

    import matplotlib

    matplotlib.use("Agg")  # no display needed, we only save files
    import matplotlib.pyplot as plt

    os.makedirs(out_dir, exist_ok=True)
    written = []

    if quality is not None:
        fig, ax1 = plt.subplots(figsize=(7, 4.5))
        ax2 = ax1.twinx()

        # One cpl/depth curve pair per nmp value, if the column exists.
        groups = quality.groupby("nmp") if "nmp" in quality.columns else [("", quality)]
        styles = ["o-", "o--"]
        for i, (nmp_val, grp) in enumerate(groups):
            suffix = f" (nmp={nmp_val})" if nmp_val != "" else ""
            ax1.plot(grp["budget_ms"], grp["cpl"], styles[i % 2], color="tab:red",
                     label=f"cpl{suffix}")
            ax2.plot(grp["budget_ms"], grp["depth"], "s" + styles[i % 2][1:], color="tab:blue",
                     label=f"depth{suffix}")

        ax1.set_xscale("log")
        ax1.set_xlabel("time budget [ms] (log)")
        ax1.set_ylabel("mean centipawn loss", color="tab:red")
        ax1.tick_params(axis="y", labelcolor="tab:red")
        ax2.set_ylabel("mean completed depth", color="tab:blue")
        ax2.tick_params(axis="y", labelcolor="tab:blue")

        if "nmp" in quality.columns:
            ax1.legend(loc="upper left")
            ax2.legend(loc="upper right")

        ax1.set_title("Quality vs. depth over time budget")
        fig.tight_layout()
        path = os.path.join(out_dir, "quality_vs_depth.png")
        fig.savefig(path, dpi=130)
        plt.close(fig)
        written.append(path)

    if search is not None and "nps" in search.columns:
        fig, ax = plt.subplots(figsize=(7, 4.5))
        # One line per pvs/nmp combination, if those columns exist.
        flag_cols = [c for c in ("pvs", "nmp") if c in search.columns]
        if flag_cols:
            for flags, grp in search.groupby(flag_cols):
                if not isinstance(flags, tuple):
                    flags = (flags,)
                label = " ".join(f"{c}={v}" for c, v in zip(flag_cols, flags))
                ax.plot(grp["budget_ms"], grp["nps"], "o-", label=label)
            ax.legend()
        else:
            ax.plot(search["budget_ms"], search["nps"], "o-")
        ax.set_xscale("log")
        ax.set_xlabel("time budget [ms] (log)")
        ax.set_ylabel("nodes per second")
        ax.set_title("Search throughput (NPS) over time budget")
        fig.tight_layout()
        path = os.path.join(out_dir, "search_nps.png")
        fig.savefig(path, dpi=130)
        plt.close(fig)
        written.append(path)

    return written


def main():
    parser = argparse.ArgumentParser(
        description="Summarize bench CSVs into per-budget tables.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--quality", help="scored time_to_quality CSV (needs a 'cpl' column)")
    parser.add_argument("--depth-time", help="depth_vs_time CSV from the bench")
    parser.add_argument("--tt-sweep", help="tt_sweep CSV from the bench")
    parser.add_argument("--pvs-sweep", help="pvs_sweep CSV from the bench")
    parser.add_argument("--plots", help="directory for PNG plots (optional)")
    args = parser.parse_args()

    if not args.quality and not args.depth_time and not args.tt_sweep and not args.pvs_sweep:
        parser.error("pass at least one of --quality, --depth-time, --tt-sweep or --pvs-sweep")

    quality_tbl = None
    search_tbl = None

    if args.quality:
        quality_tbl = summarize_quality(load_csv(args.quality))
        print_table("Quality per budget", quality_tbl)

    if args.depth_time:
        search_tbl = summarize_search(load_csv(args.depth_time))
        print_table("Search per budget", search_tbl)

    if args.tt_sweep:
        tt_tbl = summarize_tt_sweep(load_csv(args.tt_sweep))
        print_table("Search per TT size", tt_tbl)

    if args.pvs_sweep:
        pvs_tbl = summarize_pvs_sweep(load_csv(args.pvs_sweep))
        print_table("Search per PVS config", pvs_tbl)

    if args.plots:
        written = make_plots(quality_tbl, search_tbl, args.plots)
        for p in written:
            print(f"Wrote {p}")


if __name__ == "__main__":
    main()
