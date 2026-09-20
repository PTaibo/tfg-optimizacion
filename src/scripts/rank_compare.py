#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

runNum = 100

def parse_file(file_name):
    """Parse a combined wrdRank/Rank benchmark file.

    Returns two dicts, both keyed by wordsPerBlk -> list of cycle counts:
        wrdrank_data, rank_data
    """
    wrdrank_data = {}
    rank_data = {}
    current_words = None

    with open(file_name) as f:
        lines = f.readlines()
        for line in lines:
            if "word per rank block" in line:
                val = re.search(r'\d+', line)
                if val:
                    current_words = int(val.group())
                continue

            if current_words is None:
                continue

            # wrdRank / Rank lines each carry their own "Unhalted clock cycles".
            # NOTE: \bRank cannot match inside "wrdRank" (no word boundary
            # between "wrd" and "Rank"), but we still test wrdRank first for safety.
            m0 = re.search(r'wrdRank.*Unhalted clock cycles:\s*(\d+)', line)
            if m0:
                wrdrank_data.setdefault(current_words, []).append(int(m0.group(1)))
                continue

            m1 = re.search(r'\brank.*Unhalted clock cycles:\s*(\d+)', line)
            if m1:
                rank_data.setdefault(current_words, []).append(int(m1.group(1)))
                continue

    return wrdrank_data, rank_data


def filter_outliers(values, threshold=3.0):
    """Remove values that are more than threshold*sigma from mean"""
    if len(values) < 2:
        return values

    mean = np.mean(values)
    std = np.std(values)

    if std == 0:
        return values

    filtered = [v for v in values if abs(v - mean) <= threshold * std]

    removed_count = len(values) - len(filtered)
    if removed_count > 0:
        print(f"Removed {removed_count}/{len(values)} outliers (threshold: {threshold}\u03c3)")

    return filtered


def calculate_cleaned_means(data):
    """Calculate means after outlier removal for each configuration"""
    results = []

    for words, values in data.items():
        filtered_values = filter_outliers(values)
        if filtered_values:
            mean_val = np.mean(filtered_values)
            results.append([words, mean_val])

    return results


def make_overlap_plot(wrdrank_data, rank_data, plot, max_plots=None,
                      label0='wrdRank', label1='Rank',
                      color0='royalblue', color1='gold'):
    """Bar plot with wrdRank and Rank overlaid at the same x position.

    Both series share the same x-axis (words per block). Bars are drawn
    on top of one another with alpha transparency so the shorter series
    is visible inside the taller one, mirroring the reference comparison
    plot (blue = wrdRank total height, yellow/olive = Rank poking
    through underneath).
    """

    # All words-per-block values present in either series, sorted
    all_words = sorted(set(wrdrank_data.keys()) | set(rank_data.keys()))
    if max_plots is not None:
        all_words = all_words[:max_plots]

    means0 = {}
    means1 = {}

    print(f"\nResults (outliers removed) - {len(all_words)} configurations:")
    for words in all_words:
        if words in wrdrank_data:
            filtered0 = filter_outliers(wrdrank_data[words])
            if filtered0:
                means0[words] = np.mean(filtered0)
        if words in rank_data:
            filtered1 = filter_outliers(rank_data[words])
            if filtered1:
                means1[words] = np.mean(filtered1)

        v0 = f"{means0.get(words, float('nan')):.2f}"
        v1 = f"{means1.get(words, float('nan')):.2f}"
        print(f"{words}w -> {label0}: {v0}  {label1}: {v1}")

    if not means0 and not means1:
        plot.text(0.5, 0.5, 'No data available', transform=plot.transAxes,
                  ha='center', va='center')
        return

    labels = [f'{w}w' for w in all_words]
    x_coord = np.arange(len(all_words))

    vals0 = [means0.get(w, 0) for w in all_words]
    vals1 = [means1.get(w, 0) for w in all_words]

    # Draw the taller series first (so both are visible), overlapping at
    # the same x with transparency so the overlap region blends.
    plot.bar(x_coord, vals0, color=color0, alpha=0.65, label=label0, zorder=2)
    plot.bar(x_coord, vals1, color=color1, alpha=0.65, label=label1, zorder=3)

    plot.set_xticks(list(x_coord))
    plot.set_xticklabels(labels, rotation=45, ha='right')

    plot.set_xlabel('Words per block')
    plot.set_ylabel('Unhalted clock cycles (filtered)')
    plot.set_title('wrdRank vs Rank Benchmark (overlapped)')
    plot.legend()


def create_distribution_plot(file_name, target_words=None):
    """Create probability distributions for wrdRank and Rank side by side
    for a given words-per-block setting."""
    wrdrank_data, rank_data = parse_file(file_name)

    if target_words is None:
        all_keys = set(wrdrank_data.keys()) | set(rank_data.keys())
        target_words = min(all_keys) if all_keys else None

    if target_words is None or (target_words not in wrdrank_data and target_words not in rank_data):
        print(f"No data found for words-per-block {target_words}")
        print(f"Available values: {sorted(set(wrdrank_data.keys()) | set(rank_data.keys()))}")
        return

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    for ax, data, label, color in (
        (axes[0], wrdrank_data, 'wrdRank', 'royalblue'),
        (axes[1], rank_data, 'Rank', 'gold'),
    ):
        values = data.get(target_words, [])
        if not values:
            ax.text(0.5, 0.5, 'No data', transform=ax.transAxes, ha='center', va='center')
            continue

        ax.hist(values, bins=30, alpha=0.7, color=color, edgecolor='black', density=True)
        ax.set_xlabel('Unhalted clock cycles')
        ax.set_ylabel('Probability Density')
        ax.set_title(f'{label} Distribution: {target_words}w')

        mean_val = np.mean(values)
        median_val = np.median(values)

        ax.axvline(mean_val, color='red', linestyle='--', linewidth=2, label=f'Mean: {mean_val:.0f}')
        ax.axvline(median_val, color='green', linestyle='--', linewidth=2, label=f'Median: {median_val:.0f}')
        ax.legend()

        try:
            from scipy import stats
            kde = stats.gaussian_kde(values)
            x_range = np.linspace(min(values), max(values), 1000)
            ax.plot(x_range, kde(x_range), 'r-', linewidth=2)
        except ImportError:
            pass

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Usage: python rank_compare.py <benchmark_file> [--distribution [words]]")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()

    # Check for distribution mode
    if len(sys.argv) > 2 and sys.argv[2] == "--distribution":
        target_words = int(sys.argv[3]) if len(sys.argv) > 3 else None
        create_distribution_plot(sys.argv[1], target_words)
        exit()

    wrdrank_data, rank_data = parse_file(sys.argv[1])

    fig, ax = plt.subplots(1, 1, figsize=(14, 6))

    # No max_plots limit by default - show every words-per-block config
    make_overlap_plot(wrdrank_data, rank_data, ax, max_plots=None,
                      label0='wrdRank', label1='Rank')

    plt.suptitle(f'wrdRank vs Rank Benchmark: {sys.argv[1]}', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()

