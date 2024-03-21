import matplotlib.pyplot as plt

N_values = []
ordinary_times = []
optimize_times = []
unroll_times = []

with open('1.txt', 'r') as file:
    for line in file:
        if line.startswith('Testing with N = '):
            N_values.append(int(line.strip().split('= ')[1]))
        elif 'ordinary:' in line:
            ordinary_times.append(float(line.strip().split(':')[1].replace('ms', '')))
        elif 'optimize:' in line:
            optimize_times.append(float(line.strip().split(':')[1].replace('ms', '')))
        elif 'unroll:' in line:
            unroll_times.append(float(line.strip().split(':')[1].replace('ms', '')))

plt.figure(figsize=(10, 6))
plt.plot(N_values, ordinary_times, color='blue', linestyle='-', linewidth=2, markersize=5, label='Ordinary')
plt.plot(N_values, optimize_times, color='red', linestyle='-', linewidth=2, markersize=5, label='Optimize')
plt.plot(N_values, unroll_times, color='green', linestyle='-', linewidth=2, markersize=5, label='Unroll')

plt.title('Performance Comparison')
plt.xlabel('N Value')
plt.ylabel('Execution Time (ms)')

plt.legend()
plt.grid(True)
plt.show()
