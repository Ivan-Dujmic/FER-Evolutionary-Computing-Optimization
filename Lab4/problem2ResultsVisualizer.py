import re
import matplotlib.pyplot as plt

file_path = 'results.txt'

fitness_x = []
fitness_y = []

fitness_pattern = re.compile(r'fitness=\[(.*?),\s*(.*?)\]')

with open(file_path, 'r') as f:
    for line in f:
        match = fitness_pattern.search(line)
        if match:
            x = float(match.group(1))
            y = float(match.group(2))
            fitness_x.append(x)
            fitness_y.append(y)

plt.figure(figsize=(8, 6))
plt.scatter(fitness_x, fitness_y, color='blue', alpha=0.6)
plt.title('Fitness Vectors')
plt.xlabel('Fitness 1')
plt.ylabel('Fitness 2')
plt.grid(True)
plt.show()
