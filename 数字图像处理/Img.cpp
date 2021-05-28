#include "Img.h"
#include <cstdio>
Img::Img()
{
}

Img::Img(int w, int h, int c):
	width(w), height(h), channel(c)
{
	pImg = new BYTE[w * h *c];
	if (!pImg)
	{
		puts("Í¼Ïñ´´½¨Ê§°Ü");
	}
}

Img::Img(BYTE * p, int w, int h, int c):
	pImg(p), width(w), height(h), channel(c)
{

}

Img::Img(Img && a):
	width(a.width), height(a.height), channel(a.channel)
{
	int sum = width * height * channel;
	if (sum && a.pImg)
	{
		memcpy(pImg, a.pImg, sum * sizeof(BYTE));
	}
	else
	{
		delete pImg;
		pImg = nullptr;
	}
}

Img::Img(Img & a):
	width(a.width), height(a.height), channel(a.channel)
{
	int sum = width * height * channel;
	if (sum && a.pImg)
	{
		memcpy(pImg, a.pImg, sum * sizeof(BYTE));
	}
	else
	{
		delete pImg;
		pImg = nullptr;
	}
}

Img::~Img()
{
	delete pImg;
}

void Img::T()
{

}
