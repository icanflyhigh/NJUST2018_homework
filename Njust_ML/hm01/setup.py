#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import numpy as np
import numpy.random as random
import scipy.sparse as sparse
from scipy.io import loadmat, savemat
import sklearn
import pandas as pd
import handwritten as hd
import time
import itertools
import collections
import sklearn.decomposition as decomposition
from sklearn.manifold import TSNE
import matplotlib as mpl
import matplotlib.pyplot as plt
import xlwt
import os
import platform
import sys
sys.path.append(".")
import naive_maozhen as nm

if platform.system() != 'Windows':
    raise Exception("请使用Windows,否则可能无法创建文件:(  同时无法保证文件能够运行")
if sys.version_info[0] != 3 or sys.version_info[1] != 6:
    print("*" * 50)
    print("python版本不是3.6 无法保证文件能够正确运行")
if not os.path.exists("processed"):
    os.makedirs("processed")
if not os.path.exists("processed/train"):
    os.makedirs("processed/train")
if not os.path.exists("processed/test"):
    os.makedirs("processed/test")
print('*'*50)
print("创建文件夹完成")
print("*"*50)
print("处理数据中...")
# print('*'*50)
print("请忽略那个警告:)    (如果有的话)")
print('*'*50)
file_path = ""
train_file = file_path + "train/"
test_file = file_path + "test/"
save_path = r"processed/"
strain_path = save_path + "train/"
stest_path = save_path + "test/"
vocab_path = "stop_words_zh.txt"
class_list = ['电脑', '法律', '教育', '经济', '体育', '政治']
row_skip = ["'<text>'", "'</text>'"]

vocab_data = pd.read_table(vocab_path, header=None).values.squeeze()
class_num = len(class_list)
char2idx = dict()
idx2char = ["<UNK>"]
log_probability = True
train_data = np.empty(class_num, dtype=object)
test_data = np.empty(class_num, dtype=object)
nm.load_write_data(is_load=False, is_save=True)
print("初始化完成！")
print("*"*50)

















