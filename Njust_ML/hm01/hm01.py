import numpy as np
import matplotlib.pyplot as plt
import handwritten as hw
import matplotlib as mpl



mpl.rcParams['font.sans-serif'] = ['KaiTi']
mpl.rcParams['font.serif'] = ['KaiTi']
class LinearRegression:
    def __init__(self, input_num, output_num, lr=0.01):
        self.lr = lr
        self.seq = [
            hw.LinearUnit(input_num, output_num, lr)
        ]
        self.input = np.zeros(10)
        self.y_hat = 0

    def forward(self, inputs):
        ret = inputs
        self.input = inputs
        for unit in self.seq:
            #print(unit)
            #print(type(ret))
            ret = unit.forward(ret)
            #print(ret[0])
        self.y_hat = ret
        return ret

    def backward(self, label):
        grad = hw.loss_MSE_grad(self.y_hat, label)  # cross entropy loss grad
        # print(grad)
        for unit in reversed(self.seq):
            grad = unit.backward(grad)



Y = np.array([2.000, 2.500, 2.900, 3.147, 4.515, 4.903, 5.365, 5.704, 6.853, 7.971,
              8.561, 10.000, 11.280, 12.900]).reshape(-1, 1)
plt.ion()

fig, ax = plt.subplots(3, 1)
plt.subplots_adjust(wspace=0, hspace=0.5)
fig.dpi = 768
ax[0].set_title("解析法")
ax[2].set_title("loss")
X = np.concatenate((np.array(range(2000, 2014), dtype=float).reshape(-1, 1), np.ones((14, 1))), axis=1)
theta = np.dot(np.dot(np.linalg.inv(np.dot(X.T, X)), X.T), Y)
y_hat = X.dot(theta)
ax[0].plot(X[:, 0:1], y_hat)
X = np.array(range(2000, 2014), dtype=float).reshape(-1, 1)
ax[0].plot(X, Y, 'o')
mean = X.mean()
X -= mean
input_num = X.shape[1]
output_num = Y.shape[1]
lr = 1.3
Net = LinearRegression(input_num, output_num, lr)
loss = hw.MSELoss
l_list = []
for i in range(int(1e2)):
    y_hat = Net.forward(X)
    Net.backward(Y)
    l = loss(y_hat, Y)
    ax[1].cla()
    ax[1].set_title("梯度下降法")
    ax[1].plot(X+mean, Y, 'o')
    ax[1].plot(X+mean, y_hat)
    ax[2].plot(i, l, 'r.')
    plt.pause(0.1)

plt.ioff()
plt.savefig('D:/机器学习/hm01/hm01.png')
plt.show()

y_hat = Net.forward(X)
print(y_hat)
print('loss : ', loss(y_hat, Y))




# X = [2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013]


