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


file_path = r'D:\pycharm_project\datasets/mat/cora.mat'
data = loadmat(file_path)
attributes = data['attributes'].A
labels = data['labels'].reshape(-1).astype(int)
network = data['network']
# file_path = 'D:/pycharm_project/datasets/mat/dblp-s.mat'
# data = loadmat(file_path)
# print(data)
# network, labels = load_dblp(0.9)
tot_num = network.shape[0]
network += sparse.eye(tot_num)
D = sparse.csc_matrix(np.diag(np.sum(network, axis=0).A.reshape(-1) ** -.5))
S = (D @ network @ D)

# attributes = sparse.eye(network.shape[0])
# network, labels = load_dblp(0.9)


# print(type(P[0]))
labels -= 1
label_set = set(labels)
label_num = len(label_set)
labels = hd.one_hot(labels, label_num)
train_ratio = 0.9
train_num = int(tot_num * train_ratio)
test_num = tot_num - train_num

temp = S
k = 5
P = np.empty((k, attributes.shape[0], attributes.shape[1]))
P[0] = S @ attributes
start = time.time()
for i in range(1, k, 1):
    # temp = sparse.csr_matrix(temp @ S)
    # P.append(sparse.csr_matrix(temp @ attributes))
    temp = temp @ S
    P[i] = temp @ attributes
print(time.time() - start)


## formulation norm2( P - B @ Z)**2 + beta * norm2(Z)**2 + lmda * [classifier]
def trans(x):
    # idx = x > 0
    # x[idx] = 1
    # idx = x <= 0
    # x[idx] = -1
    # idx = x < -1.5
    # x[idx] = -2
    # idx = x > 1.5
    # x[idx] = 2
    mx = np.mean(x, axis=0, keepdims=True)
    # mx = np.mean(x)
    x = x / mx
    x[x > 1] = 1
    x[x < -1] = -1
    x[x < 1] = 0
    return x


def test(B, weight):
    train_B = B[:train_num, :]
    test_B = B[train_num:, :]
    train_label = labels[:train_num]
    test_label = labels[train_num:]
    predict = test_B @ weight.T
    acc_cnt = np.sum([1. if np.argmax(predict[i]) == np.argmax(test_label[i]) else 0 for i in range(test_B.shape[1])])
    print(classification_report(np.argmax(predict, axis=1), np.argmax(test_label, axis=1)))
    predict = train_B @ weight.T
    acc_cnt1 = np.sum(
        [1. if np.argmax(predict[i]) == np.argmax(train_label[i]) else 0 for i in range(train_B.shape[1])])
    # print(classification_report(np.argmax(predict, axis=1), np.argmax(train_label, axis=1)))
    print("train acc : %.2f %%" % (acc_cnt1 / train_B.shape[1] * 100))
    print("test acc : %.2f %%" % (acc_cnt / test_B.shape[1] * 100))


def train1(P, label, train_num, dim=128, beta=0.1, rau=0.5, gamma=0.3, k=5, r=2):
    # svd = TruncatedSVD(n_components=dim)
    # B = trans(svd.fit_transform(P[2]))
    B = trans(random.randn(P[0].shape[0], dim))
    alpha = np.ones(k) / k
    Q = B.copy()
    lmda = np.random.randn(B.shape[0], B.shape[1])
    A1 = label
    A2 = (label == 0)
    nz1 = np.sum(A1, axis=1, keepdims=True)
    nz2 = np.sum(A2, axis=1, keepdims=True)
    lab = - nz2 * A1 + nz1 * A2
    for i in range(15):
        print(i)
        # step w
        w = -trans(lab[:train_num, :].T @ B[:train_num, :])

        w0 = lab[:train_num, :] @ w
        # step Z
        Z = []
        for i in range(k):
            Z.append((np.linalg.inv(alpha[i] * B.T @ B + beta * np.eye(B.shape[1])) * alpha[i]) @ B.T @ P[i])
        # step B
        print("B")
        print(Z[0].shape)
        temp = rau * np.eye(Z[0].shape[0])
        temp2 = rau * (Q - lmda)
        for i in range(k):
            temp += 2 * alpha[i] * Z[i] @ Z[i].T
            temp2 += 2 * alpha[i] * P[i] @ Z[i].T
        B = temp2 @ np.linalg.inv(temp)
        B[:train_num, :] -= gamma * w0
        Q = trans(B + lmda)
        lmda = lmda + B - Q
        B = Q.copy()
        # a step
        print("a")
        temp = np.zeros(k)
        for i in range(k):
            temp[i] = 1 / np.sum((P[i] - B @ Z[i]) ** 2) ** (1 / (r - 1))
            # temp[i] = 1 / np.abs(np.sum(P[i]) - np.sum(B @ Z[i])) ** (1/(r - 1))
        for i in range(k):
            alpha[i] = temp[i] / np.sum(temp[i])
        test(B, w)


def svm_test(B, label, train_num, C=1.0):
    # pca = PCA(n_components=32)
    # B = pca.fit_transform(B)
    # clf = make_pipeline(StandardScaler(),
    #                     svm.LinearSVC(random_state=0, tol=1e-5))
    clf = svm.LinearSVC(max_iter=2000, C=C)
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


def train2(P, label, train_num, dim=128, beta=0.1, rau=0.5, gamma=0.3, k=5, r=2, C=1.0):
    # svd = TruncatedSVD(n_components=dim)
    # B = trans(svd.fit_transform(P[2]))
    B = trans(random.randn(P[0].shape[0], dim))
    alpha = np.ones(k) / k
    Q = B.copy()
    lmda = np.random.randn(B.shape[0], B.shape[1])
    Z = np.empty((k, dim, attributes.shape[1]))
    for i in range(10):
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
    svm_test(B, label, train_num, C=1.0)




train_ratio = 0.9
train_num = int(P.shape[1] * train_ratio)
test_num = P.shape[1] - train_num
k = 5
train2(P, labels, train_num, dim=128, beta=0.2505, rau=0.04999975, gamma=0.04, k=k, r=3, C=1.0)
# train1(P, labels, train_num, dim=128, beta=0.1, rau=0.01, gamma=0.03, k=k, r=3)
# {-1, 1}naive: train_num, dim=128, beta=0.65(极度敏感), rau=0.01(极度敏感), gamma=0.04, k=k, r=2)
# DNE classifier近似最优参数dim=128, beta=0.1, rau=0.01, gamma=0.03,
