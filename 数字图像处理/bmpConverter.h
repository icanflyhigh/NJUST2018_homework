#pragma once
#include <Windows.h>
#include "Img.h"
#include "BMPCore.h"

class bmpConverter : public Img
{
public:
	bmpConverter(const char * orgfile);
	bmpConverter();
	bmpConverter(Img);
	bmpConverter(bmpConverter &);
	bmpConverter(bmpConverter &&);
	~bmpConverter();
	// �Ҷ�ͼ��ת
	void InvertImg();
	// ��ȡbmp
	bool BmpFile2Img(const char * DstFile);
	// ����bmp
	bool Img2Bmp(const char * DstFile, int bitCnt=0, char RGBMOD='\0');
	// ��ȡ14bit raw���ݣ��Ѿ�д��
	bool read14bitRaw(const char * DstFile);
	// RGBת�Ҷ�ͼ
	void RGB2Gry(bool table_chk=true, bool inplace=true);
	// RGBת�Ҷ�ͼ2��ֻ���ڲ���
	void RGB2Gry2(bool table_chk=true, bool inplace=true);

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

	// ��άά��ֵ�˲���ͨ��
	Img AavgFilter2d1c(int m, int n, bool inplace = true, char mod='c');
	// �Ҷ�ͼ��ת
	Img PInvert1c(char mod = 'n', bool inplace=true);
	// ��ά��ֵ�˲���ͨ��
	Img AMedianFilter2d1c(int m, int n, bool inplace = true, char mod = 'c');
	// ��ά��˹�˲���ͨ��
	Img AGuassFilter2d1c(double dev, int m, bool inplace = true, char mod = 'c');

	// ��Ե��⣬һ���ݶ�+������
	Img AEadgeDectGrad_SJ2d1c(double a0=0.5, bool inplace=true);
	// ��Ե��⣬�����ݶ�+һ���ݶ�
	Img AEadgeDectLaplacain_Gd2d1c(double a0=0.5, BYTE threshold=10, bool inplace=true);
	// ��Ե��⣬sobel+������
	Img AEadgeDectsobel_ShenJun2d1c(double a0=0.5, BYTE threshold=64, bool inplace=true);

	Img AEadgeCanny2d1c(double sigma, int Fsize, int uBound, int lBound, bool inplace = true);

	void num_dect();

	void test();
private:
	void PHistogramEqualize14bit(short * pRawImg);
	void PHistogramEqualize14bit2(short * pRawImg);
	void getHistGram8bit(int * hist);
	void getHistGram24bitavg(int * hist);

	void getHistGram24bit(int * hist);
	bool Img28bitBmp(const char * DstFile, char mod);
	bool Img224bitBmp(const char * DstFile);
	//void AavgFilter2d_row_cal(BYTE * pSrc, int m, int n, BYTE  * pDes);
	void AavgFilter2d8bit_col_cal(BYTE * pImg, int width, int height, int m, int n, BYTE  * pDes);
	void AgetCalGraph(BYTE * pImg, int width, int height, int m, int n, int * pSum);
	void AavgFilter2d8bit_calGraph(int  * pSum, int m, int n, BYTE  * pDes);
	void PInvert8bit_SSE(BYTE * pImg, int sum, BYTE * pDes);
	void PInvert8bit(BYTE * pImg, int sum, BYTE * pDes);
	void AMedianFilter8bit_col_cal(BYTE * pImg, int width, int height, int m, int n, BYTE  * pDes);
	void AGuassFilter1d(BYTE * pSrc, int width, int height, int * pFilter, int n, BYTE * pDes);
	void T(BYTE * pSrC, int w, int h, BYTE * pDes);

	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;
	//һ������Ϊ�������ҵ����Ƶ�д���ĺ���
	// ��С��1/16
	void shrink16(BYTE *pSrc, int width, int heigt, BYTE *pDst);

};

