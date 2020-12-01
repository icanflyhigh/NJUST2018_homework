#!/usr/bin/env python
# -*- coding: UTF-8 -*-
from abc import abstractmethod
from abc import ABCMeta
import numpy as np
import numpy.random as random
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
import scipy.sparse as sparse
import sklearn
import sklearn.decomposition
from sklearn.decomposition import PCA

# 首先我需要写一个Linear
EPS = 1e-10


def regular(data):
    mean = np.array(data.mean(axis=1, keepdims=True))
    ret = data - mean
    std = np.array(ret.std(axis=1, keepdims=True))
    ret /= np.sqrt(std + EPS)
    return ret


'''
    接下来是一些IO之类的小工具
'''


def data_iter(data_X, data_y, batch_size):
    #
    if isinstance(data_X, list):
        length = len(data_X)
        indices = list(range(length))
        random.shuffle(indices)
        for i in range(0, length, batch_size):
            index = np.array(indices[i: min(i + batch_size, length)], dtype=int)  # 最后一次可能不足一个batch
            yield np.array([data_X[j] for j in index]), np.array([data_y[j] for j in index])
    elif isinstance(data_X, np.ndarray):
        length = data_X.shape[0]
        indices = list(range(length))
        random.shuffle(indices)
        for i in range(0, length, batch_size):
            index = np.array(indices[i: min(i + batch_size, length)], dtype=int)  # 最后一次可能不足一个batch
            yield np.array(data_X[index]), np.array(data_y[index])
    else:  # 假装他是稀疏矩阵 用csr 的原因是batch是列的，稀疏是行上稀疏
        length = data_X.shape[0]
        indices = list(range(length))
        random.shuffle(indices)
        for i in range(0, length, batch_size):
            index = np.array(indices[i: min(i + batch_size, length)], dtype=int)  # 最后一次可能不足一个batch
            yield sparse.csr_matrix(data_X[index]), data_y[index]


# k折交叉验证
def k_fold(data, K=10):
    """
    k折交叉验证
    :param data: 数据 只支持np array
    :param K: k折
    :return: 训练数据, 测试数据

    话说验证集数据呢？ 再加一个么
    """
    length = len(data)
    one_fold_size = int(length / K)
    random.shuffle(data)
    for i in range(0, length, one_fold_size):
        train1 = data[0:i]
        label = data[i:min(i + one_fold_size, length)]
        train2 = data[min(i + one_fold_size, length):]
        if train1 is None:
            yield train2, label
        elif train2 is None:
            yield train1, label
        else:
            yield np.concatenate((train1, train2), axis=0), label


def word2idx(label):
    """
    将文本标签转化为数字标签
    :param label: 标签vector
    :return: 一个字典将文本标签映射为数字标签
    """
    label = label.reshape(-1)
    word_set = list(set(label))
    return dict({word_set[i]: i for i in range(len(word_set))})


def one_hot(data, k):
    """
    将一维向量转化为onehot向量
    :param data:
    :param k: k个标签
    :return: int型的矩阵
    """
    temp = np.eye(k, dtype=int)
    data = data.reshape(-1).astype(int)
    return temp[data]


def lowD_draw(data, label, size):
    """
    简单的高维数据可视化
    :param data: 输入数据
    :param label: 输入标签,必须为one_hot vector
    :param size: 输出维度（其实只能为2）
    :return:
    """
    global cnames
    namelsit = list(cnames)
    # mpl.rcParams['font.serif'] = ['KaiTi']
    # mpl.rcParams['font.sans-serif'] = ['KaiTi']
    plt.rcParams['font.sans-serif'] = ['SimHei']
    plt.rcParams['axes.unicode_minus'] = False
    pca = PCA(n_components=2)
    x = pca.fit_transform(data)
    # tsne = TSNE(n_components=2, init='pca')

    # x = tsne.fit_transform(x)
    num = 0
    print(x.shape)
    plt.xlim(-size, size)
    plt.ylim(-size, size)
    label = np.argmax(label, axis=1)
    print(label)
    for i in range(data.shape[0]):
        plt.scatter(x[i, 0], x[i, 1], s=5, label=label[i], c=namelsit[label[i]])
    plt.legend(loc="upper left")
    plt.show()


# -------------------------------------------------------

#  linear regression

# def MSELoss(y_hat, y):
#     return ((y_hat - y)**2).sum() / y_hat.shape[0]


def Linear(w, b, X):
    return np.dot(X, w) + b


# ----------------------------------------------------------


# logistic regression _______________________________________

def sigmoid(z):
    return 1.0 / (1.0 + np.exp(-z))


def crossEntropy(y_hat, y):
    return -y * np.log(y_hat) - (1 - y) * np.log(1 - y_hat)


# -----------------------------------------------------------


'''
    优化器部分
'''


class Optim:
    def __init__(self, lr=0.01):
        self.lr = lr

    @abstractmethod
    def step(self, grad, delta):
        return


class SGD(Optim):
    def __init__(self, lr):
        super(SGD, self).__init__(lr)

    def step(self, body, grad):
        return body - self.lr * grad


class Adam(Optim):
    def __init__(self, lr, size, beta=0.99):
        super(Adam, self).__init__(lr)
        self.V = np.zeros(size)
        self.m = np.zeros(size)
        self.theta = np.zeros(size)

    def step(self, body, grad):
        return


# ________________________________________________________________


'''
    loss 部分
'''


class Loss:
    def __init__(self):
        self.grad = np.array([])
        self.loss = np.array([])
        return


class MSELoss(Loss):
    def __init__(self):
        super(MSELoss, self).__init__()

    def cal(self, y_hat, y):
        self.loss = (y_hat - y) / y_hat.shape[0]
        self.grad = y_hat - y
        return


# 妈个鸡，交叉熵+softmax分开算 太弱智了而且数值不稳定
# 约定   交叉熵算了这两边的loss
class CrossEntropyLoss(Loss):
    def __init__(self, isSoftmax=True):
        super(CrossEntropyLoss, self).__init__()
        self.isSoftmax = isSoftmax

    def cal(self, y_hat, y):
        self.loss = -1 * np.sum((y * np.log(y_hat + 1e-30)), axis=1, keepdims=True)
        if self.isSoftmax:
            self.grad = y_hat - y
        else:
            self.grad = y / y_hat
        return


def loss_softmax(y_hat, y):
    z = np.exp(y_hat)
    z /= np.sum(z)
    return z - y


def loss_MSE_grad(y_hat, y):
    return (y_hat - y) / y_hat.shape[0]


# ________________________________________________________________


'''
    运算单元部分    
'''


class Unit:
    __metaclass__ = ABCMeta

    def __init__(self):
        return

    # 定义前向方法
    @abstractmethod
    def forward(self, input):
        return

    # 定义反向向方法
    @abstractmethod
    def backward(self, grad):
        return


class LinearUnit(Unit):
    def __init__(self, input_num, output_num, lr, optimizer=SGD):
        super(LinearUnit, self).__init__()
        self.w = np.random.normal(0, 0.01, (input_num, output_num)) * np.sqrt(6 / (input_num + output_num))
        self.optimizer = optimizer(lr)
        # self.w = np.zeros((input_num, output_num))
        self.b = np.random.randn(1, output_num)
        self.input = np.array([])

    def forward(self, input):
        self.input = input
        # print(self.w)
        return input @ self.w + self.b

    def backward(self, grad):  # i  *  o
        # print(grad.mean())
        db = grad.mean(axis=0, keepdims=True)
        dw = (np.transpose(self.input) @ grad) / self.input.shape[0]
        din = np.dot(grad, np.transpose(self.w))
        self.b = self.optimizer.step(self.b, db)
        self.w = self.optimizer.step(self.w, dw)
        # print("\n\n")
        # print(dw)
        # print(din)
        return din

    def parameters(self):
        return self.w, self.b


class SigmoidUnit:
    def __init__(self, isCrossEntropy=True):
        self.output = np.array([])
        self.isCrossEntropy = isCrossEntropy
        return

    def forward(self, input):
        self.output = 1.0 / (1.0 + np.exp(-1 * input))
        return self.output  # 值传递

    def backward(self, grad):
        if self.isCrossEntropy:
            return grad
        else:
            return grad * self.output * (1 - self.output)


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
        # print(self.input * grad)
        return self.input * grad


class SoftMaxUnit(Unit):
    def __init__(self, isCrossEntropy=True):
        super(SoftMaxUnit, self).__init__()
        self.isCrossEntropy = isCrossEntropy
        self.y = 0.0

    def forward(self, input):
        input = input.astype(np.float64)
        input = input - input.max(axis=1, keepdims=True)  # 小细节防止数值溢出
        input = np.exp(input)
        input = input / input.sum(axis=1, keepdims=True)
        self.y = input
        return input

    def backward(self, grad):
        if self.isCrossEntropy:
            return grad
        else:
            temp = self.y * grad
            return temp - self.y * temp.sum(axis=1, keepdims=True)


#  接下来 该是卷积神经网络登场了_____________________________________________
#  不知道能不能够实现这个~
#  该单元只支持2d卷积
class ConvUnit(Unit):
    def __init__(self, lr, channels=1, kernel_size=3, stride=1, padding=0):
        super(ConvUnit, self).__init__(lr)
        self.kernel_size = kernel_size
        self.kernel = np.random.randn(channels, kernel_size, kernel_size) * EPS
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
                                  pt[num][r - margin:r + margin + 1, c - margin:c + margin + 1]).reshape(self.channels,
                                                                                                         -1), axis=1)

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
                if r + margin >= row or r - margin < 0:
                    continue
                cg = 0
                for c in range(0, col, self.stride):
                    if c + margin >= col or c - margin < 0:
                        continue
                    for chan in range(self.channels):
                        dk[chan] = dk[chan] + \
                                   self.p_input[num][r - margin:r + margin + 1, c - margin:c + margin + 1] * \
                                   grad[num][rg][cg][chan]

                        din[num][r - margin:r + margin + 1, c - margin:c + margin + 1] = \
                            din[num][r - margin:r + margin + 1, c - margin:c + margin + 1] + \
                            grad[num][rg][cg][chan] * self.kernel[chan]

                    cg += 1
                rg += 1
        self.kernel -= self.lr / din.shape[0] * dk
        # 处理din
        din = din.reshape((din.shape[0], din.shape[1], din.shape[2], 1))
        din = np.concatenate([din] * self.in_size[3], 2)
        return din[:, margin:din.shape[0] - margin, margin:din.shape[1] - margin, :]


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
        margin = np.int(self.size / 2)
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
                        ret[num][rr][rc][chan] = np.max(input[num][r - margin:r + margin + 1,
                                                        c - margin:c + margin + 1, chan:chan + 1])
                        idx = np.argmax(input[num][r - margin:r + margin + 1, c - margin:c + margin + 1, chan:chan + 1])

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

cnames = {
    'aliceblue': '#F0F8FF',
    'antiquewhite': '#FAEBD7',
    'aqua': '#00FFFF',
    'aquamarine': '#7FFFD4',
    'azure': '#F0FFFF',
    'beige': '#F5F5DC',
    'bisque': '#FFE4C4',
    'black': '#000000',
    'blanchedalmond': '#FFEBCD',
    'blue': '#0000FF',
    'blueviolet': '#8A2BE2',
    'brown': '#A52A2A',
    'burlywood': '#DEB887',
    'cadetblue': '#5F9EA0',
    'chartreuse': '#7FFF00',
    'chocolate': '#D2691E',
    'coral': '#FF7F50',
    'cornflowerblue': '#6495ED',
    'cornsilk': '#FFF8DC',
    'crimson': '#DC143C',
    'cyan': '#00FFFF',
    'darkblue': '#00008B',
    'darkcyan': '#008B8B',
    'darkgoldenrod': '#B8860B',
    'darkgray': '#A9A9A9',
    'darkgreen': '#006400',
    'darkkhaki': '#BDB76B',
    'darkmagenta': '#8B008B',
    'darkolivegreen': '#556B2F',
    'darkorange': '#FF8C00',
    'darkorchid': '#9932CC',
    'darkred': '#8B0000',
    'darksalmon': '#E9967A',
    'darkseagreen': '#8FBC8F',
    'darkslateblue': '#483D8B',
    'darkslategray': '#2F4F4F',
    'darkturquoise': '#00CED1',
    'darkviolet': '#9400D3',
    'deeppink': '#FF1493',
    'deepskyblue': '#00BFFF',
    'dimgray': '#696969',
    'dodgerblue': '#1E90FF',
    'firebrick': '#B22222',
    'floralwhite': '#FFFAF0',
    'forestgreen': '#228B22',
    'fuchsia': '#FF00FF',
    'gainsboro': '#DCDCDC',
    'ghostwhite': '#F8F8FF',
    'gold': '#FFD700',
    'goldenrod': '#DAA520',
    'gray': '#808080',
    'green': '#008000',
    'greenyellow': '#ADFF2F',
    'honeydew': '#F0FFF0',
    'hotpink': '#FF69B4',
    'indianred': '#CD5C5C',
    'indigo': '#4B0082',
    'ivory': '#FFFFF0',
    'khaki': '#F0E68C',
    'lavender': '#E6E6FA',
    'lavenderblush': '#FFF0F5',
    'lawngreen': '#7CFC00',
    'lemonchiffon': '#FFFACD',
    'lightblue': '#ADD8E6',
    'lightcoral': '#F08080',
    'lightcyan': '#E0FFFF',
    'lightgoldenrodyellow': '#FAFAD2',
    'lightgreen': '#90EE90',
    'lightgray': '#D3D3D3',
    'lightpink': '#FFB6C1',
    'lightsalmon': '#FFA07A',
    'lightseagreen': '#20B2AA',
    'lightskyblue': '#87CEFA',
    'lightslategray': '#778899',
    'lightsteelblue': '#B0C4DE',
    'lightyellow': '#FFFFE0',
    'lime': '#00FF00',
    'limegreen': '#32CD32',
    'linen': '#FAF0E6',
    'magenta': '#FF00FF',
    'maroon': '#800000',
    'mediumaquamarine': '#66CDAA',
    'mediumblue': '#0000CD',
    'mediumorchid': '#BA55D3',
    'mediumpurple': '#9370DB',
    'mediumseagreen': '#3CB371',
    'mediumslateblue': '#7B68EE',
    'mediumspringgreen': '#00FA9A',
    'mediumturquoise': '#48D1CC',
    'mediumvioletred': '#C71585',
    'midnightblue': '#191970',
    'mintcream': '#F5FFFA',
    'mistyrose': '#FFE4E1',
    'moccasin': '#FFE4B5',
    'navajowhite': '#FFDEAD',
    'navy': '#000080',
    'oldlace': '#FDF5E6',
    'olive': '#808000',
    'olivedrab': '#6B8E23',
    'orange': '#FFA500',
    'orangered': '#FF4500',
    'orchid': '#DA70D6',
    'palegoldenrod': '#EEE8AA',
    'palegreen': '#98FB98',
    'paleturquoise': '#AFEEEE',
    'palevioletred': '#DB7093',
    'papayawhip': '#FFEFD5',
    'peachpuff': '#FFDAB9',
    'peru': '#CD853F',
    'pink': '#FFC0CB',
    'plum': '#DDA0DD',
    'powderblue': '#B0E0E6',
    'purple': '#800080',
    'red': '#FF0000',
    'rosybrown': '#BC8F8F',
    'royalblue': '#4169E1',
    'saddlebrown': '#8B4513',
    'salmon': '#FA8072',
    'sandybrown': '#FAA460',
    'seagreen': '#2E8B57',
    'seashell': '#FFF5EE',
    'sienna': '#A0522D',
    'silver': '#C0C0C0',
    'skyblue': '#87CEEB',
    'slateblue': '#6A5ACD',
    'slategray': '#708090',
    'snow': '#FFFAFA',
    'springgreen': '#00FF7F',
    'steelblue': '#4682B4',
    'tan': '#D2B48C',
    'teal': '#008080',
    'thistle': '#D8BFD8',
    'tomato': '#FF6347',
    'turquoise': '#40E0D0',
    'violet': '#EE82EE',
    'wheat': '#F5DEB3',
    'white': '#FFFFFF',
    'whitesmoke': '#F5F5F5',
    'yellow': '#FFFF00',
    'yellowgreen': '#9ACD32'}
#
#
