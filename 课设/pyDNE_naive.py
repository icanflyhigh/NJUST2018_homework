#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import torch
import torch.nn as nn
import torch.nn.modules as nm
import numpy as np
import scipy.sparse
from scipy.io import loadmat
import sklearn
import torch_sparse as sparse
import handwritten as hd


# 加载数据
file_path = 'D:/pycharm_project/datasets/mat/dblp-s.mat'
data = loadmat(file_path)


data_ratio = 0.5
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
print("---初始化完成", '-'*50)


def train1(NS, dim, train_num, label, tau, lmda):
    B = np.sign(np.random.randn(dim, NS.shape[0]))
    for i in range(3):
        w = np.sign(B[:, :train_num] @ label[:train_num])
        w0 = w @ label[0:train_num].T
        for j in range(3):
            grad = - B @ NS
            grad[:, 0:train_num] = grad[:, :train_num] + lmda * w0
            B = np.sign(B - tau * grad)

    return B, w

train_num = np.int(W.shape[0] * train_ratio)
dim = 128
tau = 0.5
lmda = 10
B, weight = train1(NS, dim, train_num, label.reshape(-1, len(label_set)), tau, lmda)
print("---训练完成", '-'*50)
# 分出训练和测试部分
# 在训练时认为测试部分没有带标签的向量
train_B = B[:, :train_num]
test_B = B[:, train_num:]
train_label = label[:train_num]
test_label = label[train_num:]
print("data size: ", W.shape[0])
print("label size: ", label.shape[1])
print("train size: ", train_B.shape)
print("test size: ", test_B.shape)
predict = test_B.T @ weight
acc_cnt = np.sum([1. if np.argmax(predict[i] == test_label[i]) else 0 for i in range(test_B.shape[1])])
print("acc : %.2f %%" % (acc_cnt / test_B.shape[0] * 100))
