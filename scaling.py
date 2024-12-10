#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import subprocess
import re
import argparse
import os

DEFAULT_RESOLUTIONS = [4000, 8000, 12000, 16000, 20000, 24000]

def compile_mandelbrot():
    """Compile the Mandelbrot C++ code"""
    subprocess.run(["make", "clean"], check=True)
    subprocess.run(["make"], check=True)

def run_mandelbrot(size):
    """Run the Mandelbrot program with specified size and return runtime"""
    with open('Mandelbrot.cc', 'r') as file:
        content = file.read()
    
    modified_content = re.sub(
        r'const int width = \d+;',
        f'const int width = {size};',
        content
    )
    modified_content = re.sub(
        r'const int height = \d+;',
        f'const int height = {size};',
        modified_content
    )
    
    with open('Mandelbrot.cc', 'w') as file:
        file.write(modified_content)
    
    compile_mandelbrot()
    result = subprocess.run(['./build/serial'], capture_output=True, text=True)
    
    match = re.search(r'Mandelbrot compute took: (\d+\.\d+)', result.stdout)
    if match:
        return float(match.group(1))
    return None

def perform_scaling_study(resolutions_to_test, plot=True):
    """Perform the scaling study and optionally plot results"""
    print("Starting Mandelbrot scaling study...")
    print(f"Testing resolutions: {resolutions_to_test}")
    
    sizes = np.array(resolutions_to_test)
    times = np.zeros(len(sizes))
    
    for i, size in enumerate(sizes):
        print(f"\nTesting resolution {size}x{size}")
        times[i] = run_mandelbrot(size)
        print(f"Runtime: {times[i]:.2f} seconds")
    
    results = np.column_stack((sizes, times))
    np.savetxt('scaling_results.txt', results, 
               header='Resolution Runtime(s)', 
               fmt=['%d', '%.3f'])
    
    if plot:
        plt.figure(figsize=(10, 6))
        plt.plot(sizes, times, 'bo-', label='Actual runtime')
        
        scaled_time = times[0] * (sizes/sizes[0])**2
        plt.plot(sizes, scaled_time, 'r--', label='O(n²) scaling')
        
        plt.xlabel('Resolution (n x n pixels)')
        plt.ylabel('Runtime (seconds)')
        plt.title('Mandelbrot Set Computation Scaling')
        plt.legend()
        plt.grid(True)
        plt.savefig('scaling_study.png')
        plt.close()
        
    return sizes, times

def main():
    parser = argparse.ArgumentParser(description='Mandelbrot Scaling Study')
    parser.add_argument('--no-plot', action='store_true', 
                       help='Run without generating plot')
    parser.add_argument('--resolutions', type=int, nargs='+',
                       help='Custom resolution sizes to test')
    args = parser.parse_args()
    
    # Use default resolutions unless custom ones are specified
    resolutions_to_test = args.resolutions if args.resolutions else DEFAULT_RESOLUTIONS
    
    sizes, times = perform_scaling_study(resolutions_to_test, plot=not args.no_plot)
    
    print("\nScaling Study Results:")
    print("Resolution\tRuntime(s)")
    for size, time in zip(sizes, times):
        print(f"{size}x{size}\t{time:.2f}")

if __name__ == "__main__":
    main()