#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

runNum = 100

def parse_file(file_name):
    """Parse file and return grouped raw data (keyed by bitmap size)"""
    data = {}  # bmap_size -> list of timing values (us)
    current_size = None

    with open(file_name) as f:
        lines = f.readlines()
        for line in lines:
            if "elements in bitmap" in line:
                val = re.search(r'\d+', line)
                if val:
                    current_size = int(val.group())
            elif re.search(r'\d+\s*us\s*$', line.strip()):
                val = re.search(r'\d+', line)
                timing = int(val.group())

                if current_size is not None:
                    if current_size not in data:
                        data[current_size] = []
                    data[current_size].append(timing)

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

def format_size_label(size):
    """Format bitmap size as human-readable label"""
    if size >= 1_000_000_000:
        return f'{size // 1_000_000_000}B'
    elif size >= 1_000_000:
        return f'{size // 1_000_000}M'
    elif size >= 1_000:
        return f'{size // 1_000}K'
    else:
        return str(size)

def calculate_cleaned_means(data):
    """Calculate means after outlier removal for each configuration"""
    results = []

    for bmap_size, values in data.items():
        filtered_values = filter_outliers(values)
        if filtered_values:
            mean_val = np.mean(filtered_values)
            results.append([bmap_size, mean_val])

    return results

def make_plot_with_outlier_filtering(data, plot):
    """Create bar plot with outlier-filtered means (log-scale x-axis)"""

    # Sizes 100 - 1_000_000_000
    MAX_SIZE = 1_000_000

    # Sort by bitmap size
    sorted_sizes = sorted(s for s in data.keys() if s <= MAX_SIZE)
    filtered_data = {}

    print(f"\nResults (outliers removed):")
    for bmap_size in sorted_sizes:
        filtered_values = filter_outliers(data[bmap_size])
        if filtered_values:
            filtered_data[bmap_size] = np.mean(filtered_values)
            label = format_size_label(bmap_size)
            print(f"{label}: {filtered_data[bmap_size]:.2f} us")

    if not filtered_data:
        plot.text(0.5, 0.5, 'No data available', transform=plot.transAxes,
                  ha='center', va='center')
        return

    sizes = list(filtered_data.keys())
    means = list(filtered_data.values())
    labels = [format_size_label(s) for s in sizes]

    x_coord = range(len(sizes))

    plot.bar(x_coord, means)
    plot.set_xticks(list(x_coord))
    plot.set_xticklabels(labels, rotation=45, ha='right')

    plot.set_xlabel('Bitmap size (elements)')
    plot.set_ylabel('Execution time (µs)')
    plot.set_title('CUDA bitmap benchmark (filtered)')

def create_distribution_plot(file_name, target_size=None):
    """Create probability distribution for a given bitmap size"""
    data = parse_file(file_name)

    if target_size is None:
        # Default to the smallest bitmap size
        target_size = min(data.keys()) if data else None

    if target_size is None or target_size not in data:
        print(f"No data found for bitmap size {target_size}")
        print(f"Available sizes: {sorted(data.keys())}")
        return

    values = data[target_size]
    size_label = format_size_label(target_size)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

    # Histogram with density
    ax1.hist(values, bins=30, alpha=0.7, color='skyblue', edgecolor='black', density=True)
    ax1.set_xlabel('Execution time (µs)')
    ax1.set_ylabel('Probability Density')
    ax1.set_title(f'Distribution: {size_label} elements')

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
    ax2.set_xlabel('Execution time (µs)')
    ax2.set_yticks([1])
    ax2.set_yticklabels([size_label])
    ax2.set_title(f'Box Plot: {size_label} elements')

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
        print("Usage: python looped.py <benchmark_file> [--distribution [size]]")
        exit()

    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()

    # Check for distribution mode
    if len(sys.argv) > 2 and sys.argv[2] == "--distribution":
        target_size = int(sys.argv[3]) if len(sys.argv) > 3 else None
        create_distribution_plot(sys.argv[1], target_size)
        exit()

    data = parse_file(sys.argv[1])

    fig, ax = plt.subplots(1, 1, figsize=(10, 6))

    make_plot_with_outlier_filtering(data, ax)

    plt.suptitle(f'Bitmap Size Benchmark: {sys.argv[1]}', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()

