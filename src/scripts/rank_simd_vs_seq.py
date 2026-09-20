#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

runNum = 100

def parse_file(file_name):
    """Parse a single Rank benchmark file (SIMD or sequential/looped).

    Unlike the Select0/Select1 file, each run here carries a single plain
    "Unhalted clock cycles" line (no Select0:/Select1: prefix).

    Returns a dict keyed by wordsPerBlk -> list of cycle counts.
    """
    data = {}
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

            m = re.search(r'Unhalted clock cycles:\s*(\d+)', line)
            if m:
                data.setdefault(current_words, []).append(int(m.group(1)))
                continue

    return data


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


def make_overlap_plot(simd_data, looped_data, plot, max_plots=None,
                       label0='SIMD Rank', label1='Sequential Rank',
                       color0='royalblue', color1='gold'):
    """Bar plot with SIMD Rank and Sequential Rank overlaid at the same x position.

    Both series share the same x-axis (words per block). Bars are drawn
    on top of one another with alpha transparency so the shorter series
    is visible inside the taller one, mirroring the reference comparison
    plot (blue = SIMD Rank total height, yellow/olive = Sequential Rank
    poking through underneath).
    """

    # All words-per-block values present in either series, sorted
    all_words = sorted(set(simd_data.keys()) | set(looped_data.keys()))
    if max_plots is not None:
        all_words = all_words[:max_plots]

    means0 = {}
    means1 = {}

    print(f"\nResults (outliers removed) - {len(all_words)} configurations:")
    for words in all_words:
        if words in simd_data:
            filtered0 = filter_outliers(simd_data[words])
            if filtered0:
                means0[words] = np.mean(filtered0)
        if words in looped_data:
            filtered1 = filter_outliers(looped_data[words])
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
    plot.set_title('SIMD Rank vs Sequential Rank Benchmark (overlapped)')
    plot.legend()


def create_distribution_plot(simd_file, looped_file, target_words=None):
    """Create probability distributions for SIMD Rank and Sequential Rank side
    by side for a given words-per-block setting."""
    simd_data = parse_file(simd_file)
    looped_data = parse_file(looped_file)

    if target_words is None:
        all_keys = set(simd_data.keys()) | set(looped_data.keys())
        target_words = min(all_keys) if all_keys else None

    if target_words is None or (target_words not in simd_data and target_words not in looped_data):
        print(f"No data found for words-per-block {target_words}")
        print(f"Available values: {sorted(set(simd_data.keys()) | set(looped_data.keys()))}")
        return

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    for ax, data, label, color in (
        (axes[0], simd_data, 'SIMD Rank', 'royalblue'),
        (axes[1], looped_data, 'Sequential Rank', 'gold'),
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
    if len(sys.argv) <= 2:
        print("Usage: python rank_compare.py <simd_file> <looped_file> [--distribution [words]]")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The SIMD file doesn't exist")
        exit()

    if not os.path.exists(sys.argv[2]):
        print("The sequential/looped file doesn't exist")
        exit()

    # Check for distribution mode
    if len(sys.argv) > 3 and sys.argv[3] == "--distribution":
        target_words = int(sys.argv[4]) if len(sys.argv) > 4 else None
        create_distribution_plot(sys.argv[1], sys.argv[2], target_words)
        exit()

    simd_data = parse_file(sys.argv[1])
    looped_data = parse_file(sys.argv[2])

    fig, ax = plt.subplots(1, 1, figsize=(14, 6))

    # No max_plots limit by default - show every words-per-block config
    make_overlap_plot(simd_data, looped_data, ax, max_plots=None,
                       label0='SIMD Rank', label1='Sequential Rank')

    plt.suptitle(f'SIMD Rank vs Sequential Rank Benchmark: {sys.argv[1]} vs {sys.argv[2]}',
                 fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()
