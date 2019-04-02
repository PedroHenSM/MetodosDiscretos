import numpy as np
import matplotlib.pyplot as plt

def analitic(x, y):
    for i in range(len(x)):
        y.append(2*np.exp(-20*x[i]))

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


def mult_step(x, y, n, h):
    implicitEuler(x, y, 2, h)
    for i in range(n-2):
        y.append(-40 * h * y[i+1] + y[i])

def modifiedEuler(x, y, n, h):
    for i in range(n-1):
        yAux = (y[i] + (-y[i] + x[i] + 1/2) * h)  # ũ Explicito
        # y.append(y[i] / (-yAux + x[i+1] + 1/2) * h) # Implicito
        y.append(y[i] + (h/2.0) * ((-y[i] + x[i] + 1/2) - yAux + x[i+1] + 1/2)) #

def analitico(x, y):  # u' = -u + t + 1/2
    for t in np.arange(0, 1.001, 0.01):
        x.append(t)
        y.append(t + 3 / 2 * np.exp(-t) - 1 / 2)
    return x, y

if __name__ == '__main__':
    deltas = [1/9, 1/10, 1/11, 1/20]
    y_names = ["y_ee", "y_ie", "y_trap", "y_mult"]
    y_labels = ["ee", "ie", "trap", "unk"]
    y_dots = ["x", "o", "h", "+"]
    y_colors = ["blue", "red", "darkorange", "green"]
    # print(deltas)

    # First exercise
    x_analitic = np.linspace(0, 2, 100)
    y_analitic = []
    analitic(x_analitic, y_analitic)
	
    """
    for i in range(len(deltas)):
        y = [[2] for j in range(4)]
        n = int((2.0 - 0.0)/deltas[i]) + 1
        x = np.linspace(0., 2., n)
        explicitEuler(x, y[0], n, deltas[i])
        implicitEuler(x, y[1], n, deltas[i])
        trapezium(x, y[2], n, deltas[i])
        mult_step(x, y[3], n, deltas[i])

        for j in range(len(y_names)):
            plt.plot(x_analitic, y_analitic, color='black')
            plt.plot(x, y[j], y_dots[j], label=y_labels[j], color=y_colors[j])
            plt.legend(loc="best")
            plt.title(y_names[j]+ " delta="+str(deltas[i]))
            plt.autoscale(enable=True, axis='both', tight=None)
            plt.show()
    """

    # Code for modifiedEuler
    x_analitic = []
    y_analitic = []
    x_analitic, y_analitic = analitico(x_analitic, y_analitic)
    h = 0.1
    y = [1]
    n = int((1.0 - 0.0) / h) + 1
    x = np.linspace(0., 1., n)
    modifiedEuler(x, y, n, h)
    plt.plot(x_analitic, y_analitic, color="black")
    plt.plot(x, y, "x", label="Modified Euler", color="red")
    plt.legend(["Solução Analítica", "Euler Modificado"])
    plt.title("Euler Modificado (Método de Heun) h = 0.1")
    plt.show()