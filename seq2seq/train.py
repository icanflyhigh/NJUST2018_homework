import os
import math
import argparse
import torch
from torch import optim
from torch.autograd import Variable
from torch.nn.utils import clip_grad_norm_
from torch.nn import functional as F
from model import Encoder, Decoder, Seq2Seq
from utils import load_dataset


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


def evaluate(model, val_iter, vocab_size, ASK, ANS):
    with torch.no_grad():
        model.eval()
        pad = ANS.vocab.stoi['<pad>']
        total_loss = 0
        for b, batch in enumerate(val_iter):
            Ask, len_Ask = batch.Ask
            Ans, len_Ans = batch.Ans
            Ask = Ask.data.cuda()
            Ans = Ans.data.cuda()
            output = model(Ask, Ans, teacher_forcing_ratio=0.0)
            loss = F.nll_loss(output[1:].view(-1, vocab_size),
                              Ans[1:].contiguous().view(-1),
                              ignore_index=pad)
            total_loss += loss.data.item()
        return total_loss / len(val_iter)


def train(e, model, optimizer, train_iter, vocab_size, grad_clip, ASK, ANS):
    model.train()
    total_loss = 0
    pad = ANS.vocab.stoi['<pad>']
    for b, batch in enumerate(train_iter):
        Ask, len_Ask = batch.Ask
        Ans, len_Ans = batch.Ans
        Ask, Ans = Ask.cuda(), Ans.cuda()
        optimizer.zero_grad()
        output = model(Ask, Ans)
        loss = F.nll_loss(output[1:].view(-1, vocab_size),
                          Ans[1:].contiguous().view(-1),
                          ignore_index=pad)
        loss.backward()
        clip_grad_norm_(model.parameters(), grad_clip)
        optimizer.step()
        total_loss += loss.data.item()

        if b % 100 == 0 and b != 0:
            total_loss = total_loss / 100
            print("[%d][loss:%5.2f][pp:%5.2f]" %
                  (b, total_loss, math.exp(total_loss)))
            total_loss = 0
    # print('|'*50+'\n', "train over")


def main():
    args = parse_arguments()
    hidden_size = 512
    embed_size = 256
    assert torch.cuda.is_available()

    print("[!] preparing dataset...")
    train_iter, val_iter, test_iter, ASK, ANS = load_dataset(args.batch_size)
    ask_size, ans_size = len(ASK.vocab), len(ANS.vocab)
    print("[TRAIN]:%d (dataset:%d)\t[TEST]:%d (dataset:%d)"
          % (len(train_iter), len(train_iter.dataset),
             len(test_iter), len(test_iter.dataset)))
    print("[ASK_vocab]:%d [ANS_vocab]:%d" % (ask_size, ans_size))

    print("[!] Instantiating models...")
    encoder = Encoder(ask_size, embed_size, hidden_size,
                      n_layers=2, dropout=0.5)
    decoder = Decoder(embed_size, hidden_size, ans_size,
                      n_layers=1, dropout=0.5)
    seq2seq = Seq2Seq(encoder, decoder).cuda()
    optimizer = optim.Adam(seq2seq.parameters(), lr=args.lr)
    print(seq2seq)

    best_val_loss = None
    for e in range(1, args.epochs + 1):
        train(e, seq2seq, optimizer, train_iter,
              ans_size, args.grad_clip, ASK, ANS)
        val_loss = evaluate(seq2seq, val_iter, ans_size, ASK, ANS)
        print("[Epoch:%d] val_loss:%5.3f | val_pp:%5.2fS"
              % (e, val_loss, math.exp(val_loss)))

        # Save the model if the validation loss is the best we've seen so far.
        # if not best_val_loss or val_loss < best_val_loss:
        print("[!] saving model...")
        if not os.path.isdir(".save"):
            os.makedirs(".save")
        torch.save(seq2seq.state_dict(), './.save/seq2seq_%d.pt' % (e))
        best_val_loss = val_loss
        # elif e % 10 == 0:
        #     torch.save(seq2seq.state_dict(), './.save/seq2seq_%d.pt' % (e))
    test_loss = evaluate(seq2seq, test_iter, ans_size, ASK, ANS)
    print("[TEST] loss:%5.2f" % test_loss)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt as e:
        print("[STOP]", e)
