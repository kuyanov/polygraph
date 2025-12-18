fin_approx_old = open('approx_old')
approx_log = open('approx_log', 'a')
x = float(fin_approx_old.readline())
print('old:', x, file=approx_log)
fin_grad = open('grad')
grad = float(fin_grad.readline())
if abs(grad) > 1e-3:
    fout_approx_new = open('approx_new', 'w')
    print(x - grad * 0.1, file=fout_approx_new)
    print('new:', x - grad * 0.1, file=approx_log)
else:
    fout_solution = open('solution', 'w')
    print(x, file=fout_solution)
