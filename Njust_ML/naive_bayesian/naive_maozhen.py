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

file_path = ""
train_file = file_path + "train/"
test_file = file_path + "test/"
save_path = r"processed/"
strain_path = save_path + "train/"
stest_path = save_path + "test/"
vocab_path = "stop_words_zh.txt"
class_list = ['电脑', '法律', '教育', '经济', '体育', '政治']
row_skip = ["'<text>'", "'</text>'"]
del_row_str = [
    '<text>', '</text>'
]
replace_str = [
    r'([【][ ][日][ ][期][ ][】]).*([【][ ][标][ ][题][ ][】])',
    r'([【][ ][作][ ][者][ ][】]).*([【][ ][正][ ][文][ ][】])',
    r'[人][民][日][报]',
    r'[新][华][社]',
    '\u3000',
    r'[（].*?[）]',
    r'[(].*?[)]',
    '[—“》《”、：:，‘’％？…·＊！。』『．″∶／＋－���￣,＆）●......%*＜＞"\\]\\[＝, ＇＼]',
    '[〉〈▲×‰ⅠⅡ①②③④⑤⑥⑦□▲○〇\\)﹐［］]',
    '[─]',
    r'([0-9a-zA-Z])+',
    r'[０-９ａ-ｚＡ-ＺⅢ]',

]
vocab_data = pd.read_table(vocab_path, header=None).values.squeeze()
class_num = len(class_list)
char2idx = dict()
idx2char = ["<UNK>"]
log_probability = True
train_data = np.empty(class_num, dtype=object)
test_data = np.empty(class_num, dtype=object)

def data_process(data, row, del_row_str, replace_str):
    row = data.columns[row]
    for s in del_row_str:
        data = data[~data[row].str.contains(s)]
    for s in replace_str:
        data[row] = data[row].str.replace(s, ' ', regex=True)

    return data


def class_process(file_path, class_name):
    data = pd.read_table(file_path)
    data = data_process(data, 0, del_row_str, replace_str)
    data['<text>'] = data['<text>'].str.strip()
    seq1 = data['<text>'].str.split(' ')
    seq1 = seq1.values
    temp = []
    # print("in")
    for i in range(len(seq1)):
        seq1[i] = [s for s in seq1[i] if len(s) != 0 and s not in vocab_data]
        temp.append(seq1[i])
        # print("1")
    # corpus = np.array(temp, dtype=object)

    return temp


def write_softmax(train_data, test_data, doc_len, vocab_len, class_num, char2idx, target):
    train = np.zeros((np.sum(doc_len), vocab_len))
    test = np.zeros((np.sum([len(s) for s in test_data]), vocab_len))
    cnt = 0
    cnt1 = 0
    if target == 'TF':
        for i in range(class_num):
            for s in train_data[i]:
                Count(train[cnt], s, char2idx)
                cnt += 1
        for i in range(class_num):
            for s in test_data[i]:
                Count(test[cnt1], s, char2idx)
                cnt1 += 1

    else:
        target = 'bl'
        for i in range(class_num):
            for s in train_data[i]:
                Count2(train[cnt], s, char2idx)
                cnt += 1
        for i in range(class_num):
            for s in test_data[i]:
                Count2(test[cnt1], s, char2idx)
                cnt1 += 1
    np.save(save_path + target + ".npy", train)
    np.save(save_path + target + "_test.npy", test)



def load_write_data(is_load=True, is_save=True):
    cnt = 1
    global char2idx, idx2char, train_data, test_data
    for i in range(len(class_list)):
        if is_load:
            train_data[i] = np.load(strain_path + class_list[i] + ".npy", allow_pickle=True)
            test_data[i] = np.load(stest_path + class_list[i] + ".npy", allow_pickle=True)

            # print(train_data[i])
        else:
            train_data[i] = class_process(train_file + class_list[i] + ".txt", class_list[i])
            test_data[i] = class_process(test_file + class_list[i] + ".txt", class_list[i])
            char2idx["<UNK>"] = 0
            for c in itertools.chain.from_iterable(train_data[i]):
                if c not in char2idx:
                    char2idx[c] = cnt
                    idx2char.append(c)
                    cnt += 1
            if is_save:
                np.save(strain_path + class_list[i] + ".npy", train_data[i])
                np.save(stest_path + class_list[i] + ".npy", test_data[i])
    if is_load:
        idx2char = np.load(save_path + "idx2char.npy", allow_pickle=True)
        char2idx = np.load(save_path + "char2idx.npy", allow_pickle=True).item()
    else:
        np.save(save_path + "idx2char.npy", idx2char)
        np.save(save_path + "char2idx.npy", char2idx)
        vocab_len = len(idx2char)
        doc_len = np.array([len(d) for d in train_data])
        write_softmax(train_data, test_data, doc_len, vocab_len, class_num, char2idx, "TF")
        write_softmax(train_data, test_data, doc_len, vocab_len, class_num, char2idx, "bl")



def Count(fre, data, char2idx):
    for c in data:
        if c not in char2idx:
            fre[0] += 1
        else:
            fre[char2idx[c]] += 1


def Count2(fre, data, char2idx):
    c_set = list(set(data))
    for c in c_set:
        if c not in char2idx:
            c = '<UNK>'
        fre[char2idx[c]] += 1


def cal_doc_prob(doc_num, log_probability=True):
    tot = np.sum(doc_num)
    if log_probability:
        return np.log(doc_num / tot)
    else:
        return doc_num / tot


def cal_vocab_prob(fre, vocab_len, smoothing=None, log_probability=True):
    vocab_prob = np.array(fre)
    if smoothing is None and not log_probability:
        return vocab_prob / np.sum(vocab_prob)
    elif smoothing == 'laplacian':  # 拉普拉斯平滑
        # print(np.sum(vocab_prob))
        if log_probability:
            return np.log((vocab_prob + 1) / (np.sum(vocab_prob) + vocab_len))
            # return np.log((vocab_prob + 1) )
        else:
            return vocab_prob / (np.sum(vocab_prob) + vocab_len)
    else:
        print("cal_vocab_prob ERROR")
        return None


def cal_vocab_prob2(fre, doc_len, smoothing=None, log_probability=False):
    vocab_prob = np.array(fre, dtype=float)
    if smoothing is None:
        vocab_prob = vocab_prob / doc_len
        return vocab_prob
    elif smoothing == 'laplacian':  # 拉普拉斯平滑
        try:
            print(vocab_prob.shape)  # 分母加2的平滑不靠谱
            vocab_prob = (vocab_prob + 1e-4) / (doc_len + 2e-4)
            # print(min(vocab_prob), max(vocab_prob), doc_len+class_num)
            if log_probability:
                return np.log(vocab_prob), np.log(1 - vocab_prob)
            else:
                return vocab_prob
        except RuntimeWarning:
            print("!")
    else:
        print("cal_vocab_prob ERROR")
        return None


def predict(sentences, doc_prob, vocab_prob, class_num, char2idx, log_prob=True):
    # print(sentences.shape[0], doc_len)
    prob = np.zeros((sentences.shape[0], class_num))
    if log_prob:
        prob += doc_prob
        # print(vocab_prob)
        for i in range(len(sentences)):
            # print(s)
            # print(vocab_prob[:, [char2idx[c] for c in s]].shape)
            prob[i] += np.sum(vocab_prob[:, [char2idx[c] for c in sentences[i]]], axis=1)
    else:
        prob += 1
        prob *= doc_prob
        for s in sentences:
            prob += np.sum(vocab_prob[:, [char2idx[c] for c in s]], axis=1)
    # print(prob)
    # print(np.argmax(prob, axis=1).shape)
    return np.argmax(prob, axis=1)


def predict2(sentence, doc_prob, vocab_prob, class_num, char2idx, log_prob=True):

    c_set = set(sentence)
    if log_prob:
        prob = np.array(doc_prob) + np.sum(vocab_prob[:, 1], axis=1)
        for j in range(len(idx2char)):
            if idx2char[j] in c_set:
                prob += vocab_prob[:, 0, j] - vocab_prob[:, 1, j]
            # else:  # 如果不乘1-p训练效果显著提升，我认为原因是体育训练集样本数目太多导致当词处于test文本的词之外时，概率特别高
            #     prob += vocab_prob[:, 1, j]
    else:
        prob = np.array(doc_prob)
        for j in range(len(idx2char)):
            if idx2char[j] in c_set:
                prob *= vocab_prob[:, j]
            else:
                prob *= 1 - vocab_prob[:, j]
    # print(prob)
    # print(cnt)
    return np.argmax(prob)


def lowD_draw(data, doc_len, class_num, class_list):
    mpl.rcParams['font.serif'] = ['KaiTi']
    # mpl.rcParams['font.sans-serif'] = ['KaiTi']
    pca = decomposition.PCA(n_components=1024)
    x = pca.fit_transform(data)
    tsne = TSNE(n_components=2, init='pca')

    x = tsne.fit_transform(x)
    num = 0
    print(x.shape)
    plt.xlim(-50, 50)
    plt.ylim(-50, 50)
    for i in range(class_num):
        plt.scatter(x[num:num + doc_len[i], 0], x[num:num + doc_len[i], 1], s=5, label=class_list[i])
        num += doc_len[i]
    plt.legend(loc="upper left")
    plt.show()


def write_csv(fre):
    plt.rcParams['font.sans-serif'] = ['SimHei']
    plt.rcParams['axes.unicode_minus'] = False
    ans = xlwt.Workbook(encoding='utf-8')
    sheet = ans.add_sheet('sheet1', cell_overwrite_ok=True)
    for i in range(class_num):
        sheet.write(0, i + 1, class_list[i])
        sheet.write(1, i + 1, int(doc_len[i]))
    for row in range(vocab_len):
        sheet.write(row + 2, 0, idx2char[row])
        for i in range(class_num):
            sheet.write(row + 2, i + 1, int(fre[i][row]))

    ans.save('test2.xls')


def train1(class_num, char2idx, idx2char, train_data, test_data):
    vocab_len = len(idx2char)
    doc_len = np.array([len(d) for d in train_data])
    freCnt = np.zeros((class_num, vocab_len))

    for i in range(class_num):
        for s in train_data[i]:
            Count(freCnt[i], s, char2idx)

    doc_prob = cal_doc_prob(doc_len)
    vocab_prob = np.array(
        [cal_vocab_prob(freCnt[i], vocab_len, smoothing='laplacian') for i in range(class_num)])
    # print(np.exp(vocab_prob))
    # write_csv(freCnt)
    print('训练集 :')
    for i in range(class_num):
        cnt_list = np.zeros(class_num)
        acc_cnt = 0.0
        tot = len(train_data[i])
        pred = predict(train_data[i], doc_prob, vocab_prob, class_num, char2idx, log_prob=True)
        for j in range(len(pred)):
            if pred[j] == i:
                acc_cnt += 1
            cnt_list[pred[j]] += 1
        print(class_list[i] + "acc: %.2f%%" % (acc_cnt / tot * 100))
        for j in range(class_num):
            print("     pred" + class_list[j] + " : %.2f %%" % (cnt_list[j] / tot * 100))
    print('-' * 50)
    print('测试集 :')
    for i in range(class_num):
        cnt_list = np.zeros(class_num)
        acc_cnt = 0.0
        tot = len(test_data[i])
        for s in test_data[i]:
            for j in range(len(s)):
                if s[j] not in char2idx:
                    s[j] = '<UNK>'
        pred = predict(test_data[i], doc_prob, vocab_prob, class_num, char2idx, log_prob=True)
        for j in range(len(pred)):
            if pred[j] == i:
                acc_cnt += 1
            cnt_list[pred[j]] += 1
        # pred = predict(np.array([["电脑"]], dtype=object), doc_prob, vocab_prob, class_num, char2idx, log_prob=True)
        # print(class_list[pred[0]])
        print(class_list[i] + "acc: %.2f%%" % (acc_cnt / tot * 100))
        for j in range(class_num):
            print("     pred" + class_list[j] + " : %.2f %%" % (cnt_list[j] / tot * 100))


def train2(class_num, char2idx, idx2char, train_data, test_data):
    vocab_len = len(idx2char)
    doc_len = np.array([len(d) for d in train_data])

    freCnt = np.zeros((class_num, vocab_len))
    for i in range(class_num):
        for j in range(len(train_data[i])):
            Count2(freCnt[i], train_data[i][j], char2idx)
    # write_csv(freCnt)
    doc_prob = cal_doc_prob(doc_len, log_probability=log_probability)
    vocab_prob = np.array(
        [cal_vocab_prob2(freCnt[i], smoothing='laplacian', log_probability=log_probability, doc_len=doc_len[i]) for i in
         range(class_num)])
    tot = len(test_data[0])
    # print(np.exp(vocab_prob))
    acc_cnt = 0.0
    # temp_test = ["经济", "电脑", "体育"]
    # print(class_list[predict2(temp_test, doc_prob, vocab_prob, class_num, char2idx, log_prob=True)])
    for i in range(class_num):
        print(class_list[i])
        print(vocab_prob[i, 1].mean())

    print("训练集：")
    for i in range(class_num):
        cnt_list = np.zeros(class_num)
        acc_cnt = 0.0
        tot = len(train_data[i])
        for s in train_data[i]:
            pred = predict2(s, doc_prob, vocab_prob, class_num, char2idx, log_prob=log_probability)
            if pred == i:
                acc_cnt += 1
            cnt_list[pred] += 1
        print(class_list[i] + "acc: %.2f%%" % (acc_cnt / tot * 100))
        for j in range(class_num):
            print("     pred" + class_list[j] + " : %.2f %%" % (cnt_list[j] / tot * 100))
    print("测试集：")
    for i in range(class_num):
        cnt_list = np.zeros(class_num)
        acc_cnt = 0.0
        tot = len(test_data[i])
        for s in test_data[i]:
            pred = predict2(s, doc_prob, vocab_prob, class_num, char2idx, log_prob=log_probability)
            if pred == i:
                acc_cnt += 1
            cnt_list[pred] += 1
        print(class_list[i] + "acc: %.2f%%" % (acc_cnt / tot * 100))
        for j in range(class_num):
            print("     pred" + class_list[j] + " : %.2f %%" % (cnt_list[j] / tot * 100))






def load_tf():
    return np.load(save_path + "TF.npy"), np.load(save_path + "TF_test.npy")


def train3(target='TF'):
    plt.ion()
    fig, ax = plt.subplots(1, 2)
    plt.subplots_adjust(wspace=0, hspace=0.5)
    plt.title(target)
    fig.dpi = 768
    ax[0].set_title("loss")
    ax[1].set_title("acc_rate")
    ax[0].set_xlabel('epoch')
    ax[1].set_xlabel("epoch")

    if target == 'TF':
        train, test = load_tf()
    else:
        target = 'BL'
        train, test = load_bl()
    # mean = train.mean()
    # std = train.std()
    # train = (train - mean) / std
    # test = (test - mean) / std

    train_label = np.zeros((train.shape[0], class_num), dtype=int)
    test_label = np.zeros((test.shape[0], class_num), dtype=int)
    # pca = decomposition.PCA(n_components=1000)
    # data = np.vstack((train, test))
    # pca.fit(data)
    # train = pca.transform(train)
    # test = pca.transform(test)
    cnt = 0
    cnt1 = 0
    for i in range(class_num):
        train_label[cnt:cnt + doc_len[i], i] = 1
        test_label[cnt1:cnt1 + len(test_data[i]), i] = 1
        cnt += doc_len[i]
        cnt1 += len(test_data[i])
    # 离散化
    train = sparse.csr_matrix(train)
    test = sparse.csr_matrix(test)
    # print(test_label)
    # idx = np.array(range(train.shape[0]))
    # random.shuffle(idx)
    # train = train[idx]
    # train_label = train_label[idx]
    lr = 3
    net = Net(train.shape[1], class_num, lr)
    epochs = 20
    batch_size = 2048  # GD
    flag_l = False
    for epoch in range(epochs):
        test_tot = len(test_label)
        test_acc_cnt = 0
        train_tot = len(train_label)
        train_acc_cnt = 0
        y_point = 0
        start = time.time()
        for X, y in hd.data_iter(train, train_label, batch_size):

            y_hat = net.forward(X)
            # print(type(X))
            for i in range(y_hat.shape[0]):
                if np.argmax(y_hat[i]) == np.argmax(y[i]):
                    train_acc_cnt += 1
                net.loss.cal(y_hat, y)
                y_point += np.sum(net.loss.loss)
        ax[1].scatter(epoch, train_acc_cnt / train_tot, c='r', marker='.', label='train')
        ax[0].scatter(epoch, y_point / train_tot, c='r', marker='.', label='train')
        #  plt.pause(0.1)
        # print("forward1")
        y_point = 0
        for X, y in hd.data_iter(test, test_label, batch_size):
            y_hat = net.forward(X)
            for i in range(y_hat.shape[0]):
                if np.argmax(y_hat[i]) == np.argmax(y[i]):
                    test_acc_cnt += 1
                net.loss.cal(y_hat, y)
                y_point += np.sum(net.loss.loss)
            ax[1].scatter(epoch, test_acc_cnt / test_tot, c='black', marker='.', label='test')
            ax[0].scatter(epoch, y_point / test_tot, c='black', marker='.', label='test')
            plt.pause(0.1)
            if not flag_l:
                ax[0].legend(loc=2)
                ax[1].legend(loc=2)
            flag_l = True

        for X, y in hd.data_iter(train, train_label, batch_size):
            y_hat = net.forward(X)
            net.backward(y)
        # print(net.seq[0].w.max())
        print("epoch %d: train_acc: %.2f%%   test_acc: %.2f%%    time:%.3f" % (
            epoch + 1, train_acc_cnt / train_tot * 100, test_acc_cnt / test_tot * 100, time.time() - start))
    plt.ioff()
    plt.show()
    plt.savefig(target+".png")



def load_bl():
    return np.load(save_path + "bl.npy"), np.load(save_path + "bl_test.npy")




class Net:
    def __init__(self, input_num, output_num, lr):
        self.seq = [
            hd.LinearUnit(input_num, output_num, lr),
            # hd.ReLUUnit(),
            # hd.LinearUnit(1024, output_num, lr),
            hd.SoftMaxUnit(isCrossEntropy=True),
        ]
        self.loss = hd.CrossEntropyLoss(isSoftmax=True)
        self.y_hat = np.array([])

    def forward(self, input):
        ret = input
        for unit in self.seq:
            ret = unit.forward(ret)
        self.y_hat = ret
        return ret

    def backward(self, label):
        self.loss.cal(self.y_hat, label)
        grad = self.loss.grad
        for unit in reversed(self.seq):
            grad = unit.backward(grad)

if __name__ == '__main__':
    load_write_data(is_load=True, is_save=False)
    vocab_len = len(idx2char)
    doc_len = np.array([len(d) for d in train_data])
    train1(class_num, char2idx, idx2char, train_data, test_data)  # 多项式朴素贝叶斯
    # train1(class_num, char2idx, idx2char, train_data, test_data)  # 多变量伯努利朴素贝叶斯
    # train3("TF")  # 基于TF的SoftMax
    # train3("bl")  # 基于BOOL的SoftMax
    # train4("TF")  # 基于TF的SoftMax(没有画图)
    # train4("bl")  # 基于BOOL的SoftMax(没有画图)










