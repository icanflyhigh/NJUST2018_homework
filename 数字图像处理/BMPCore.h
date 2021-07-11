#pragma once
#include <Windows.h>
float fSqrtByCarmack(float number);
void AEDOpGrad1_8bit(BYTE *pSrc, int width, int height, BYTE *pDst);
void AEDOpLaplacian4_8bit(BYTE *pSrc, BYTE *pTmp, int width, int height, BYTE *pDst);
void AEDOpSobel_8bit(BYTE *pGryImg, int width, int height, BYTE *pDstImg);
void AEDOpShenJun4_8bit(
	BYTE *pSrcImg, //ԭʼ�Ҷ�ͼ�� 
	BYTE *pTmpImg, //����ͼ��
	int width, int height,
	double a0,
	BYTE *pSJImg
);
void AGuassFilter1d_8bit(BYTE * pSrc, int width, int height, int * pFilter, int m, BYTE * pDes);
void getGuassFilter_8bit(double dev, int m, int * pFilter, int offset = 17);
void AEDOpCanny_8bit(
	BYTE *pSrcImg,							// ԭͼ��
	BYTE * pTmpImg,
	int *dx, int *dy,						// sobel���Ӵ���õ����ݶ�
	int width, int height,					// ͼ����
	int *pFilter, double sigma, int Fsize,	// ��˹�˲����Ĳ���
	int uBound, int lBound,					// ��˫��ֵ�㷨�������ӱ�Ե
	BYTE * pDstImg							// Ŀ��ͼ��
);

void AConv2d3_8bit(BYTE *pSrcImg, int width, int height, double *pFilter, BYTE *pDstImg);


//___________hw5________________
//void getHist2d(BYTE *pSrcImg, BYTE *pAvgImg, int width, int height, int *hist);
void getHist8b(BYTE *pSrcImg, int pixleNum, int *hist);
int getOtsuThreshold8b(int *hist, int pixelNum);
void PGry2Bin(BYTE *pSrcImg, int pixelNum, int threshold, BYTE *pDstImg);
int RmwGetOtsuThreshold(int *histogram, int nSize);
void GuassianBlur(BYTE *pSrcImg, BYTE *pTmpImg, int width, int height, double sigma, int Fsize, int *pFilter, BYTE *pDstImg);
void AMinFilter(BYTE *pSrcImg, int width, int height, int fw, int fh, BYTE *pDstImg);
void AMaxFilter(BYTE * pSrcImg, int width, int height, int fw, int fh, BYTE * pDstImg);
void Hist_kMeans(int *hist, int len, int class_num, double *Center, double bound = 1e-8, int max_iter=1000, int *iniCenter=NULL);


//____________hw6___________________
void HoughLine(BYTE * pSrcImg, int width, int height, int *pCount, int *X, int *Y, int N);
void DrawABCLine(BYTE *pGryImg, int width, int height,
	double A, double B, double C,
	int color
);
// ���������㷨
int TraceContour(BYTE *pBinImg, int width, int height,//��ֵͼ��
	int x0, int y0, //�������
	bool isOuter, //�Ƿ���������
	BYTE *pChainCode, //�ⲿ����õ�һ���ڴ�ռ䣬�����������
	int maxChainCodeNum //���Դ�ŵ�����������
);
//������
void DrawContour(BYTE *pGryImg, int width, int x, int y, BYTE *pCode, int N, BYTE color);
//��Ӿ���
void ContourRect(int x0, int y0, BYTE *pCode, int N, int *x1, int *x2, int *y1, int *y2);
//�ܳ�
double ContourPerimeter(BYTE *pChainCode, int n);
//���
double ContourArea(BYTE *pChainCode, int n);
//������Χ�����ظ���
int ContourPixels(BYTE *pChainCode, int n);
//�����Ƿ�������Χ
bool isPixelInContour(int x0, int y0, BYTE *pChainCode, int n, int x, int y);
//��������㷨
void FillContour(BYTE *pGryImg, int width, int height,//�����ͼ��
	int x0, int y0, //�������
	bool isOuter, //�Ƿ���������
	BYTE *pCode, //����
	int N, //�������
	int regionC, //�����ڲ�����ɫ
	int contourC, //��������ɫ
	int nonC //�����ڵ���ɫ
);

void getChainCode_fill(BYTE *pSrcImg, BYTE *pShowImg, BYTE *pChainCode, int width, int height);

