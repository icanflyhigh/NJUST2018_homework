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
