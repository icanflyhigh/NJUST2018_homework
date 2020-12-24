import re
import spacy
from torchtext.data import Field, BucketIterator
from torchtext.datasets import Multi30k
import torchtext.data as ttd
import jieba
import torch
# def load_dataset(batch_size):
#     spacy_de = spacy.load('de')
#     spacy_en = spacy.load('en')
#     url = re.compile('(<url>.*</url>)')
#
#     def tokenize_de(text):
#         return [tok.text for tok in spacy_de.tokenizer(url.sub('@URL@', text))]
#
#     def tokenize_en(text):
#         return [tok.text for tok in spacy_en.tokenizer(url.sub('@URL@', text))]
#
#     DE = Field(tokenize=tokenize_de, include_lengths=True,
#                init_token='<sos>', eos_token='<eos>')
#     EN = Field(tokenize=tokenize_en, include_lengths=True,
#                init_token='<sos>', eos_token='<eos>')
#     train, val, test = Multi30k.splits(exts=('.de', '.en'), fields=(DE, EN))
#     DE.build_vocab(train.src, min_freq=2)
#     EN.build_vocab(train.trg, max_size=10000)
#     train_iter, val_iter, test_iter = BucketIterator.splits(
#             (train, val, test), batch_size=batch_size, repeat=False)
#     return train_iter, val_iter, test_iter, DE, EN

def tokenizer(text):
    return list(jieba.cut(text, cut_all=False))
def load_dataset(batch_size):


    ASK = Field(tokenize=tokenizer, include_lengths=True,
                eos_token='<eos>', init_token='<sos>')

    ANS = Field(tokenize=tokenizer, include_lengths=True,
                eos_token='<eos>', init_token='<sos>')
    file_path = r"D:\pycharm_project\datasets\NLP\cn_chat/"
    train, val, test = ttd.TabularDataset.splits(
        path=file_path,
        train='weibo_mini_train.csv', test='weibo_mini_test.csv', validation='weibo_mini_val.csv',
        format='csv', fields=[('Ask', ASK), ('Ans', ANS)])

    ASK.build_vocab(train.Ask, min_freq=1)
    ANS.build_vocab(train.Ans, max_size=20000)

    torch.save(ASK, file_path + 'ASK')
    torch.save(ANS, file_path + 'ANS')
    # train_iter, val_iter, test_iter = BucketIterator.splits(
    #         (train, val, test), batch_size=batch_size, repeat=False)
    train_iter, val_iter, test_iter = BucketIterator.splits(
            (train, val, test), batch_size=batch_size, repeat=False, sort=False)
    return train_iter, val_iter, test_iter, ASK, ANS



