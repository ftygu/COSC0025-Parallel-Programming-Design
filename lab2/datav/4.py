# Plotting without specific text description in the title regarding color differentiation

plt.figure(figsize=(10, 6))
plt.plot(problem_data['problem_size'], problem_data['serial_time'], marker='o', color='navy', label='Serial Time')
plt.plot(problem_data['problem_size'], problem_data['parallel_time'], marker='o', color='orange', label='Parallel Time')
plt.plot(problem_data['problem_size'], problem_data['parallel_aligned_time'], marker='o', color='limegreen', label='Parallel + Aligned Time')

# Formatting
plt.xlabel('Problem Size')
plt.ylabel('Time (ms)')
plt.title('Time Comparison for Different Problem Sizes')
plt.legend()
plt.grid(True)

plt.show()
