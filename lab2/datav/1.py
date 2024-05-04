import matplotlib.pyplot as plt
# Data for the new visualization request
data_points = [1, 2, 3, 4, 5]  # General data point markers as the actual values are not specified
sse_dev_times = [4.36, 7.912, 80.62, 517.78, 2542.37]
avx256_dev_times = [3.84, 6.14, 57.55, 471.40, 2317.17]
avx512_dev_times = [1.6, 3.87, 52.49, 452.24, 2111.37]

# Plotting the data
plt.figure(figsize=(10, 6))
plt.plot(data_points, sse_dev_times, marker='o', linestyle='-', color='blue', label='SSE')
plt.plot(data_points, avx256_dev_times, marker='s', linestyle='--', color='green', label='AVX-256')
plt.plot(data_points, avx512_dev_times, marker='^', linestyle='-.', color='red', label='AVX-512')

# Adding labels, title, and legend
plt.xlabel('Data Point Index')
plt.ylabel('Execution Time (ms)')
plt.title('Development Execution Time Comparison')
plt.legend()
plt.grid(True)

# Show plot
plt.show()
