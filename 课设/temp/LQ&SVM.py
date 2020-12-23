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
    mx = np.mean(x, axis=0, keepdims=True)
    # mx = np.mean(x)
    x = x / mx
    x[x > 1] = 1
    x[x < 1] = 0
    x[x < -1] = -1

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


def kernelized_pegasos(x, y, kernel, alpha=None, iterations=2000, lamd=1, b_lr=0.1):
    tot_num = x.shape[0]
    change_cnt = 0
    b = 0
    y = y.reshape(-1, 1)

    if alpha is None:
        alpha = np.zeros((tot_num, 1))
    # print((y * kernel(x[0], x)).shape)
    for i in range(iterations):
        it = random.randint(tot_num)
        # print(y[it])
        decision = np.sum(alpha * y * kernel(x[it], x).reshape(-1, 1) + b) * y[it] / lamd / (i + 1)

        # print(alpha * y * kernel(x[it], x).reshape(-1, 1))
        # print('-' * 50)
        # print(decision)
        if decision < 1:
            change_cnt += 1
            alpha[it] += 1
            # b = b + b_lr * y[it]
    alpha /= iterations * lamd
    # print(alpha)
    # print(b)
    print(change_cnt)
    return alpha, b



def k_test(X, y, weight, b, train_num):
    # y[y == -1] = 0

    weight = weight.reshape(-1, 1)
    y = y.reshape(-1, 1)
    train_B = X[:train_num, :]
    test_B = X[train_num:, :]
    train_label = y[:train_num]
    test_label = y[train_num:]
    acc_cnt = 0
    acc_cnt1 = 0
    # print(train_label.shape, weight.shape)
    weight *= train_label
    print(weight.shape)
    # print(weight)
    for i in range(train_B.shape[0]):
        # print(i)
        # for j in range(train_B.shape[0]):
        #     decision += weight[j] * RBF_kernel(train_B[i].reshape(1, -1), train_B[j])
        decision = np.sum(weight * RBF_kernel(train_B[i], train_B).reshape(-1, 1) + b)
        # print((weight * RBF_kernel(train_B[i], train_B).reshape(-1, 1)).shape)
        print(decision)
        if np.sign(decision) == train_label[i]:
            acc_cnt += 1
    for i in range(test_B.shape[0]):
        decision = np.sum(weight * RBF_kernel(test_B[i], train_B).reshape(-1, 1) + b)
        if np.sign(decision) == test_label[i]:
            acc_cnt1 += 1
    print("train acc : %.2f %%" % (acc_cnt / train_B.shape[0] * 100))
    print("test acc : %.2f %%" % (acc_cnt1 / test_B.shape[0] * 100))

def svm_test(B, label, train_num, C=1.0):
    # pca = PCA(n_components=32)
    # B = pca.fit_transform(B)
    # clf = make_pipeline(StandardScaler(),
    #                     svm.LinearSVC(random_state=0, tol=1e-5))
    clf = svm.SVC(max_iter=2000, C=C)
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












