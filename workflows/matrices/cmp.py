import numpy as np

with open('matrix_in', 'r') as matrix_in:
    A_in = np.array([list(map(float, line.split())) for line in matrix_in.readlines()])
with open('matrix_out', 'r') as matrix_out:
    A_out = np.array([list(map(float, line.split())) for line in matrix_out.readlines()])
assert np.linalg.norm(A_in - A_out) < 1e-5
