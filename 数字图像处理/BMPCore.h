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
