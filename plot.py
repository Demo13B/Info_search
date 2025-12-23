import matplotlib.pyplot as plt
import numpy as np


x = [np.log(i) for i in range(1, 129014)]
y = []

with open("zipf.txt") as f:
    for line in f.readlines():
        y.append(np.log(int(line.strip())))

perfect_y = [y[0] - el for el in x]

plt.figure(figsize=(16, 9))
plt.plot(x, y, label='Token frequencies in log-log scale')
plt.xlabel('log(rank)')
plt.ylabel('log(frequency)')
plt.legend()
plt.grid()
plt.show()
