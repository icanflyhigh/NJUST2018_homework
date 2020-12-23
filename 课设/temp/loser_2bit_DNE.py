#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import torch
import torch.nn as nn
import torch.nn.modules as nm
import numpy as np
import numpy.random as random
import scipy.sparse
from scipy.io import loadmat
import sklearn
from sklearn.decomposition import PCA
import torch_sparse as sparse
import handwritten as hd
import time

#TODO
#1.制作-1 0 1
#2.制作-2 -1 0 1 2
#

random.seed(0)
# 加载数据
file_path = 'D:/pycharm_project/datasets/mat/dblp-s.mat'
data = loadmat(file_path)

data_ratio = 1.0
train_ratio = 0.9

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

# 求关系矩阵
g1 = W
k = 2
for i in range(2, k, 1):
    g1 = g1 * g1
    W = W + g1

W = W / k

sm = scipy.sparse.csc_matrix(np.diag(np.sum(W, axis=0).A.reshape(-1) ** -.5))
NS = sm @ W @ sm
print("---初始化完成", '-' * 50)


def train1(NS, dim, train_num, label, tau, lmda):
    B = np.sign(np.random.randn(dim, NS.shape[0]))
    # B = np.random.randn(dim, NS.shape[0])
    for i in range(5):
        w = B[:, :train_num] @ label[:train_num]
        # print(" w mean : ", w.mean())
        w = np.sign(w)

        # w = B[:, :train_num] @ label[:train_num]
        w0 = w @ label[0:train_num].T
        for j in range(5):
            grad = - B @ NS
            grad[:, 0:train_num] = grad[:, :train_num] + lmda * w0
            B = B - tau * grad
            # print(" B mean : ", B.mean())
            B = np.sign(B)
    print("---train1训练完成", '-' * 50)
    return B, w


def low_bitlize(x, MF):
    shape = x.shape
    x = x.reshape(-1)
    idx = np.argsort(x)
    s1 = x[x > 0]
    s2 = x[x <= 0]
    sum1 = np.sum(s1 ** 2)
    sum2 = np.sum(s2 ** 2)
    num1 = s1.shape[0]
    num2 = s2.shape[0]
    F = sum1 / num1 + sum2 / num2
    s3 = 0
    low = -1
    high = x.shape[0]
    l = 0
    h = high -1
    while l < h:
        mid = int((l + h) / 2)
        if x[idx[mid]] < 0:
            l = mid + 1
        elif x[idx[mid]] > 0:
            h = mid - 1
        else:
            h = l = mid
    while h+1 < x.shape[0] and x[idx[h + 1]] == 0:
        h += 1
    high = h
    low = h
    while F < MF:
        if high > x.shape[0] or low < 0:
            break
        if s3 <= 0:
            high += 1
            temp = x[idx[high]]
            x[idx[high]] = 0
            s3 += temp
            num1 -= 1
            sum1 -= temp**2
        else:
            temp = x[idx[low]]
            x[idx[low]] = 0
            s3 += temp
            num2 -= 1
            sum2 -= temp**2
            low -= 1
        F = sum1 / num1 + sum2 / num2
    low_bitlize.MF = F
    a = x[idx[low]]
    b = x[idx[high]]
    x[idx[high:]] = 1
    x[idx[:low+1]] = -1
    x = x.reshape(shape)
    return x, a, b, F

def train2(NS, dim, train_num, label, tau, lmda, alpha):
    # pca = PCA(n_components=dim)
    # t1 = pca.fit_transform(NS)
    # B = np.sign(t1.T)
    # tf = 0
    # _, _, _, tf = low_bitlize(t1, tf)
    # B, _, _, _ = low_bitlize(t1, tf)
    B = np.sign(np.random.randn(dim, NS.shape[0]))
    # B = np.random.randn(dim, NS.shape[0])
    for i in range(5):
        print("iter: ", i)
        MF1 = 1e3
        MF2 = 40
        w = B[:, :train_num] @ label[:train_num]
        # w, a, b, MF1 = low_bitlize(w, MF1*alpha)
        w = np.sign(w)

        # w = B[:, :train_num] @ label[:train_num]
        # print("W shape", w.shape, w[w == 0].shape)
        w0 = w @ label[0:train_num].T
        # print("W shape", w.shape, w[w == 0].shape)
        # print("W:a %.2f,  b %.2f" % (a, b))
        a = 0
        b = 0
        for j in range(10):
            grad = - B @ NS
            grad[:, 0:train_num] = grad[:, :train_num] + lmda * w0
            B = B - tau * grad


            # print(B.shape)
            B, a, b, MF2 = low_bitlize(B, MF2*alpha)
        print('\n', a, b)
        print(B[B < a].shape)
        print(B[B > b].shape)
        print(B[B < a].shape[0] + B[B > b].shape[0] - B.shape[0] * B.shape[1])

        # print("B shape", B.shape, B[B == 0].shape)
        # print("B:a %.2f,  b %.2f" % (a, b))
    print("---train2训练完成", '-' * 50)
    return B, w



def test(B, weight):
    train_B = B[:, :train_num]
    test_B = B[:, train_num:]
    train_label = label[:train_num]
    test_label = label[train_num:]
    # print("data size: ", W.shape[0])
    # print("label size: ", label.shape[1])
    # print("train size: ", train_B.shape)
    # print("test size: ", test_B.shape)
    predict = test_B.T @ weight
    # print(predict.shape)
    acc_cnt = np.sum([1. if np.argmax(predict[i]) == np.argmax(test_label[i]) else 0 for i in range(test_B.shape[1])])
    print("acc : %.2f %%" % (acc_cnt / test_B.shape[1] * 100))



train_num = np.int(W.shape[0] * train_ratio)
dim = 128
tau = 16
lmda = 0.3
alpha = 1
start = time.time()
# B, weight = train1(NS, dim, train_num, label.reshape(-1, len(label_set)), tau, lmda)
# test(B, weight)
B, weight = train2(NS, dim, train_num, label.reshape(-1, len(label_set)), tau, lmda, alpha)
print(time.time() - start)
test(B, weight)
# 分出训练和测试部分
# 在训练时认为测试部分没有带标签的向量

