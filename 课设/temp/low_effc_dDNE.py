#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import torch
import torch.nn as nn
import torch.nn.modules as nm
import numpy as np
import numpy.random as random
import scipy.sparse
import scipy as sp
import scipy.linalg
from scipy.io import loadmat
import sklearn
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


def low_bitlize(x, max_F=2.0):
    if not hasattr(low_bitlize, 'max_F'):
        low_bitlize.max_F = max_F
    idx1 = x > 0
    idx2 = x < 0
    s1 = x[idx1]
    s2 = x[idx2]
    sum1 = np.sum(s1 ** 2)
    sum2 = np.sum(s2 ** 2)
    num1 = s1.shape[0]
    num2 = s2.shape[0]
    F = sum1 / num1 + sum2 / num2
    s3 = 0

    tot_len = x.shape[0] * x.shape[1]
    tot_len1 = tot_len*0.2
    book = np.array(range(tot_len))
    tot_len *= 0.5
    sb1 = book[idx1.reshape(-1)]
    sb2 = book[idx2.reshape(-1)]
    iter = 0
    while F < low_bitlize.max_F:
        if iter % 1000 == 0 or iter < 100:
            print(iter)
        iter += 1
        if iter > tot_len or sb2.shape[0] < tot_len1 or sb1.shape[0] < tot_len1:
            break
        if s3 > 0:
            idx3 = np.argmax(x[idx2])
            temp = np.max(x[idx2])
            idx2.reshape(-1)[sb2[idx3]] = False
            sb2 = np.delete(sb2, idx3)
            sum2 -= temp ** 2
            num2 -= 1
            s3 += temp
            # print(idx3)
        else:
            idx3 = np.argmin(x[idx1])
            temp = np.min(x[idx1])
            idx1.reshape(-1)[sb1[idx3]] = False
            sb1 = np.delete(sb1, idx3)
            sum1 -= temp ** 2
            num1 -= 1
            s3 += temp
            # print(idx3)
            # print(idx1)
        F = sum1 / num1 + sum2 / num2
    low_bitlize.max_F = F
    a = np.max(x[idx2])
    b = np.min(x[idx1])
    x[:, :] = 0
    x[idx2] = -1
    x[idx1] = 1


    return x, a, b

def train2(NS, dim, train_num, label, tau, lmda):
    _, B = sp.linalg.eig(NS)
    B = np.sign(B)
    # B = np.sign(np.random.randn(dim, NS.shape[0]))
    # B = np.random.randn(dim, NS.shape[0])
    for i in range(5):
        print("iter: ", i)
        w, a, b = low_bitlize(B[:, :train_num] @ label[:train_num], 1e4)

        # w = B[:, :train_num] @ label[:train_num]
        # print("W shape", w.shape, w[w == 0].shape)
        w0 = w @ label[0:train_num].T
        # print("W shape", w.shape, w[w == 0].shape)
        print("W:a %.2f,  b %.2f" % (a, b))
        for j in range(5):
            grad = - B @ NS
            grad[:, 0:train_num] = grad[:, :train_num] + lmda * w0
            print("J"*60)
            B, a, b = low_bitlize(B - tau * grad, 40)
        print("B shape", B.shape, B[B == 0].shape)
        print("B:a %.2f,  b %.2f" % (a, b))
    print("---train2训练完成", '-' * 50)
    return B, w



def test(B, weight):
    train_B = B[:, :train_num]
    test_B = B[:, train_num:]
    train_label = label[:train_num]
    test_label = label[train_num:]
    print("data size: ", W.shape[0])
    print("label size: ", label.shape[1])
    print("train size: ", train_B.shape)
    print("test size: ", test_B.shape)
    predict = test_B.T @ weight
    print(predict.shape)
    acc_cnt = np.sum([1. if np.argmax(predict[i]) == np.argmax(test_label[i]) else 0 for i in range(test_B.shape[1])])
    print("acc : %.2f %%" % (acc_cnt / test_B.shape[1] * 100))



train_num = np.int(W.shape[0] * train_ratio)
dim = 64
tau = 8.5
lmda = 0.25
start = time.time()
# B, weight = train1(NS, dim, train_num, label.reshape(-1, len(label_set)), tau, lmda)
# test(B, weight)
B, weight = train2(NS, dim, train_num, label.reshape(-1, len(label_set)), tau, lmda)
test(B, weight)
# 分出训练和测试部分
# 在训练时认为测试部分没有带标签的向量

