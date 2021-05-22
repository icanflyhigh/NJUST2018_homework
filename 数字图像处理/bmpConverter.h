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
	// �Ҷ�ͼ��ת
	void InvertImg();
	// ��ȡbmp
	bool BmpFile2Img(const char * DstFile);
	// ����bmp
	bool Img2Bmp(const char * DstFile, int bitCnt=0, char RGBMOD='\0');
	// ��ȡ14bit raw���ݣ��Ѿ�д��
	bool read14bitRaw(const char * DstFile);
	//RGBת�Ҷ�ͼ
	void RGB2Gry(bool table_chk=true, bool inplace=true);
	// �������죨����任��
	void PAffine(double k, double b);
	// ��ֵ��׼��涨��
	void P2avgstd8Bit(double mean = 128.0, double stddev = 1);
	// 8bitֱ��ͼ��׼��ʵ��
	void PHistogramEqualize8bit();
	// 24bitֱ��ͼ��׼��ʵ�֣�����ͨ���ֱ�ʵ�֣�
	void PHistogramEqualize24bit();
	// 24bitֱ��ͼ��׼��ʵ�֣�����ͨ����һ��ʵ�֣�
	void PHistogramEqualize24bit1();
	// 24bitֱ��ͼ��׼��ʵ�֣�����ͨ���ľ�ֵʵ�֣�
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

