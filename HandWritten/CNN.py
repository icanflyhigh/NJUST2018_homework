import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


# 首先我需要写一个Linear
EPS = 1e-10
def regular(data):
    mean = np.array(data.mean(axis=1, keepdims=True))
    ret = data - mean
    std = np.array(ret.std(axis=1, keepdims=True))
    ret /= np.sqrt(std+EPS)
    return ret






# -------------------------------------------------------

#  linear regression

def MSELoss(y_hat, y):
    return (y_hat - y)**2


def Linear(w, b, X):
    return np.dot(X, w) + b

# ----------------------------------------------------------


# logistic regression _______________________________________

def sigmoid(z):
    return 1.0/(1.0+np.exp(-z))

def crossEntropy(y_hat, y):
    return -y*np.log(y_hat)-(1-y)*np.log(1-y_hat)

# -----------------------------------------------------------



# neural network______________________________________________

class Unit:
    def __init__(self, lr=0.0):
        self.lr = lr



class LinearUnit(Unit):
    def __init__(self, input_num, output_num, lr):
        super(LinearUnit, self).__init__(lr)
        self.w = np.random.randn(input_num, output_num)*EPS
        # self.w = np.zeros((input_num, output_num))
        self.b = np.random.randn(1, output_num)
        self.input = np.array([])

    def forward(self, input):
        self.input = input
        return np.dot(input, self.w) + self.b

    def backward(self, grad):    # i  *  o
        db = grad.mean(axis=0, keepdims=True)
        dw = np.dot(np.transpose(self.input), grad) / self.input.shape[0]
        din = np.dot(grad, np.transpose(self.w))
        self.b -= self.lr * db
        self.w -= self.lr * dw
        return din


class ReLUUnit(Unit):
    def __init__(self):
        super(ReLUUnit, self).__init__()
        self.input = 0


    def forward(self, input):
        input[input < 0] = 0
        self.input = input
        return input

    def backward(self, grad):
        self.input[self.input > 0] = 1
        return self.input * grad


class SoftMaxUnit(Unit):
    def __init__(self):
        super(SoftMaxUnit, self).__init__()
        self.y = 0.0

    def forward(self, input):
        input = input - input.max(axis=1, keepdims=True)  # 小细节防止数值溢出
        input = np.exp(input)
        input = input / input.sum(axis=1, keepdims=True)
        self.y = input
        return input


    def backward(self, grad):
        temp = self.y * grad
        return temp - self.y * temp.sum(axis=1, keepdims=True)


#  接下来 该是卷积神经网络登场了_____________________________________________
#  不知道能不能够实现这个~
#  该单元只支持2d卷积
class ConvUnit(Unit):
    def __init__(self, lr, channels=1, kernel_size=3, stride=1, padding=0):
        super(ConvUnit, self).__init__(lr)
        self.kernel_size = kernel_size
        self.kernel = np.random.randn(channels, kernel_size, kernel_size)
        self.stride = stride
        self.padding = padding
        self.channels = channels
        self.p_input = np.array([])
        self.in_size = (0, 0)
# 我操 我忘记了 batch的那个维度！！！

    def forward(self, mat):
        # 认为矩阵输入是 x, y, channels
        margin = np.int(self.kernel_size / 2)
        row_k = int((mat.shape[1] - self.kernel_size + 2.0 * self.padding) / self.stride + 1)
        col_k = int((mat.shape[2] - self.kernel_size + 2.0 * self.padding) / self.stride + 1)
        ret = np.zeros((mat.shape[0], row_k, col_k, self.channels))
        self.in_size = mat.shape
        mat = mat.sum(axis=3)

        # padding
        mat = np.insert(mat, [0], np.zeros((1, self.padding, mat.shape[2])), axis=1)
        mat = np.insert(mat, [mat.shape[1]], np.zeros((1, self.padding, mat.shape[2])), axis=1)
        mat = np.c_[np.zeros((mat.shape[0], mat.shape[1], self.padding)), mat, np.zeros((mat.shape[0], mat.shape[1],
                                                                                         self.padding))]
        pt = mat
        row = pt.shape[1]
        col = pt.shape[2]
        self.p_input = pt

        for num in range(pt.shape[0]):
            rk = 0
            for r in range(0, row, self.stride):
                ck = 0
                if r + margin >= row or r - margin < 0:
                    continue
                for c in range(0, col, self.stride):
                    if c + margin >= col or c - margin < 0:
                        continue
                    ans = np.sum((self.kernel *
                                  pt[num][r-margin:r+margin+1, c-margin:c+margin+1]).reshape(self.channels, -1), axis=1)

                    # print(ck, rk, num)
                    # print(r, c)
                    ret[num][rk][ck] = ans
                    ck += 1
                rk += 1
        return ret


    def backward(self, grad):
        dk = np.zeros((self.channels, self.kernel_size, self.kernel_size))
        din = np.zeros((self.p_input.shape[0], self.p_input.shape[1], self.p_input.shape[2]))
        # ker = np.array(self.kernel)
        # for i in range(self.kernel.shape[0]):
        #     ker[i] = np.flip(ker[i])
        # 处理dk
        bnum = self.p_input.shape[0]
        row = self.p_input.shape[1]
        col = self.p_input.shape[2]
        margin = np.int(self.kernel_size / 2)
        for num in range(bnum):
            rg = 0
            for r in range(0, row, self.stride):
                if r+margin >= row or r-margin < 0:
                    continue
                cg = 0
                for c in range(0, col, self.stride):
                    if c + margin >= col or c - margin < 0:
                        continue
                    for chan in range(self.channels):
                        dk[chan] = dk[chan] + \
                                   self.p_input[num][r-margin:r+margin+1, c-margin:c+margin+1]*grad[num][rg][cg][chan]

                        din[num][r-margin:r+margin+1, c-margin:c+margin+1] = \
                            din[num][r-margin:r+margin+1, c-margin:c+margin+1] + \
                            grad[num][rg][cg][chan] * self.kernel[chan]

                    cg += 1
                rg += 1
        self.kernel -= self.lr / din.shape[0] * dk
        # 处理din
        din = din.reshape((din.shape[0], din.shape[1], din.shape[2], 1))
        din = np.concatenate([din]*self.in_size[3], 2)
        return din[:, margin:din.shape[0]-margin, margin:din.shape[1]-margin, :]

class MaxPoolUnit:
    def __init__(self, size, stride):
        self.size = size
        self.stride = stride
        self.max_idx = np.array([])
        self.input_size = (0, 0, 0, 0)

    def forward(self, input):
        self.input_size = input.shape
        row = np.int((input.shape[1] - self.size) / self.stride + 1)
        col = np.int((input.shape[2] - self.size) / self.stride + 1)
        bnum = input.shape[0]
        self.max_idx = np.zeros((bnum, row, col, input.shape[3], 2), dtype=int)
        margin = np.int(self.size/2)
        ret = np.zeros((bnum, row, col, input.shape[3]))
        for num in range(bnum):
            rr = 0
            for r in range(0, row, self.stride):
                rc = 0
                if r + margin >= row or r - margin < 0:
                    continue
                for c in range(0, col, self.stride):
                    if c + margin >= col or c - margin < 0:
                        continue
                    for chan in range(input.shape[3]):
                        # print(r+margin+1)
                        ret[num][rr][rc][chan] = np.max(input[num][r-margin:r+margin+1,
                                                        c-margin:c+margin+1, chan:chan+1])
                        idx = np.argmax(input[num][r-margin:r+margin+1, c-margin:c+margin+1, chan:chan+1])

                        self.max_idx[num][rr][rc][chan][0] = np.int(idx / self.size)
                        self.max_idx[num][rr][rc][chan][1] = np.int(np.mod(idx, self.size))
                    rc += 1
                rr += 1
        return ret

    def backward(self, grad):
        ret = np.zeros(self.input_size)
        bnum = grad.shape[0]
        for num in range(bnum):
            for i in range(grad.shape[1]):
                for j in range(grad.shape[2]):
                    for chan in range(grad.shape[3]):
                        idx = self.max_idx[num][i][j][chan]
                        ret[num][idx[0]][idx[1]][chan] += grad[num][i][j][chan]

        return ret

class FlattenUnit:
    def __init__(self):
        self.size = (0, 0, 0)

    def forward(self, input):
        # print(input.shape)
        self.size = input.shape
        return input.reshape(input.shape[0], -1)

    def backward(self, grad):
        return grad.reshape(self.size)
# def train():
#     np.random.shuffle(pd_data)
#     train_data = pd_data[:train_size, :]
#     test_data = pd_data[train_size:-1, :]
#
#     lr = 0.9
#     net = LinearUnit(feature_num, 1, lr=lr)
#     epoch_num = 5
#     for epoch in range(epoch_num):
#         z = net.forward(train_data[:, :-1])
#         # z = Linear(w, b, train_data[:, :-1])    # m*n * n*1
#         y_hat = sigmoid(z)
#         dz = y_hat - train_data[:, -1:]
#         net.backward(dz)
#         ###test###
#         loss = 0.0
#         count = 0
#         count2 = 0
#         for sample in train_data:
#             y_hat = sigmoid(net.forward(sample[:-1]))
#             if abs(y_hat - sample[-1]) < 0.5:
#                 count2 += 1
#         for sample in test_data:
#             y_hat = sigmoid(net.forward(sample[:-1]))
#             if abs(y_hat - sample[-1]) < 0.5:
#                 count += 1
#             loss += crossEntropy(y_hat, sample[-1:])
#         print('epoch %d: loss: %.3f,  train acc: %.3f%%  test acc: %.3f %%'
#               % (epoch+1, loss, count2*100.0/train_data.shape[0], count*100.0/test_data.shape[0]))

#
#
#
