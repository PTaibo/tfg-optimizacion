#!/bin/python

import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

def parse_file(file_name):
    """Parse file and return grouped raw data"""
    data = {}
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

def create_distribution_plot(file_name, bits=32, words=1):
    """Create comprehensive probability distribution plot"""
    data = parse_file(file_name)
    
    key = (bits, words)
    if key not in data:
        print(f"No data found for {bits}-bit, {words}-word configuration")
        print(f"Available configurations: {list(data.keys())}")
        return
    
    values = data[key]
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # 1. Histogram with KDE
    ax1 = axes[0, 0]
    n, bins, patches = ax1.hist(values, bins=30, alpha=0.7, color='lightblue', 
                               edgecolor='black', density=True)
    
    # Calculate and add KDE
    try:
        from scipy import stats
        kde = stats.gaussian_kde(values)
        x_range = np.linspace(min(values), max(values), 1000)
        ax1.plot(x_range, kde(x_range), 'r-', linewidth=2, label='KDE')
    except ImportError:
        # Fallback: simple normal curve
        mu, sigma = np.mean(values), np.std(values)
        x = np.linspace(mu - 4*sigma, mu + 4*sigma, 1000)
        y = (1/(sigma * np.sqrt(2 * np.pi))) * np.exp(-0.5 * ((x - mu)/sigma)**2)
        ax1.plot(x, y, 'r-', linewidth=2, label='Normal Fit')
    
    mean_val = np.mean(values)
    median_val = np.median(values)
    ax1.axvline(mean_val, color='darkred', linestyle='--', linewidth=2, 
               label=f'Mean: {mean_val:.1f}')
    ax1.axvline(median_val, color='darkgreen', linestyle='--', linewidth=2, 
               label=f'Median: {median_val:.1f}')
    
    ax1.set_xlabel('Unhalted Clock Cycles')
    ax1.set_ylabel('Probability Density')
    ax1.set_title(f'Distribution: {bits}-bit, {words} word/block')
    ax1.legend(loc='best')
    ax1.grid(True, alpha=0.3)
    
    # 2. Cumulative Distribution Function
    ax2 = axes[0, 1]
    sorted_values = np.sort(values)
    cdf = np.arange(1, len(sorted_values)+1) / len(sorted_values)
    
    ax2.plot(sorted_values, cdf, 'b-', linewidth=2)
    ax2.axvline(mean_val, color='darkred', linestyle='--', alpha=0.7)
    ax2.axvline(np.percentile(sorted_values, 25), color='orange', linestyle=':', 
               label='Q1 (25%)')
    ax2.axvline(np.percentile(sorted_values, 75), color='purple', linestyle=':', 
               label='Q3 (75%)')
    ax2.set_xlabel('Unhalted Clock Cycles')
    ax2.set_ylabel('Cumulative Probability')
    ax2.set_title('Cumulative Distribution Function (CDF)')
    ax2.legend(loc='lower right')
    ax2.grid(True, alpha=0.3)
    
    # 3. Box Plot with outliers marked
    ax3 = axes[1, 0]
    boxprops = dict(linewidth=2, color='darkblue')
    medianprops = dict(linewidth=2, color='red')
    whiskerprops = dict(linewidth=1.5, color='gray')
    capprops = dict(linewidth=1.5, color='gray')
    
    bp = ax3.boxplot(values, vert=True, patch_artist=True,
                    boxprops=boxprops, medianprops=medianprops,
                    whiskerprops=whiskerprops, capprops=capprops)
    
    # Color the box
    for patch in bp['boxes']:
        patch.set_facecolor('lightblue')
    
    ax3.set_ylabel('Unhalted Clock Cycles')
    ax3.set_title('Box Plot with Outliers')
    ax3.set_xticks([1])
    ax3.set_xticklabels([f'{bits}b-{words}w'])
    ax3.grid(True, alpha=0.3, axis='y')
    
    # Mark outliers explicitly
    q1 = np.percentile(values, 25)
    q3 = np.percentile(values, 75)
    iqr = q3 - q1
    lower_bound = q1 - 1.5 * iqr
    upper_bound = q3 + 1.5 * iqr
    
    outliers = [v for v in values if v < lower_bound or v > upper_bound]
    if outliers:
        ax3.scatter([1]*len(outliers), outliers, color='red', alpha=0.6, 
                   s=50, label='Outliers', zorder=5)
        ax3.legend()
    
    # 4. Statistics Summary
    ax4 = axes[1, 1]
    ax4.axis('off')
    
    # Calculate additional statistics
    min_val = np.min(values)
    max_val = np.max(values)
    std_dev = np.std(values)
    variance = np.var(values)
    skewness = (np.mean((values - mean_val)**3) / std_dev**3) if std_dev > 0 else 0
    kurtosis = (np.mean((values - mean_val)**4) / std_dev**4) - 3 if std_dev > 0 else 0
    
    stats_info = f"""Statistics for {bits}-bit, {words} word/block:
    
Sample Size: {len(values)}
Minimum: {min_val:,}
Maximum: {max_val:,}
Mean: {mean_val:,.1f}
Median: {median_val:,.1f}
Std Dev: {std_dev:,.1f}
Variance: {variance:,.1f}
Q1 (25%): {np.percentile(values, 25):,.1f}
Q3 (75%): {np.percentile(values, 75):,.1f}
IQR: {iqr:,.1f}
Skewness: {skewness:.3f}
Kurtosis: {kurtosis:.3f}

Outliers (>1.5×IQR): {len(outliers)}
Range: {max_val - min_val:,}
"""
    
    ax4.text(0.1, 0.8, stats_info, transform=ax4.transAxes,
            verticalalignment='top', fontfamily='monospace', fontsize=10,
            bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8, pad=0.5))
    
    plt.suptitle(f'Probability Distribution Analysis: {bits}-bit, {words} word/block', 
                fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    file_name = "rank_looped.txt"
    if len(sys.argv) > 1:
        file_name = sys.argv[1]
    
    if not os.path.exists(file_name):
        print(f"File not found: {file_name}")
        exit()
    
    print(f"Generating distribution plot for 32-bit, 1 word/block...")
    create_distribution_plot(file_name, bits=32, words=1)
