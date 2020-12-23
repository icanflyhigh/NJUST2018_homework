#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import torch
import torch.nn as nn
import torch.nn.modules as nm
import numpy as np
import numpy.random as random
import scipy.sparse as sparse
from scipy.io import loadmat

import sklearn
from sklearn import svm
from sklearn.decomposition import TruncatedSVD
from sklearn.metrics import classification_report
# import torch_sparse as sparse
import handwritten as hd
import time
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA

random.seed(0)

def Binarize(tensor, quant_mode='det'):
    if quant_mode == 'det':
        return tensor.sign()
    else:
        return tensor.add_(1).div_(2).add_(torch.rand(tensor.size()).add(-0.5)).clamp_(0, 1).round().mul_(2).add_(-1)



class BinActive(torch.autograd.Function):

    @staticmethod
    def forward(ctx, input):
        size = input.shape[0]
        # print(input.shape)
        alpha = input.mean(dim=1).view(-1, 1)
        # print(alpha)
        ctx.save_for_backward(input, torch.tensor(size), alpha)
        input = input.sign()
        # print(input.shape)
        return input * alpha

    @staticmethod
    def backward(ctx, grad_output):
        input, size, alpha, = ctx.saved_tensors
        grad_input = grad_output.clone()
        grad_input[input.ge(1)] = 0
        grad_input[input.le(-1)] = 0
        return grad_input * alpha + 1 / size


class BinLinear(nn.Module):  # change the name of BinConv2d
    def __init__(self, input_num, output_num):
        super(BinLinear, self).__init__()
        self.layer_type = 'BinLinear'
        self.alpha = 0.0
        self.Linear = nn.Linear(input_num, output_num)

    def forward(self, x):
        x = self.Linear(x)
        x = BinActive.apply(x)
        return x


class BinarizeLinear(nn.Linear):

    def __init__(self, *kargs, **kwargs):
        super(BinarizeLinear, self).__init__(*kargs, **kwargs)

    def forward(self, input):
        input.data = trans(input.data)
        if not hasattr(self.weight, 'org'):
            self.weight.org = self.weight.data.clone()
        # print(self.weight.org)
        # print(self.weight.data)
        self.weight.data = trans(self.weight.org)

        out = nn.functional.linear(input, self.weight)
        if self.bias is not None:
            self.bias.org = self.bias.data.clone()
            out += self.bias.view(1, -1).expand_as(out)

        return out





def load_dblp(data_ratio=0.9):
    file_path = './datasets/mat/dblp-s.mat'
    data = loadmat(file_path)
    W = data['W'].astype(int)
    label = data['gnd'].astype(int)
    tot_len = W.shape[0]
    # shuffle数组
    idx = np.array(range(tot_len))
    np.random.shuffle(idx)
    W = W[idx, :]
    W = W[:, idx]
    label = label[idx]

    data_len = np.int(W.shape[0] * data_ratio)
    W = W[:data_len, :]
    W = W[:, :data_len]
    label = label[:data_len].reshape(-1)

    # # 去除孤立点
    degree = np.sum(W, axis=0).astype(int)
    ZD = (degree.A != 0).reshape(-1)
    W = W[ZD]
    W = W[:, ZD]
    label = label[ZD]

    # 转化为one_hot 向量
    label_set = set(label)
    label = hd.one_hot(label, len(label_set))
    return W, label



P = np.load(r'D:\pycharm_project\DNE\processed_data\cora_P.npy')
labels = np.load(r'D:\pycharm_project\DNE\processed_data\cora_label.npy')
attributes = np.load(r'D:\pycharm_project\DNE\processed_data\cora_attributes.npy')

## formulation norm2( P - B @ Z)**2 + beta * norm2(Z)**2 + lmda * [classifier]
def trans(x):

    # mx = np.mean(x, axis=0, keepdims=True)
    if torch.is_tensor(x):
        ret =torch.zeros_like(x)
    else:
        ret = np.zeros_like(x)
    # x = x / mx
    ret[x > 0.5] += 1
    ret[x > 1.5] += 1
    ret[x > 3] += 2
    ret[x > 6] += 4
    # ret[x > 12] += 8
    ret[x < -0.5] -= 1
    ret[x < -1.5] -= 1
    ret[x < -3] -= 2
    ret[x < -6] -= 4
    # ret[x < -12] -= 8
    # print(ret)
    print(ret.mean())
    return ret




class Net(nn.Module):
    def __init__(self, input_num, hidden_num, output_num):
        super(Net, self).__init__()
        self.seq = nm.Sequential(
            # BinarizeLinear(input_num, hidden_num),
            # nm.ReLU(),
            # BinarizeLinear(hidden_num, output_num),
            # #______________________________________
            # nn.BatchNorm1d(input_num),
            # nn.Linear(input_num, hidden_num),
            # nn.ReLU(),
            # nn.BatchNorm1d(hidden_num),
            # nn.Linear(hidden_num, output_num),
            # #______________________________________
            nm.BatchNorm1d(input_num),
            BinarizeLinear(input_num, hidden_num),
            nm.BatchNorm1d(hidden_num),
            BinarizeLinear(hidden_num, output_num),
            # BinarizeLinear(hidden_num, output_num),
        )

    def forward(self, input):
        return self.seq(input)


def data_iter(data_X, data_y, batch_size):
    length = data_X.shape[0]
    indices = list(range(length))
    random.shuffle(indices)
    for i in range(0, length, batch_size):
        index = np.array(indices[i: min(i + batch_size, length)], dtype=int)  # 最后一次可能不足一个batch
        yield torch.tensor(data_X[index], dtype=torch.float32), torch.tensor(data_y[index], dtype=torch.float32)


def fit(net, B, label, train_num, loss, optimizer, epoch_num=12):
    for epoch in range(epoch_num):
        for X, y in data_iter(B[:train_num], label[:train_num], 64):
            # print(X)
            y_hat = net(X)
            # print(torch.argmax(y, dim=1).shape)
            l = loss(y_hat, torch.argmax(y, dim=1))
            optimizer.zero_grad()
            l.backward()
            optimizer.step()
        # net.eval()  # 评估模式, 这会关闭dropout
        # print(net(B[:train_num]))
        # print(torch.argmax(net(B[:train_num]), dim=1) == torch.argmax(label[:train_num], dim=1).float())
        acc_sum = torch.sum(
            (torch.argmax(net(B[:train_num]), dim=1) == torch.argmax(label[:train_num], dim=1)).float()).item()
        acc_sum1 = torch.sum((torch.argmax(net(B[train_num:]), dim=1) == torch.argmax(label[train_num:],
                                                                                      dim=1)).float()).item()
        # net.train()
        print("epoch %d, train acc: %.2f %%  test acc: %.2f %%" % (
            epoch + 1, acc_sum / train_num * 100, acc_sum1 / (B.shape[0] - train_num) * 100))


def nn_test(B, label, train_num):
    B = torch.tensor(B, dtype=torch.float32)
    net = Net(B.shape[1], B.shape[1]*4, label.shape[1])
    loss = nm.CrossEntropyLoss()
    optimizer = torch.optim.Adam(net.parameters(), lr=0.003)  # binlinear
    # optimizer = torch.optim.Adam(net.parameters(), lr=0.001) # linear
    label = torch.tensor(label)
    fit(net, B, label, train_num, loss, optimizer, 20)
    return

# TODO
# 1、修改train2使得GD
# 2、使用bnn获得好效果!一定拜托了
# 事实是效果可能不会太好
def train2(P, label, train_num, dim=128, beta=0.1, rau=0.5, gamma=0.3, k=5, r=2, C=1.0):
    # svd = TruncatedSVD(n_components=dim)
    # B = trans(svd.fit_transform(P[2]))
    B = trans(random.randn(P[0].shape[0], dim))
    alpha = np.ones(k) / k
    Q = B.copy()
    lmda = np.random.randn(B.shape[0], B.shape[1])
    Z = np.empty((k, dim, attributes.shape[1]))
    for i in range(5):
        print("epoch ", i + 1)
        # step Z
        for i in range(k):
            Z[i] = np.linalg.inv((alpha[i] * B.T @ B + beta * np.eye(B.shape[1])) * alpha[i]) @ (B.T @ P[i])
        # step B
        # print("B")
        temp = rau * np.eye(Z[0].shape[0])
        temp2 = rau * (Q - lmda)
        for i in range(k):
            temp += 2 * alpha[i] * Z[i] @ Z[i].T
            temp2 += 2 * alpha[i] * P[i] @ Z[i].T
        B = temp2 @ np.linalg.inv(temp)
        Q = trans(B + lmda)
        lmda = lmda + B - Q
        # hd.lowD_draw(B, label, 10)
        B = Q
        # a step
        # print("a")
        temp = np.zeros(k)
        for i in range(k):
            temp[i] = 1 / (np.sum((P[i] - B @ Z[i]) ** 2) ** (1 / (r - 1)))
            # temp[i] = 1 / np.abs(np.sum(P[i]) - np.sum(B @ Z[i])) ** (1/(r - 1))
        for i in range(k):
            alpha[i] = temp[i] / np.sum(temp[i])
        print(np.sum((P - B @ Z) ** 2))
    nn_test(B, label, train_num)


train_ratio = 0.9
train_num = int(P.shape[1] * train_ratio)
test_num = P.shape[1] - train_num
k = 5
train2(P, labels, train_num, dim=128, beta=0.2505, rau=0.04999975, gamma=0.04, k=k, r=3, C=1.0)
# train1(P, labels, train_num, dim=128, beta=0.1, rau=0.01, gamma=0.03, k=k, r=3)
# {-1, 1}naive: train_num, dim=128, beta=0.65(极度敏感), rau=0.01(极度敏感), gamma=0.04, k=k, r=2)
# DNE classifier近似最优参数dim=128, beta=0.1, rau=0.01, gamma=0.03,





# file_path = r'D:\pycharm_project\datasets/mat/cora.mat'
# data = loadmat(file_path)
# attributes = data['attributes'].A
# labels = data['labels'].reshape(-1).astype(int)
# network = data['network']
# # file_path = 'D:/pycharm_project/datasets/mat/dblp-s.mat'
# # data = loadmat(file_path)
# # print(data)
# # network, labels = load_dblp(0.9)
# tot_num = network.shape[0]
# network += sparse.eye(tot_num)
# D = sparse.csc_matrix(np.diag(np.sum(network, axis=0).A.reshape(-1) ** -.5))
# S = (D @ network @ D)
#
# # attributes = sparse.eye(network.shape[0])
# # network, labels = load_dblp(0.9)
#
#
# # print(type(P[0]))
# labels -= 1
# label_set = set(labels)
# label_num = len(label_set)
# labels = hd.one_hot(labels, label_num)
# train_ratio = 0.9
# train_num = int(tot_num * train_ratio)
# test_num = tot_num - train_num
#
# temp = S
# k = 5
# P = np.empty((k, attributes.shape[0], attributes.shape[1]))
# P[0] = S @ attributes
# start = time.time()
# for i in range(1, k, 1):
#     # temp = sparse.csr_matrix(temp @ S)
#     # P.append(sparse.csr_matrix(temp @ attributes))
#     temp = temp @ S
#     P[i] = temp @ attributes



