#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import numpy as np
import numpy.random as random
from numpy import linalg as LA
import scipy.sparse
from scipy.io import loadmat
from sklearn.metrics import classification_report
import sklearn
from sklearn import svm
from sklearn.decomposition import PCA
import torch_sparse as sparse
import handwritten as hd
import time
import pandas
random.seed(0)
import matplotlib.pyplot as plt




def load_ex6data2():
    file_path = r"D:\pycharm_project\datasets\demo\SVM\ex6data2.mat"
    data = loadmat(file_path)
    X = data['X']
    y = data['y'].astype(int)
    idx = np.array(range(X.shape[0]))
    random.shuffle(idx)
    X = X[idx]
    # plt.scatter(X[:, 0], X[:, 1])
    # # plt.plot(np.arange(half), np.arange(half))
    # plt.show()
    y = y[idx]
    return X, y

def gen_data(num=20):

    X = np.ones((num, 2))
    y = np.ones(num, dtype=int)
    half = int(num / 2.0)
    for i in range(half):
        X[i, 0] = i
        X[i, 1] = abs(random.randn())

    for i in range(half, num, 1):
        X[i, 0] = i - half
        X[i, 1] = - abs(random.randn())
        y[i] = -1
    idx = np.arange(num)
    random.shuffle(idx)
    X = X[idx]
    y = y[idx]

    # plt.scatter(X[:, 0], X[:, 1])
    # plt.plot(np.arange(half), np.arange(half))
    # plt.show()


    return X, y


def RBF_kernel(x1, x2, delta=0.00001):
    """
    :param x1:
    :param x2:
    :param delta:delta 越大分界线越平滑，越小分界线越尖锐
    :return:
    """
    #  求norm2用norm函数比sum(**2)高效大概13倍左右
    # print((x1 - x2).shape)
    # return np.exp(-LA.norm(x1 - x2, axis=1) ** 2 / delta)

    # print(x1, x2)
    # print(np.exp(-LA.norm(x1 - x2) ** 2 / delta))
    # print(",", LA.norm(x1 - x2, axis=1) ** 2)
    return np.exp(-LA.norm(x1 - x2, axis=1) ** 2 / delta).reshape(-1, 1)


def kernelized_pegasos(x, y, kernel, alpha=None, iterations=2000, lamd=1, b_lr=0.1):
    tot_num = x.shape[0]
    change_cnt = 0
    b = 0
    y = y.reshape(-1, 1)

    if alpha is None:
        alpha = np.zeros((tot_num, 1))
    for i in range(iterations):
        it = random.randint(tot_num)
        decision = np.sum(alpha * y * kernel(x[it], x).reshape(-1, 1) + b) * y[it] / lamd / (i + 1)
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
    # print(weight)
    for i in range(train_B.shape[0]):
        # print(i)
        # for j in range(train_B.shape[0]):
        #     decision += weight[j] * RBF_kernel(train_B[i].reshape(1, -1), train_B[j])
        decision = np.sum(weight * RBF_kernel(train_B[i], train_B).reshape(-1, 1) + b)
        # print((weight * RBF_kernel(train_B[i], train_B).reshape(-1, 1)).shape)
        if np.sign(decision) == train_label[i]:
            acc_cnt += 1
    for i in range(test_B.shape[0]):
        decision = np.sum(weight * RBF_kernel(test_B[i], train_B).reshape(-1, 1) + b)
        if np.sign(decision) == test_label[i]:
            acc_cnt1 += 1
    print("train acc : %.2f %%" % (acc_cnt / train_B.shape[0] * 100))
    print("test acc : %.2f %%" % (acc_cnt1 / test_B.shape[0] * 100))




X, y = load_ex6data2()
print(np.std(X))
# mean = np.mean(X, axis=0)
# X -= mean
# std = np.std(X, axis=0)
# X /= std
y[y == 0] = -1
train_num = int(0.7 * X.shape[0])
# w = my_svm(X[:train_num, :], y[:train_num], batch_size=1, iter_num=1000, lmda=1e-3)
w, b = kernelized_pegasos(X[:train_num], y[:train_num], RBF_kernel)
k_test(X, y, w, b, train_num)

# clf = svm.SVC(kernel=RBF_kernel)
# clf.fit(X[:train_num], np.ravel(y[:train_num]))
# print(classification_report(clf.predict(X[train_num:]), np.ravel(y[train_num:])))
# print(classification_report(clf.predict(X[:train_num]), np.ravel(y[:train_num])))
# test(X, y, w, train_num)

