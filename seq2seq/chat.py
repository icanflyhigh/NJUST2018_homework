import os
import math
import argparse
import torch
from torch import optim
from torch.autograd import Variable
from torch.nn.utils import clip_grad_norm_
from torch.nn import functional as F
from model import Encoder, Decoder, Seq2Seq
from torchtext.data import Dataset, Example, Batch
from utils import load_dataset
import math
import torch
import random
from torch import nn
from torch.autograd import Variable
import torch.nn.functional as F
import jieba

def tokenizer(text):
    return [s for s in jieba.cut(text, cut_all=False)]

def parse_arguments():
    p = argparse.ArgumentParser(description='Hyperparams')
    p.add_argument('-epochs', type=int, default=20,
                   help='number of epochs for train')
    p.add_argument('-batch_size', type=int, default=64,
                   help='number of epochs for train')
    p.add_argument('-lr', type=float, default=0.0001,
                   help='initial learning rate')
    p.add_argument('-grad_clip', type=float, default=10.0,
                   help='in case of gradient explosion')
    return p.parse_args()



def main():
    args = parse_arguments()
    hidden_size = 512
    embed_size = 256
    assert torch.cuda.is_available()

    print("[!] preparing dataset...")
    file_path = r"D:\pycharm_project\datasets\NLP\cn_chat/"
    ASK = torch.load(file_path+'ASK')
    ANS = torch.load(file_path+'ANS')
    ask_size, ans_size = len(ASK.vocab), len(ANS.vocab)

    encoder = Encoder(ask_size, embed_size, hidden_size,
                      n_layers=2, dropout=0.5)
    decoder = Decoder(embed_size, hidden_size, ans_size,
                      n_layers=1, dropout=0.5)
    seq2seq = Seq2Seq(encoder, decoder).cuda()
    seq2seq.load_state_dict(torch.load(r'./.save/seq2seq_20.pt'))
    seq2seq.eval()
    youAsk = ""
    while youAsk != 'end':
        youAsk = input("你说：")
        # youAsk = "好大的宇宙飞船"
        if youAsk == '':
            youAsk = '然后呢'
        youAsk = ASK.numericalize(([tokenizer(youAsk)], 1))[0].cuda().long()
        # print(type(example))
        output = seq2seq.inference(youAsk, max_len=20, EOS=ASK.vocab.stoi["<eos>"], SOS=ASK.vocab.stoi["<sos>"])
        s = ""
        for i in output:
            w = ANS.vocab.itos[torch.argmax(i)]
            if w == '<eos>':
                break
            s = s + w
        print(s)
    print(seq2seq)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt as e:
        print("[STOP]", e)



