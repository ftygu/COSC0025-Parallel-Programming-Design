import matplotlib.pyplot as plt

# Given data
problem_sizes = [200, 500, 1000, 2000, 3000]
naive_times = [19.288, 300.876, 2509.59, 20975.7, 58691.8]
cache_optimized_times = [19.357, 300.75, 2503.35, 19635.5, 52580.6]

# Plotting the data
plt.figure(figsize=(10, 6))
plt.plot(problem_sizes, naive_times, label='Naive Algorithm (Serial)', marker='o')
plt.plot(problem_sizes, cache_optimized_times, label='Cache Optimized (Serial)', marker='s')
plt.title('Performance Comparison: Naive vs. Cache Optimized Algorithms')
plt.xlabel('Problem Size')
plt.ylabel('Execution Time (ms)')
plt.legend()
plt.grid(True)
plt.show()