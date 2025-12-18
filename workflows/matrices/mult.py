import numpy as np

with open('m1', 'r') as m1:
    U = np.array([list(map(float, line.split())) for line in m1.readlines()])
with open('m2', 'r') as m2:
    S = np.array([list(map(float, line.split())) for line in m2.readlines()])
with open('m3', 'r') as m3:
    VT = np.array([list(map(float, line.split())) for line in m3.readlines()])

A = U @ S @ VT
for row in A:
    print(*row)
