import sys
import numpy as np

n = int(sys.argv[1])
A = np.random.rand(n, n)
for row in A:
    print(*row)
