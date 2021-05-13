#pragma once
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include "switcher.h"
#include <iostream>
class bmpConverter
{
public:
	bmpConverter(const char * orgfile);
	bmpConverter();
	void InvertImg();
	void InvertImg(const char * DstFile);
	void PesudoColImg();
	void PesudoColImg(const char * DstFile);
	bool save(const char * DstFile);
	bool BmpFile2Img(const char * DstFile);
	bool Img2Bmp(const char * DstFile, int bitCnt=0, char RGBMOD='\0');
	~bmpConverter();

private:
	bool Img28bitBmp(const char * DstFile, char mod);
	bool Img224bitBmp(const char * DstFile);
	BYTE * pImg=nullptr;
	long width, height;
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;
	int channel;

};

