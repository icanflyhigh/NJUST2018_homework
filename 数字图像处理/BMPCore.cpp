#include "BMPCore.h"
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <nmmintrin.h>
#define DEBUG
using namespace std;

float fEPS = 1e-6;
double dEPS = 1e-10;

float fSqrtByCarmack(float number)
{
	int i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(int *)&y;            
	i = 0x5f375a86 - (i >> 1); 
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));   
	
	return 1.0f / y;
}

// �������㣬��Ե����ݶ�����4����8bitͼ��
void AEDOpGrad1_8bit(BYTE *pSrc, int width, int height, BYTE *pDst)
{
	BYTE *pCur, *pDes;
	int dx, dy;
	int x, y, i, m;
	BYTE LUT[1024];  // �ֲ����

	for (i = 0; i < 1024; i++)LUT[i] = (BYTE)sqrt(double(i));

	for (y = 0, pCur = pSrc, pDes = pDst; y < height - 1; y++)
	{
		for (x = 0; x < width - 1; x++, pCur++)
		{
			dx = *pCur - pCur[1];
			dy = *pCur - pCur[width];
			m = dx * dx + dy * dy;
			*(pDes++) = m < 1024 ? LUT[m] : (min)(255, (int)(sqrt((double) m)));
		}
		*(pDes++) = 0;  // β�в���,��Եǿ�ȸ�0
		pCur++;
	}
	memset(pDes, 0, width);  // β�в���,��Եǿ�ȸ�0
	return;
}
// �������㣬��Ե�������������4����8bitͼ��
void AEDOpSobel_8bit(BYTE *pGryImg, int width, int height, BYTE *pSbImg)
{
	BYTE *pGry, *pSb;
	int dx, dy;
	int x, y;

	memset(pSbImg, 0, width); //���в���,��Եǿ�ȸ�0
	for (y = 1, pGry = pGryImg + width, pSb = pSbImg + width; y < height - 1; y++)
	{
		*(pSb++) = 0;  //���в���,��Եǿ�ȸ�0
		pGry++;
		for (x = 1; x < width - 1; x++, pGry++)
		{
			//��dx
			dx = *(pGry - 1 - width) + (*(pGry - 1) * 2) + *(pGry - 1 + width);
			dx -= *(pGry + 1 - width) + (*(pGry + 1) * 2) + *(pGry + 1 + width);
			//��dy
			dy = *(pGry - width - 1) + (*(pGry - width) * 2) + *(pGry - width + 1);
			dy -= *(pGry + width - 1) + (*(pGry + width) * 2) + *(pGry + width + 1);
			//���
			*(pSb++) = min(255, abs(dx) + abs(dy));
		}
		*(pSb++) = 0; //β�в���,��Եǿ�ȸ�0
		pGry++;
	}
	memset(pSb, 0, width); //β�в���,��Եǿ�ȸ�0
	return;
}


// �������㣬��Ե���Laplacian����4����8bitͼ��
void AEDOpLaplacian4_8bit(BYTE *pSrc, BYTE *pTmp, int width, int height, BYTE *pDst)
{
	BYTE *pCur, *pDes;
	int grad;
	int x, y;

	memset(pTmp, 0, width);  // ���в���,��Եǿ�ȸ�0
	for (y = 1, pCur = pSrc + width, pDes = pTmp + width; y < height - 1; y++)
	{
		*(pDes++) = 0;  // ���в���,��Եǿ�ȸ�0
		pCur++;
		for (x = 1; x < width - 1; x++, pCur++)
		{
			// grad����
			grad = -((int)*pCur) * 4;
			grad += pCur[-width];
			grad += pCur[-1];
			grad += pCur[+1];
			grad += pCur[+width];
			*(pDes++) = grad > 0;
		}
		*(pDes++) = 0; //β�в���,��Եǿ�ȸ�0
		pCur++;
	}
	memset(pDes, 0, width); //β�в���,��Եǿ�ȸ�0

	pDes = pDst + width, pCur = pTmp + width;
	memset(pDst, 0, width);
	for (y = 1; y < height - 1; y++)
	{
		*pDes++ = 0, pCur++;
		for (x = 1; x < width - 1; x++, pCur++)
		{
			if (*pCur && (!pCur[-1] || !pCur[1] || !pCur[-width] || !pCur[width]))*pDes++ = 255;
			else *pDes++ = 0;
		}
		*pDes++ = 0, pCur++;
	}
	memset(pDes, 0, width);
	return;
}


// �������㣬��Ե���������4����8bitͼ��
void AEDOpShenJun4_8bit(
	BYTE *pSrcImg, //ԭʼ�Ҷ�ͼ�� 
	BYTE *pTmpImg, //����ͼ��
	int width, int height,
	double a0,
	BYTE *pDstImg
)
{
	BYTE * pCur, * pDes, * pEnd;
	int LUT[511], *A;
	int x, y, i, pre;
	// ��ʼ��
	A = LUT + 255;
	for (A[0] = 0, i = 1; i < 256; i++)  // ����ȡ��
	{
		A[i] = a0 * i + 0.5;	
		A[-i] = -a0 * i - 0.5;  
	}
	// ���
	// ����
	for (y = 0, pCur = pSrcImg, pDes = pTmpImg; y < height; y++)
	{
		pre = *pDes++ = *pCur++;
		// ������
		for (x = 1; x < width; x++)
		{
			pre = *pDes++ = pre + A[*pCur++ - pre];
		}
		// ���ҵ���
		pDes--;
		for (x = 1; x < width; x++)
		{
			pDes--;
			pre = *pDes = pre + A[*pDes - pre];
		}
		pDes += width;
	}
	// ����
	for (x = 0, pDes=pTmpImg; x < width; x++)
	{
		// ���ϵ���
		pDes = pTmpImg + x, pre = *pDes;
		for (y = 1; y < height; y++)
		{
			pDes += width;
			pre = *pDes = pre + A[*pDes - pre];
		}
		// ���µ���
		for (y = 1; y < height; y++)
		{
			pDes -= width;
			pre = *pDes = pre + A[*pDes - pre];
		}
	}


	// ������ݶȵķ���
	for (pCur = pTmpImg, pEnd = pTmpImg + width * height, pDes = pSrcImg; pCur<pEnd; pDes++)
	{
		*(pCur++) = (*pCur <= *pDes);
	}

	// ���Ե
	pDes = pDstImg + width, pCur = pTmpImg + width;
	memset(pDstImg, 0, width);
	for (y = 1; y < height - 1; y++)
	{
		*pDes++ = 0, pCur++;
		for (x = 1; x < width - 1; x++, pCur++)
		{
			if (!*pCur && (pCur[-1] ||pCur[1] || pCur[-width] || pCur[width]))*pDes++ = 255;
			else *pDes++ = 0;
		}
		*pDes++ = 0, pCur++;
	}
#ifdef DEBUG
	//printf("���Եok\n");
#endif // DEBUG
	memset(pDes, 0, width);
#ifdef DEBUG
	//printf("return ok\n");
#endif // DEBUG

	return;
}

void AGuassFilter1d_8bit(BYTE * pSrc, int width, int height, int * pFilter, int m, BYTE * pDes)
{
	int offset = 17;
	int halfm, y, x, f, sum = 0;
	BYTE  *pCur, *pRes, *pY;

	halfm = m / 2;

	for (y = 0, pY = pSrc, pRes = pDes; y < height; y++, pY += width)
	{
		// �������
		for (x = 0, sum = 0; x < halfm; x++, sum = 0)
		{
			for (f = halfm - x, pCur = pY; f < m; f++)
			{
				sum += pFilter[f] * *pCur++;
			}
			sum >>= offset;
			*pRes++ = sum;
		}
		// �����м�
		for (x = halfm, sum = 0; x < width - halfm; x++, sum = 0)
		{
			for (f = 0, pCur = pY + x - halfm; f < m; f++)
			{
				sum += pFilter[f] * *pCur++;
			}
			sum >>= offset;
			*pRes++ = sum;
		}
		// �����Ҳ�
		for (x = width - halfm, sum = 0; x < width; x++, sum = 0)
		{
			for (f = 0, pCur = pY + x - halfm; pCur < pY + width; f++)
			{
				sum += pFilter[f] * *pCur++;
			}
			sum >>= offset;
			*pRes++ = sum;
		}
	}
}

void Transpose(BYTE * pSrc, int w, int h, BYTE * pDes)
{
	BYTE * pX, *pCur;
	int x, y;

	pCur = pDes;

	for (x = 0; x < w; x++)
	{
		for (y = 0, pX = pSrc + x; y < h; y++, pX += w)
		{
			*pCur++ = *pX;
		}
	}

	return;
}

void getGuassFilter_8bit(double dev, int m, int * pFilter, int offset)
{
	int halfm;
	double sum;
	double dF[1024];

	halfm = m / 2;
	sum = dF[halfm] = 1;

	for (int i = 1; i <= halfm; i++)
	{
		dF[halfm + i] = dF[halfm - i] = exp(-i * i / 2.0 / dev / dev);
		sum += dF[halfm + i] + dF[halfm + i];
	}
	for (int i = 0; i <= halfm; i++)
	{
		pFilter[halfm + i] = pFilter[halfm - i] = (1 << offset) * dF[halfm + i] / sum;
	}
}

void AEDOpSobe4Canny(BYTE *pGryImg, int width, int height, int *pDX, int *pDY, BYTE *pSbImg)
{
	BYTE *pGry, *pSb;
	int *pDx, *pDy;
	int dx, dy;
	int x, y;

	pDx = pDX;
	pDy = pDY;
	memset(pSbImg, 0, width); //���в���,��Եǿ�ȸ�0
	for (y = 1, pGry = pGryImg + width, pSb = pSbImg + width; y < height - 1; y++)
	{
		*(pSb++) = 0;  //���в���,��Եǿ�ȸ�0
		*(pDx++) = 0;
		*(pDy++) = 0;
		pGry++;
		for (x = 1; x < width - 1; x++, pGry++)
		{
			//��dx
			dx = *(pGry - 1 - width) + (*(pGry - 1) * 2) + *(pGry - 1 + width);
			dx -= *(pGry + 1 - width) + (*(pGry + 1) * 2) + *(pGry + 1 + width);
			//��dy
			dy = *(pGry - width - 1) + (*(pGry - width) * 2) + *(pGry - width + 1);
			dy -= *(pGry + width - 1) + (*(pGry + width) * 2) + *(pGry + width + 1);
			//���
			*(pSb++) = min(255, abs(dx) + abs(dy));
			*(pDx++) = dx;
			*(pDy++) = dy;

		}
		*(pSb++) = 0; //β�в���,��Եǿ�ȸ�0
		*(pDx++) = 0;
		*(pDy++) = 0;
		pGry++;
	}
	memset(pSb, 0, width); //β�в���,��Եǿ�ȸ�0
	return;
}

// ���ݶȷ�ֵ���зǼ���ֵ����
void GradMagThresholding(BYTE *pSrcImg, int width, int height, int *pDx, int *pDy)
{
	BYTE *pCur;
	int *pDX, *pDY;
	BYTE Cur;
	int DX, DY;
	int x, y;
	BYTE btan, binvtan;
	// ������Ե
	for (y = 1, pCur = pSrcImg + width, pDX = pDx + width, pDY = pDy + width; y < height - 1; y++)
	{
		pCur++, pDX++, pDY++; 
		for (x = 1; x < width - 1; x++, pCur++, pDX++, pDY++)
		{
			Cur = *pCur, DX = *pDX, DY = *pDY;
			// ȡ��Χ��8�������Ƚ�
			if (!Cur)continue; // ģΪ0�������
			if (!DX)  // �����ݶ�
			{
				if (Cur < pCur[-width] || Cur < pCur[width])*pCur = 0;
			}
			else if (!DY) // �����ݶ�
			{
				if (Cur < pCur[-1] || Cur < pCur[1])*pCur = 0;
			}
			else 
			{
				btan = DY / DX;
				if (!btan)  // abs(tan)С��1
				{
					binvtan = DX / DY;
					if (binvtan > 2 || binvtan < -2)
					{
						if (Cur < pCur[-1] || Cur < pCur[1])*pCur = 0;
					}
					else if (binvtan > 0)
					{
						if (Cur < pCur[-width - 1] || Cur < pCur[width + 1])*pCur = 0;
					}
					else
					{
						if (Cur < pCur[-width + 1] || Cur < pCur[width - 1])*pCur = 0;
					}
				}
				else
				{
					if (btan >= 2 || btan < -2)
					{
						if (Cur < pCur[-width] || Cur < pCur[width])*pCur = 0;
					}
					else if (btan > 0)
					{
						if (Cur < pCur[-width - 1] || Cur < pCur[width + 1])*pCur = 0;
					}
					else
					{
						if (Cur < pCur[-width + 1] || Cur < pCur[width - 1])*pCur = 0;
					}
				}
			}
		}
		pCur++, pDX++, pDY++;
	}
}


void DoubThre_EdgTrack(BYTE *pSrcImg, int width, int height, int uBound, int lBound)
{
	BYTE *pCur;
	int x, y;

	for (y = 1, pCur = pSrcImg + width; y < height - 1; y++)  // �������Ե
	{
		pCur++;
		for (x = 1; x < width - 1; x++)
		{
			BYTE &Cur = *pCur++;
			if (Cur > uBound)Cur = 255;  // �õ���ǿ���ӵ�
			else if (Cur < lBound)Cur = 0;  // �õ�������
			else  // �õ�ʱ�����ӵ�
			{
				// �����Χ��ǿ���ӵ㣬����->ǿ
				if (pCur[-width - 1] == 255 || pCur[-width] == 255 || pCur[-width + 1] == 255 ||
					pCur[-1] == 255 || pCur[1] > uBound ||
					pCur[width - 1] > uBound || pCur[width] > uBound || pCur[width + 1] > uBound)
				{
					Cur = 255;
				}
				else Cur = 0;  // ������->��
			}
		}
		pCur++;
	}
}

void AEDOpCanny_8bit(
	BYTE *pSrcImg,							// ԭͼ��
	BYTE * pTmpImg,		
	int *dx, int *dy,						// sobel���Ӵ���õ����ݶ�
	int width, int height,					// ͼ����
	int *pFilter, double sigma, int Fsize,	// ��˹�˲����Ĳ���
	int uBound, int lBound,					// ��˫��ֵ�㷨�������ӱ�Ե
	BYTE * pDstImg							// Ŀ��ͼ��
)
{
	// ��˹ƽ��
	// sobel���Ӵ���
	// ���ݶȷ�ֵ���зǼ���ֵ����
	// ��˫��ֵ�㷨�������ӱ�Ե


	// ��ʼ��
	Fsize |= 1;
	getGuassFilter_8bit(sigma, Fsize, pFilter);

	// ʩչ��˹ƽ��
	AGuassFilter1d_8bit(pSrcImg, width, height, pFilter, Fsize, pDstImg);
	Transpose(pDstImg, width, height, pTmpImg);
	AGuassFilter1d_8bit(pTmpImg, height, width, pFilter, Fsize, pDstImg);
	Transpose(pDstImg, height, width, pTmpImg);
	
	// sobel���Ӵ���
	AEDOpSobe4Canny(pTmpImg, width, height, dx, dy, pDstImg);

	// ���ݶȷ�ֵ���зǼ���ֵ����
	GradMagThresholding(pDstImg, width, height, dx, dy);

	// ��˫��ֵ�㷨�������ӱ�Ե
	DoubThre_EdgTrack(pDstImg, width, height, uBound, lBound);
}

void AConv2d3_8bit(BYTE * pSrcImg, int width, int height, double * pFilter, BYTE * pDstImg)
{
	BYTE *pCur, *pDes;
	int x, y, i, j;
	double *pF, sum;

	pCur = pSrcImg;
	pDes = pDstImg;
	pF = pFilter;

	for (x = 0; x < width; x++)*pDes++ = *pCur++;  // ��Ե����
	for (y = 1; y < height - 1; y++)
	{
		*pDes++ = *pCur++;
		for (x = 1; x < width - 1; x++, pCur++)
		{
			sum = pCur[-1 - width] * pF[0] + pCur[-width] * pF[1] + pCur[1 - width] * pF[2] +
				pCur[-1] * pF[3] + pCur[0] * pF[4] + pCur[1] * pF[5] +
				pCur[-1 + width] * pF[6] + pCur[width] * pF[7] + pCur[width + 1] * pF[8];
			//cout << pCur[-1 - width] * pF[0] << endl;
			//cout << sum << endl;
			*pDes++ = max(0, min(255, (sum)));
		}
		*pDes++ = *pCur++;

	}
	for (x = 0; x < width; x++)*pDes++ = *pCur++;  // ��Ե����

}





