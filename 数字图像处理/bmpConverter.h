#pragma once
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <nmmintrin.h>
#include "Img.h"
#include "switcher.h"

class bmpConverter : public Img
{
public:
	bmpConverter(const char * orgfile);
	bmpConverter();
	bmpConverter(Img);
	bmpConverter(bmpConverter &);
	bmpConverter(bmpConverter &&);
	~bmpConverter();
	// 灰度图像反转
	void InvertImg();
	// 读取bmp
	bool BmpFile2Img(const char * DstFile);
	// 保存bmp
	bool Img2Bmp(const char * DstFile, int bitCnt=0, char RGBMOD='\0');
	// 读取14bit raw数据，已经写死
	bool read14bitRaw(const char * DstFile);
	// RGB转灰度图
	void RGB2Gry(bool table_chk=true, bool inplace=true);
	// RGB转灰度图2，只用于测试
	void RGB2Gry2(bool table_chk=true, bool inplace=true);

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
	
	// 二维维均值滤波
	Img AavgFilter2d1c(int m, int n, bool inplace = true, char mod='c');
	// 灰度图像反转，SSE
	Img PInvert1c(char mod = 'n', bool inplace=true);
	// 二维中值滤波
	Img AMedianFilter2d1c(int m, int n, bool inplace = true, char mod = 'c');
	// 高斯滤波
	Img AGuassFilter2d1c(double dev, int m, bool inplace = true, char mod = 'c');



private:
	void PHistogramEqualize14bit(short * pRawImg);
	void PHistogramEqualize14bit2(short * pRawImg);
	void getHistGram8bit(int * hist);
	void getHistGram24bitavg(int * hist);

	void getHistGram24bit(int * hist);
	bool Img28bitBmp(const char * DstFile, char mod);
	bool Img224bitBmp(const char * DstFile);
	//void AavgFilter2d_row_cal(BYTE * pSrc, int m, int n, BYTE  * pDes);
	void AavgFilter2d8bit_col_cal(int m, int n, BYTE  * pDes);
	void getCalGraph(int m, int n, int * pSum);
	void AavgFilter2d8bit_calGraph(int  * pSum, int m, int n, BYTE  * pDes);
	void PInvert8bit_SSE(BYTE * pDes);
	void PInvert8bit(BYTE * pDes);
	void AMedianFilter8bit_col_cal(int m, int n, BYTE  * pDes);
	void AGuassFilter1d(BYTE * pSrc, int width, int height, int * pFilter, int n, BYTE * pDes);
	Img T(BYTE * & pSrC, int w, int h, bool inplace = true);


	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;


};

