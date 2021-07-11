#pragma once
#include <Windows.h>
float fSqrtByCarmack(float number);
void AEDOpGrad1_8bit(BYTE *pSrc, int width, int height, BYTE *pDst);
void AEDOpLaplacian4_8bit(BYTE *pSrc, BYTE *pTmp, int width, int height, BYTE *pDst);
void AEDOpSobel_8bit(BYTE *pGryImg, int width, int height, BYTE *pDstImg);
void AEDOpShenJun4_8bit(
	BYTE *pSrcImg, //原始灰度图像 
	BYTE *pTmpImg, //辅助图像
	int width, int height,
	double a0,
	BYTE *pSJImg
);
void AGuassFilter1d_8bit(BYTE * pSrc, int width, int height, int * pFilter, int m, BYTE * pDes);
void getGuassFilter_8bit(double dev, int m, int * pFilter, int offset = 17);
void AEDOpCanny_8bit(
	BYTE *pSrcImg,							// 原图像
	BYTE * pTmpImg,
	int *dx, int *dy,						// sobel算子处理得到的梯度
	int width, int height,					// 图像宽高
	int *pFilter, double sigma, int Fsize,	// 高斯滤波器的参数
	int uBound, int lBound,					// 用双阈值算法检测和连接边缘
	BYTE * pDstImg							// 目标图像
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
// 轮廓跟踪算法
int TraceContour(BYTE *pBinImg, int width, int height,//二值图像
	int x0, int y0, //轮廓起点
	bool isOuter, //是否是外轮廓
	BYTE *pChainCode, //外部申请好的一个内存空间，用来存放链码
	int maxChainCodeNum //可以存放的最大链码个数
);
//画轮廓
void DrawContour(BYTE *pGryImg, int width, int x, int y, BYTE *pCode, int N, BYTE color);
//外接矩形
void ContourRect(int x0, int y0, BYTE *pCode, int N, int *x1, int *x2, int *y1, int *y2);
//周长
double ContourPerimeter(BYTE *pChainCode, int n);
//面积
double ContourArea(BYTE *pChainCode, int n);
//轮廓包围的像素个数
int ContourPixels(BYTE *pChainCode, int n);
//像素是否被轮廓包围
bool isPixelInContour(int x0, int y0, BYTE *pChainCode, int n, int x, int y);
//轮廓填充算法
void FillContour(BYTE *pGryImg, int width, int height,//待填充图像
	int x0, int y0, //轮廓起点
	bool isOuter, //是否是外轮廓
	BYTE *pCode, //链码
	int N, //链码个数
	int regionC, //区域内部的颜色
	int contourC, //轮廓点颜色
	int nonC //不存在的颜色
);

void getChainCode_fill(BYTE *pSrcImg, BYTE *pShowImg, BYTE *pChainCode, int width, int height);

