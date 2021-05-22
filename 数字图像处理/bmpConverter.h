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
	// 灰度图像反转
	void InvertImg();
	// 读取bmp
	bool BmpFile2Img(const char * DstFile);
	// 保存bmp
	bool Img2Bmp(const char * DstFile, int bitCnt=0, char RGBMOD='\0');
	// 读取14bit raw数据，已经写死
	bool read14bitRaw(const char * DstFile);
	//RGB转灰度图
	void RGB2Gry(bool table_chk=true, bool inplace=true);
	// 线性拉伸（仿射变换）
	void PAffine(double k, double b);
	// 均值标准差规定化
	void P2avgstd8Bit(double mean = 128.0, double stddev = 1);
	// 8bit直方图标准化实现
	void PHistogramEqualize8bit();
	// 24bit直方图标准化实现（三个通道分别实现）
	void PHistogramEqualize24bit();
	// 24bit直方图标准化实现（三个通道在一起实现）
	void PHistogramEqualize24bit1();
	// 24bit直方图标准化实现（三个通道的均值实现）
	void PHistogramEqualize24bit2();
	~bmpConverter();
	bmpConverter(bmpConverter &);
	bmpConverter(bmpConverter &&);
private:
	void PHistogramEqualize14bit(short * pRawImg);
	void PHistogramEqualize14bit2(short * pRawImg);
	void getHistGram8bit(int * hist);
	void getHistGram24bitavg(int * hist);
	void getHistGram24bit(int * hist);
	bool Img28bitBmp(const char * DstFile, char mod);
	bool Img224bitBmp(const char * DstFile);
	BYTE * pImg=nullptr;
	long width, height;
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;
	int channel;

};

