import matplotlib.pyplot as plt
"""
# 问题规模
problem_sizes = [200, 500, 1000, 2000, 3000, 4000, 5000, 6000]

# 串行时间/ms
serial_times = [19.284, 301.546, 2434.87, 19620.7, 65878.5, 158788, 259545.3, 448727.686]

# 并行时间/ms
parallel_times = [12.712, 197.319, 1592.47, 12837.6, 42919.8, 103591, 169738.75, 292886.5]

# 加速比
speedups = []
for i in range(len(problem_sizes)):
    speedups.append(serial_times[i]/parallel_times[i])
"""
# 问题规模
problem_sizes = [8399, 23045, 37960]

# 串行时间/ms
serial_times = [97.412, 227.282, 475.283]

# 并行时间/ms
parallel_times = [82.391, 216.58, 438.493]

# 加速比
speedups = []
for i in range(len(problem_sizes)):
    speedups.append(serial_times[i]/parallel_times[i])

# 绘制串行时间和并行时间
plt.figure(figsize=(10, 6))
plt.plot(problem_sizes, serial_times, marker='o', label='Serial Time')
plt.plot(problem_sizes, parallel_times, marker='o', label='Parallel Time')
plt.xlabel('Problem Size')
plt.ylabel('Time (ms)')
plt.title('Serial Time vs Parallel Time')
plt.legend()
plt.grid(True)
plt.show()

# 绘制加速比
plt.figure(figsize=(10, 6))
plt.plot(problem_sizes, speedups, marker='o')
plt.xlabel('Problem Size')
plt.ylabel('Speedup')
plt.title('Speedup vs Problem Size')
plt.grid(True)
plt.show()
