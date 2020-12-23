#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import torch
import torch.nn as nn
import torch.nn.modules as nm
import numpy as np
import numpy.random as random
import numpy.linalg as LA
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


P = np.load(r'D:\pycharm_project\DNE\processed_data\cora_P.npy')
labels = np.load(r'D:\pycharm_project\DNE\processed_data\cora_label.npy')
attributes = np.load(r'D:\pycharm_project\DNE\processed_data\cora_attributes.npy')

class BinActive(torch.autograd.Function):
    def forward(self, input):
        self.save_for_backward(input)
        size = input.size()
        input = input.sign()
        return input

    def backward(self, grad_output):
        input, = self.saved_tensors
        grad_input = grad_output.clone()
        grad_input[input.ge(1)] = 0
        grad_input[input.le(-1)] = 0
        return grad_input

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




## formulation norm2( P - B @ Z)**2 + beta * norm2(Z)**2 + lmda * [classifier]
def trans(x):
    # mx = np.mean(x, axis=0, keepdims=True)
    # x = x / mx
    # x[x > 1] = 1
    # x[x < 1] = 0
    # x[x < -1] = -1

    return np.sign(x)


def svm_test(B, label, train_num, C=1):
    # pca = PCA(n_components=32)
    # B = pca.fit_transform(B)
    # clf = make_pipeline(StandardScaler(),
    #                     svm.LinearSVC(random_state=0, tol=1e-5))
    clf = svm.SVC(kernel='poly', max_iter=2000, C=C)
    clf.fit(B[:train_num, :], np.argmax(label[:train_num, :], axis=1))
    predict = clf.predict(B[:train_num, :])
    print(classification_report(predict, np.argmax(label[:train_num], axis=1)))
    # print(predict)
    acc_cnt1 = np.sum(
        [1. if predict[i] == np.argmax(label[i]) else 0 for i in range(train_num)])
    predict = clf.predict(B[train_num:, :])
    # print("predict shape", predict.shape, np.argmax(label[:train_num], axis=1).shape)
    acc_cnt2 = np.sum(
        [1. if predict[i] == np.argmax(label[train_num + i]) else 0 for i in range(B.shape[0] - train_num)])
    print(classification_report(predict, np.argmax(label[train_num:], axis=1)))
    print("train acc : %.2f %%" % (acc_cnt1 / train_num * 100))
    print("test acc : %.2f %%" % (acc_cnt2 / (B.shape[0] - train_num) * 100))
    return


class Net(nn.Module):
    def __init__(self, input_num, hidden_num, output_num):
        super(Net, self).__init__()
        self.seq = nm.Sequential(
            nm.Linear(input_num, hidden_num),
            nm.ReLU(),
            nm.Linear(hidden_num, output_num)
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
        for X, y in data_iter(B[:train_num], label[:train_num], 128):
            # print(X)
            y_hat = net(X)
            # print(torch.argmax(y, dim=1).shape)
            l = loss(y_hat, torch.argmax(y, dim=1))
            optimizer.zero_grad()
            l.backward()
            optimizer.step()
        # net.eval()  # 评估模式, 这会关闭dropout
        # print(net(B[:train_num]))
        print(torch.argmax(net(B[:train_num]), dim=1) == torch.argmax(label[:train_num], dim=1).float())
        acc_sum = torch.sum(
            (torch.argmax(net(B[:train_num]), dim=1) == torch.argmax(label[:train_num], dim=1)).float()).item()
        acc_sum1 = torch.sum((torch.argmax(net(B[train_num:]), dim=1) == torch.argmax(label[train_num:],
                                                                                      dim=1)).float()).item()
        # net.train()
        print("epoch %d, train acc: %.2f %%  test acc: %.2f %%" % (
            epoch + 1, acc_sum / train_num * 100, acc_sum1 / (B.shape[0] - train_num) * 100))


def nn_test(B, label, train_num):
    B = torch.tensor(B, dtype=torch.float32)
    net = Net(B.shape[1], B.shape[1] * 4, label.shape[1])
    loss = nm.CrossEntropyLoss()
    optimizer = torch.optim.Adam(net.parameters(), lr=0.01)
    label = torch.tensor(label)
    fit(net, B, label, train_num, loss, optimizer)
    return


def train2(P, label, train_num, dim=128, beta=0.1, rau=0.5, gamma=1.0, k=5, r=2, C=1.0):
    # svd = TruncatedSVD(n_components=dim)
    # B = trans(svd.fit_transform(P[2]))
    Q = random.randn(P[0].shape[0], dim)
    B = np.sign(Q)
    Z = np.empty((k, P.shape[2], dim))
    PP = np.empty((k, P.shape[2], P.shape[2]))
    alpha = np.ones(k) / k
    for i in range(k):
        # print(P[i].shape)
        PP[i] = P[i].T @ P[i]
    for i in range(12):
        print("epoch ", i + 1)
        # step Z
        temp = np.zeros_like(B)
        for i in range(k):
            Z[i] = np.linalg.inv(PP[i] + beta * np.eye(P[i].shape[1])) @ (P[i].T @ B) * alpha[i]
            temp += P[i] @ Z[i]
        # step B
        # print("B")
        Q = Q - gamma * (B - temp)
        B = np.mean(Q) * np.sign(Q)
        # print(B)
        # a step
        # print("a")
        temp = np.zeros(k)
        for i in range(k):
            temp[i] = 1 / (LA.norm(P[i] @ Z[i] - B) ** 2 ** (1 / (r - 1)))
            # temp[i] = 1 / np.abs(np.sum(P[i]) - np.sum(B @ Z[i])) ** (1/(r - 1))
        for i in range(k):
            alpha[i] = temp[i] / np.sum(temp[i])
        print(LA.norm((P @ Z - B)) ** 2)

        svm_test(B, label, train_num)





def train3(P, label, train_num, dim=128, beta=0.1, rau=0.5, gamma=0.3, k=5, r=2, C=1.0):
    # svd = TruncatedSVD(n_components=dim)
    # B = trans(svd.fit_transform(P[2]))
    B = trans(random.randn(P[0].shape[0], dim))
    alpha = np.ones(k) / k
    Q = B.copy()
    lmda = np.random.randn(B.shape[0], B.shape[1])
    Z = np.empty((k, P.shape[2], dim))
    PP = np.empty((k, P.shape[2], P.shape[2]))
    for i in range(k):
        # print(P[i].shape)
        PP[i] = P[i].T @ P[i]
    for i in range(12):
        print("epoch ", i + 1)
        if i > 4:
            svm_test(B, label, train_num)
        # step Z
        for i in range(k):
            Z[i] = np.linalg.inv(PP[i] + beta * np.eye(P[i].shape[1])) @ (P[i].T @ B) * alpha[i]
        # step B
        # print("B")
        temp = np.zeros_like(B)
        temp2 = rau * (Q - lmda)
        for i in range(k):
            temp += alpha[i] * P[i] @ Z[i]
        B = (temp + temp2) / (rau + 1)
        Q = trans(B + lmda)
        lmda = lmda + B - Q
        # hd.lowD_draw(B, label, 10)
        B = Q
        # a step
        # print("a")
        temp = np.zeros(k)
        for i in range(k):
            temp[i] = 1 / (LA.norm((P[i] @ Z[i] - B) ** 2) ** (1 / (r - 1)))
            # temp[i] = 1 / np.abs(np.sum(P[i]) - np.sum(B @ Z[i])) ** (1/(r - 1))
        for i in range(k):
            alpha[i] = temp[i] / np.sum(temp[i])
        print(LA.norm(P @ Z - B) ** 2)

    # np.save(r'D:\pycharm_project\DNE\processed_data\B.npy', B)
    # svm_test(B, label, train_num)


train_ratio = 0.9
# print(P.shape)
train_num = int(P.shape[1] * train_ratio)
test_num = P.shape[1] - train_num
k = 5
# train2(P, labels, train_num, dim=128, beta=6.45, rau=0.0499997, gamma=0.04, k=k, r=3, C=1.0)
# temp = np.zeros_like(P[0])
# for i in range(k):
#     temp += P[i]
# pca = PCA(128)
# temp = pca.fit_transform(temp)
# svm_test(temp, labels, train_num)


train2(P, labels, train_num, dim=128, beta=6.45, rau=0.0499997, gamma=0.001, k=k, r=3, C=1.0)
# train1(P, labels, train_num, dim=128, beta=0.1, rau=0.01, gamma=0.03, k=k, r=3)
# {-1, 1}naive: train_num, dim=128, beta=0.65(极度敏感), rau=0.01(极度敏感), gamma=0.04, k=k, r=2)
# DNE classifier近似最优参数dim=128, beta=0.1, rau=0.01, gamma=0.03,
#


