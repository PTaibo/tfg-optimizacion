#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

runNum = 100

def parse_file(file_name):
    """Parse file and return grouped raw data (keyed by words per block only)"""
    data = {}  # wordsPerBlk -> list of cycle counts
    current_words = None

    with open(file_name) as f:
        lines = f.readlines()
        for line in lines:
            if "word per rank block" in line:
                val = re.search(r'\d+', line)
                if val:
                    current_words = int(val.group())
            elif "Unhalted clock cycles:" in line:
                val = re.search(r':\s*(\d+)', line)
                if val:
                    cycles = int(val.group(1))

                    if current_words is not None:
                        if current_words not in data:
                            data[current_words] = []
                        data[current_words].append(cycles)

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
        print(f"Removed {removed_count}/{len(values)} outliers (threshold: {threshold}σ)")

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

def make_plot_with_outlier_filtering(data, plot, max_plots=6):
    """Create bar plot with outlier-filtered means"""

    # Sort by words per block and limit to first N
    sorted_sizes = sorted(data.keys())[:max_plots]
    filtered_data = {}

    print(f"\nResults (outliers removed) - first {len(sorted_sizes)} configurations:")
    for words in sorted_sizes:
        filtered_values = filter_outliers(data[words])
        if filtered_values:
            filtered_data[words] = np.mean(filtered_values)
            label = f'{words}w'
            print(f"{label}: {filtered_data[words]:.2f} cycles")

    if not filtered_data:
        plot.text(0.5, 0.5, 'No data available', transform=plot.transAxes,
                  ha='center', va='center')
        return

    sizes = list(filtered_data.keys())
    means = list(filtered_data.values())
    labels = [f'{w}w' for w in sizes]

    x_coord = range(len(sizes))

    plot.bar(x_coord, means)
    plot.set_xticks(list(x_coord))
    plot.set_xticklabels(labels, rotation=45, ha='right')

    plot.set_xlabel('Words per block')
    plot.set_ylabel('Unhalted clock cycles (filtered)')
    plot.set_title('SIMD Rank Benchmark (filtered)')

def create_distribution_plot(file_name, target_words=None):
    """Create probability distribution for a given words-per-block setting"""
    data = parse_file(file_name)

    if target_words is None:
        # Default to the smallest words-per-block value
        target_words = min(data.keys()) if data else None

    if target_words is None or target_words not in data:
        print(f"No data found for words-per-block {target_words}")
        print(f"Available values: {sorted(data.keys())}")
        return

    values = data[target_words]
    config_label = f'{target_words}w'

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

    # Histogram with density
    ax1.hist(values, bins=30, alpha=0.7, color='skyblue', edgecolor='black', density=True)
    ax1.set_xlabel('Unhalted clock cycles')
    ax1.set_ylabel('Probability Density')
    ax1.set_title(f'Distribution: {config_label}')

    # Add mean and median lines
    mean_val = np.mean(values)
    median_val = np.median(values)
    std_val = np.std(values)

    ax1.axvline(mean_val, color='red', linestyle='--', linewidth=2, label=f'Mean: {mean_val:.0f}')
    ax1.axvline(median_val, color='green', linestyle='--', linewidth=2, label=f'Median: {median_val:.0f}')
    ax1.legend()

    # KDE plot (if scipy available)
    try:
        from scipy import stats
        kde = stats.gaussian_kde(values)
        x_range = np.linspace(min(values), max(values), 1000)
        ax1.plot(x_range, kde(x_range), 'r-', linewidth=2, label='KDE')
        ax1.legend()
    except ImportError:
        pass

    # Box plot for additional visualization
    ax2.boxplot(values, vert=False)
    ax2.set_xlabel('Unhalted clock cycles')
    ax2.set_yticks([1])
    ax2.set_yticklabels([config_label])
    ax2.set_title(f'Box Plot: {config_label}')

    # Add statistics text
    stats_text = (f'Mean: {mean_val:.1f}\n'
                  f'Std Dev: {std_val:.1f}\n'
                  f'Median: {median_val:.1f}\n'
                  f'Q1: {np.percentile(values, 25):.1f}\n'
                  f'Q3: {np.percentile(values, 75):.1f}')
    ax2.text(0.02, 0.95, stats_text, transform=ax2.transAxes,
             verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Usage: python looped.py <benchmark_file> [--distribution [words]]")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()

    # Check for distribution mode
    if len(sys.argv) > 2 and sys.argv[2] == "--distribution":
        target_words = int(sys.argv[3]) if len(sys.argv) > 3 else None
        create_distribution_plot(sys.argv[1], target_words)
        exit()

    data = parse_file(sys.argv[1])

    # Optional: limit number of bars (default 6)
    max_plots = 6

    fig, ax = plt.subplots(1, 1, figsize=(10, 6))

    make_plot_with_outlier_filtering(data, ax, max_plots=max_plots)

    plt.suptitle(f'SIMD Rank Benchmark: {sys.argv[1]}', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()

