import numpy as np
import matplotlib.pyplot as plt


def explicitEuler(x, y, n, h):
    for i in range(n-1):
        y.append(y[i] - 20*h*y[i])


def implicitEuler(x, y, n, h):
    for i in range(n-1):
        y.append(y[i] / (1+20*h))


def trapezium(x, y, n, h):
    for i in range(n-1):
        y.append(y[i] * (1 - 20*h/2) / (1 + 20*h/2))
        # verify if a = 20 or a = -20 (if a = 20 change equation signal)


def unkown(x, y, n, h):
    implicitEuler(x, y, 2, h)
    for i in range(n-2):
        y.append(-40 * h * y[i+1] + y[i])

if __name__ == '__main__':
    deltas = [1/9, 1/10, 1/11, 1/20]
    y_names = ["y_ee", "y_ie", "y_trap", "y_unk"]
    y_labels = ["ee", "ie", "trap", "unk"]
    y_dots = ["x", "o", "*", "+"]
    y_colors = ["blue", "red", "yellow", "green"]
    # print(deltas)
    for i in range(len(deltas)):
        y = [[2] for j in range(4)]
        n = int((2.0 - 0.0)/deltas[i]) + 1
        x = np.linspace(0., 2., n)
        explicitEuler(x, y[0], n, deltas[i])
        implicitEuler(x, y[1], n, deltas[i])
        trapezium(x, y[2], n, deltas[i])
        unkown(x, y[3], n, deltas[i])
        for j in range(len(y_names)):
            plt.plot(x, y[j], y_dots[j], label=y_labels[j], color=y_colors[j])
            plt.legend(loc="best")
            plt.show()
