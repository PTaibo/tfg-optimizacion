#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

runNum = 100

def parse_file(file_name):
    """Parse file and return grouped raw data"""
    data = {}  # (bits, wordsPerBlk) -> list of cycle counts
    cntr = 0
    current_bits = None
    current_words = None
    
    with open(file_name) as f:
        lines = f.readlines()
        for line in lines:
            if "bit word" in line:
                val = re.search(r'\d+', line)
                current_bits = int(val.group())
            elif "word per rank block" in line:
                val = re.search(r'\d+', line)
                current_words = int(val.group())
            elif "clock cycles" in line:
                val = re.search(r'\d+', line)
                cycles = int(val.group())
                
                key = (current_bits, current_words)
                if key not in data:
                    data[key] = []
                data[key].append(cycles)
    
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
    
    # Group by bits
    bits_groups = {}
    for (bits, words), values in data.items():
        if bits not in bits_groups:
            bits_groups[bits] = {}
        
        # Filter outliers and calculate mean
        filtered_values = filter_outliers(values)
        if filtered_values:
            mean_val = np.mean(filtered_values)
            # Store as (bits, words, normalized_index, mean_value)
            results.append([bits, words, 0, mean_val])  # Normalized index will be set later
    
    return results

def make_plot_with_outlier_filtering(data, bits, plot):
    """Create bar plot with outlier-filtered means"""
    bits = str(bits)
    
    x_label = ['1w', '2w', '10w', '20w', '30w', '100w']
    x_coord = [i+1 for i in range(len(x_label))]
    height = []
    
    # Get all data points for this bit width
    filtered_data = {}
    for (b, w), values in data.items():
        if b == int(bits):
            filtered_values = filter_outliers(values)
            if filtered_values:
                filtered_data[w] = np.mean(filtered_values)
    
    print(f"\n{bits}-bit word results (outliers removed):")
    for label, word_count in zip(x_label, [1, 2, 10, 20, 30, 100]):
        if word_count in filtered_data:
            val = filtered_data[word_count]
            height.append(val)
            print(f"{label}: {val:.2f} cycles")
        else:
            height.append(0)
            print(f"{label}: No data available")
    
    # Remove zeros if no data
    non_zero_heights = [h for h in height if h != 0]
    non_zero_coords = [x for x, h in zip(x_coord, height) if h != 0]
    non_zero_labels = [l for l, h in zip(x_label, height) if h != 0]
    
    if non_zero_heights:
        plot.bar(non_zero_coords, non_zero_heights)
        plot.set_xticks(non_zero_coords)
        plot.set_xticklabels(non_zero_labels, rotation=90, ha='right')
    else:
        plot.text(0.5, 0.5, 'No data available', transform=plot.transAxes, 
                 ha='center', va='center')
    
    plot.set_xlabel('Words per block')
    plot.yaxis.set_tick_params(labelleft=True)
    plot.set_title(f'{bits} bit words (filtered)')

def create_distribution_plot(file_name):
    """Create probability distribution for 32b 1w data"""
    data = parse_file(file_name)
    
    # Extract 32-bit, 1-word data
    key = (32, 1)
    if key not in data:
        print(f"No data found for 32-bit, 1-word configuration")
        return
    
    values = data[key]
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    
    # Histogram with density
    ax1.hist(values, bins=30, alpha=0.7, color='skyblue', edgecolor='black', density=True)
    ax1.set_xlabel('Unhalted clock cycles')
    ax1.set_ylabel('Probability Density')
    ax1.set_title('Distribution: 32-bit, 1 word/block')
    
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
    ax2.set_yticklabels(['32-bit, 1w'])
    ax2.set_title('Box Plot: 32-bit, 1 word/block')
    
    # Add statistics text
    stats_text = f'Mean: {mean_val:.1f}\nStd Dev: {std_val:.1f}\nMedian: {median_val:.1f}\nQ1: {np.percentile(values, 25):.1f}\nQ3: {np.percentile(values, 75):.1f}'
    ax2.text(0.02, 0.95, stats_text, transform=ax2.transAxes, 
             verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Usage: python mean_cycles_filtered.py <benchmark_file> [--distribution]")
        exit()
    
    if not os.path.exists(sys.argv[1]):
        print("The file doesn't exist")
        exit()
    
    # Check for distribution mode
    if len(sys.argv) > 2 and sys.argv[2] == "--distribution":
        create_distribution_plot(sys.argv[1])
        exit()
    
    data = parse_file(sys.argv[1])
    
    _, axs = plt.subplots(1, 2, sharey=True, figsize=(12, 6))
    axs[0].set_ylabel('Unhalted core cycles (filtered)')
    
    # Create plots for 32-bit and 64-bit
    make_plot_with_outlier_filtering(data, 32, axs[0])
    make_plot_with_outlier_filtering(data, 64, axs[1])
    
    plt.suptitle(f'Benchmark: {sys.argv[1]} (Outlier Filtering)', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()

