import sys
import numpy as np

A = [list(map(float, line.split())) for line in sys.stdin.readlines()]
U, S, VT = np.linalg.svd(A)
with open('m1', 'w') as m1:
    for row in U:
        print(*row, file=m1)
with open('m2', 'w') as m2:
    for row in np.diag(S):
        print(*row, file=m2)
with open('m3', 'w') as m3:
    for row in VT:
        print(*row, file=m3)
