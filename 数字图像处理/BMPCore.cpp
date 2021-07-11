#include "BMPCore.h"
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <nmmintrin.h>
#define DEBUG
using namespace std;

const float fEPS = 1e-6;
const double dEPS = 1e-10;
const double PI = 3.1415926535;
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
	int x, y;
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

//_________hw5____________________________________

//�õ�ֱ��ͼ
void getHist8b(BYTE *pSrcImg, int pixleNum, int *hist)
{
	BYTE *pCur = pSrcImg, *pEnd = pSrcImg + pixleNum;
	memset(hist, 0, sizeof(int) * 256);

	while (pCur < pEnd)hist[*pCur++]++;
}

int getOtsuThreshold8b(int *hist, int HistLen)
{
	int ming = 0, maxg = HistLen - 1;
	// �õ��Ҷ���Сֵ�����ֵ
	for (; ming < HistLen && !hist[ming]; ming++);
	for (; maxg && !hist[maxg]; maxg--);
	if (maxg == ming)return ming;
	int maxThreshold = 0;  // ��ֵ���Լ���ֵ
	int numL = 0, numH = 0, sumL = 0, sumH = 0;
	int pNum[1024], *pN;
	int offset = 31;
	int i;
	double curf, maxf = -1;

	for (i = ming, pN = pNum; i <= maxg; i++, pN++)
	{
		//if (!hist[i])continue;
		*pN = hist[i] * i;
		sumH += *pN;
		numH += hist[i];
	}
	// �õ�������Ŀ��������
	while (offset && !((1 << offset)&numH))offset--;
	// ö�ٳ���ûҶ�
	for (i = ming, pN = pNum; i < maxg; i++, pN++)
	{	
		if (!hist[i])continue;
		sumH -= *pN;
		numH -= hist[i];
		sumL += *pN; 
		numL += hist[i];
		// ���longlong����1e15��,ת��Ϊdouble���о��ȶ�ʧ
		// ����һ��pixelnum��֤double���ȣ�����ͼƬ�����ش���1e7�����
		curf = (double(sumH) / numH - double(sumL) / numL) *
			((long long (sumH) * numL - long long(sumL) * numH) >> offset);
		if (curf > maxf)
		{
			maxThreshold = i;
			maxf = curf;
		}
	}
	return maxThreshold + 1;
}






int RmwGetOtsuThreshold(int *histogram, int nSize)
{
	int thre;
	int i, gmin, gmax;
	double dist, f, max;
	int s1, s2, n1, n2, n;

	// step.1-----ȷ��������Χ:��Сֵ----------------------------//
	gmin = 0;
	while (histogram[gmin] == 0) ++gmin;
	// step.2-----ȷ��������Χ:���ֵ----------------------------//
	gmax = nSize - 1;
	while (histogram[gmax] == 0) --gmax;
	// step.3-----���������ֵ-----------------------------------//
	if (gmin == gmax) return gmin; //���������ֲ�
	max = 0;
	thre = 0;
	//��ʼ��u1
	s1 = n1 = 0;
	//��ʼ��u2
	for (s2 = n2 = 0, i = gmin; i <= gmax; i++)
	{
		s2 += histogram[i] * i;
		n2 += histogram[i];
	}
	//����
	for (i = gmin, n = n2; i < gmax; i++)
	{   //����
		if (!histogram[i]) continue;
		//����s1s2
		s1 += histogram[i] * i;
		s2 -= histogram[i] * i;
		//����n1n2
		n1 += histogram[i];
		n2 -= histogram[i];
		//���ۺ���
		dist = (s1*1.0 / n1 - s2 * 1.0 / n2);
		f = dist * dist*(n1*1.0 / n)*(n2*1.0 / n);
		if (f > max)
		{
			max = f;
			thre = i;
		}
	}
	// step.4-----����-------------------------------------------//
	return thre + 1; //��ֵ��ʱ����>=thre,����Ҫ+1
}
// ��ֵ��
void PGry2Bin(BYTE *pSrcImg, int pixelNum, int threshold, BYTE *pDstImg)
{
	BYTE *pCur = pSrcImg, *pEnd = pSrcImg + pixelNum, *pDst = pDstImg;

	while (pCur < pEnd) *pDst++ = (*pCur++ < threshold) - 1;  // 1 - 1 = 0; 0 - 1 = -1(0xffffffff->255)��ʡȥ�˳˷�;

}


void GuassianBlur(BYTE *pSrcImg, BYTE *pTmpImg, int width, int height, double sigma, int Fsize, int *pFilter, BYTE *pDstImg)
{
	Fsize |= 1;
	getGuassFilter_8bit(sigma, Fsize, pFilter);

	// ʩչ��˹ƽ��
	AGuassFilter1d_8bit(pSrcImg, width, height, pFilter, Fsize, pTmpImg);
	Transpose(pTmpImg, width, height, pDstImg);
	AGuassFilter1d_8bit(pDstImg, height, width, pFilter, Fsize, pTmpImg);
	Transpose(pTmpImg, height, width, pDstImg);
}
// һ���򵥵���Сֵ�˲�
void AMinFilter(BYTE * pSrcImg, int width, int height, int fw, int fh, BYTE * pDstImg)
{
	int x, y, tx;
	BYTE * ty, minPix;
	BYTE *pCur = pSrcImg, *pDst = pDstImg;
	int halfx = fw / 2, halfy = fh / 2;
	fw |= 1, fh |= 1;

	// ���⴦������
	for (y = 0; y < halfy * width; y++)
			*pDst++ = *pCur++;
	for (y = halfy; y < height - halfy; y++)
	{
		for (x = 0; x < halfx; x++)  // ���⴦�����
			*pDst++ = *pCur++;
		for (; x < width - halfx; x++, pCur++)  // ��������
		{
			minPix = 255;
			for (ty = pCur - halfy * width ; ty <= pCur + halfy * width; ty += width)
			{
				for (tx = -halfx; tx <= halfx; tx++)
				{
					minPix = (min)(minPix, ty[tx]);
				}
			}
			*pDst++ = minPix;
		}  // x
		for (; x < width; x++)  // ���⴦���ұ�
			*pDst++ = *pCur++;
	}  // y
	// ���⴦������
	for (y = 0; y < halfy * width; y++)
		*pDst++ = *pCur++;
}
// һ���򵥵����ֵ�˲�
void AMaxFilter(BYTE * pSrcImg, int width, int height, int fw, int fh, BYTE * pDstImg)
{
	int x, y, tx;
	BYTE * ty, maxPix;
	BYTE *pCur = pSrcImg, *pDst = pDstImg;
	int halfx = fw / 2, halfy = fh / 2;
	fw |= 1, fh |= 1;

	// ���⴦������
	for (y = 0; y < halfy * width; y++)
		*pDst++ = *pCur++;
	for (y = halfy; y < height - halfy; y++)
	{
		for (x = 0; x < halfx; x++)  // ���⴦�����
			*pDst++ = *pCur++;
		for (; x < width - halfx; x++, pCur++)  // ��������
		{
			maxPix = 0;
			for (ty = pCur - halfy * width; ty <= pCur + halfy * width; ty += width)
			{
				for (tx = -halfx; tx <= halfx; tx++)
				{
					maxPix = (max)(maxPix, ty[tx]);
				}
			}
			*pDst++ = maxPix;
		}  // x
		for (; x < width; x++)  // ���⴦���ұ�
			*pDst++ = *pCur++;
	}  // y
	// ���⴦������
	for (y = 0; y < halfy * width; y++)
		*pDst++ = *pCur++;
}
// ֱ��ͼk��ֵ����
void Hist_kMeans(int *hist, int len, int class_num, double *Center, double bound, int max_iter, int *iniCenter)
{
	int i, g, iter;
	int  num[256];
	double sum[256];
	int hmax = len - 1, hmin = 0;
	double dist, dist1, tc;
	bool boundFlag;

	// ��ʼ��
	for (; hmin < len && !hist[hmin]; hmin++);
	for (; hmax > -1 && !hist[hmax]; hmax--);
	if (hmax - hmin < class_num) return;
	if (iniCenter)
	{
		// ���������ʼ�������Ĳ�����hmin��hmax����
		for (i = 0; i < class_num; i++)Center[i] = iniCenter[i];
	}
	else
	{
		int step = (hmax - hmin) / (class_num - 1);
		for (i = 0, g = hmin; i < class_num; i++, g += step) Center[i] = g;
	}

	// ִ��
	for (iter = 0; iter < max_iter; iter++)
	{
		// ����
		g = 0; // gָ��ǰһ���Ҷ�ֵ���ڵ����
		for (i = 0; i < class_num; i++)
		{
			sum[i] = 0;
			num[i] = 0;
		}
		for (i = hmin; i <= hmax; i++)
		{
			if (!hist[i])continue;
			// �õ�����ʵ�����
			dist = abs(i - Center[g]);
			while (g < class_num - 1 && (dist1 = abs(i - Center[g + 1])) < dist)
			{
				dist = dist1;
				g++;
			}  // g
			sum[g] += i * hist[i];
			num[g] += hist[i];
		}  // i

		// �õ��µ�����
		boundFlag = false;
		for (i = 0; i < class_num; i++)
		{
			if (!num[i])continue;
			tc = sum[i] / num[i];
			if (abs(tc - Center[i]) > bound)
				boundFlag = true;
			Center[i] = tc;
		}
		if (!boundFlag)
			return;
	}  // iter
}

//_____________hw6____________________
void HoughLine(BYTE * pSrcImg, int width, int height, int *pCount, int *X, int *Y, int N)
{	
	
	int *pCenter, *pCur;
	int maxThro, cosV, sinV;
	int theta, thro, i;

	maxThro = (int)sqrt(1.0*width*width + height * height + 0.5) + 1;
	memset(pCount, 0, sizeof(int)*(maxThro * 2) * 180);
	// ����任
	for (theta = 0; theta < 180; theta++) //����Ϊ1��
	{
		cosV = (int)(cos(theta * PI / 180) * (1 << 13)); //�Ŵ�8192��
		sinV = (int)(sin(theta * PI / 180) * (1 << 13));
		pCenter = pCount + (maxThro * 2)*theta + maxThro; //����ƫ��maxThro
		for (i = 0; i < N; i++)
		{
			thro = ((int(X[i] * cosV + Y[i] * sinV)) >> 13); //��С8192��,thro�Ĳ���Ϊ1
			pCenter[thro] += 1;
		}
	}

}
void DrawABCLine(BYTE *pGryImg, int width, int height,
	double A, double B, double C,
	int color
)
{
	int x, y;

	if (fabs(B) > fabs(A))
	{
		for (x = 0; x < width; x++)
		{
			y = (int)(-(C + A * x) / B + 0.5);
			if ((y >= 0) && (y < height))  *(pGryImg + y * width + x) = color;
		}
	}
	else
	{
		for (y = 0; y < height; y++)
		{
			x = (int)(-(C + B * y) / A + 0.5);
			if ((x >= 0) && (x < width))  *(pGryImg + y * width + x) = color;
		}
	}
	return;
}



// ���������㷨
int TraceContour
(
	BYTE *pBinImg, int width, int height,//��ֵͼ��
	int x0, int y0, //�������
	bool isOuter, //�Ƿ���������
	BYTE *pChainCode, //�ⲿ����õ�һ���ڴ�ռ䣬�����������
	int maxChainCodeNum //���Դ�ŵ�����������
)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };  // ˳ʱ��
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	static int initCode[8] = { 7, 7, 1, 1, 3, 3, 5, 5 };
	int dADD[8]; //��ַƫ����
	BYTE *pBegin, *pCur, *pTst; //�������,��ǰ��,����
	int code, beginCode, returnCode, i;
	int N;

	// step.1-----��ʼ��----------------------------------------//
	//��ͬ�����Ӧ�ĵ�ַƫ����
	for (code = 0; code < 8; code++) dADD[code] = dy[code] * width + dx[code];
	pBegin = pBinImg + y0 * width + x0; //�������ĵ�ַ
	pCur = pBegin; //��ǰ�����õ��������
	if (isOuter) //������ʱ�ĳ�ʼ��
	{
		*(pCur - 1) = 1;  //����Ǳ�����,���Ϊ�Ҷ�ֵ1
		code = 7; //��ʼ��Ϊ7
	}
	else //������ʱ�ĳ�ʼ��
	{
		*(pCur + 1) = 1; //�Ҳ��Ǳ�����,���Ϊ�Ҷ�ֵ1
		code = 3; //��ʼ��Ϊ3
	}
	beginCode = initCode[code]; //�������ĵ�һ������
	// step.2-----��������--------------------------------------//
	N = 0; //���������ʼ��Ϊ0
	do {
		*pCur = 254; //��������,���Ϊ�Ҷ�ֵ254
		for (i = 0, code = initCode[code]; i < 7; i++, code = (code + 1) % 8)
		{
			pTst = pCur + dADD[code]; //�õ�Ҫ����������ĵ�ַ
			if (*pTst < 2) *pTst = 1; //�Ǳ�����,���Ϊ�Ҷ�ֵ1
			else //��������
			{
				if (N < maxChainCodeNum) pChainCode[N++] = code; //��������
				if (pTst == pBegin) //�ص����Ĵ���
				{
					//�ҳ�ʣ���������ʼ���
					returnCode = (code + 4) % 8; //ת��Ϊ�ڴ��������Ѿ�����������
					for (i = 0, code = beginCode; i < 7; i++, code = (code + 1) % 8)
					{
						if (code == returnCode)
						{
							i++; //ʣ���������ʼ���
							code = (code + 1) % 8; //ʣ���������ʼֵ
							break;
						}
					}
					//���ʣ������
					for (; i < 7; i++, code = (code + 1) % 8)
					{
						pTst = pBegin + dADD[code];
						if (*pTst < 2) *pTst = 1; //�Ǳ�����,���Ϊ�Ҷ�ֵ1
						else
						{   //��������
							if (N < maxChainCodeNum) pChainCode[N++] = code;
							break; //����㿪ʼ,�ҵ����µ�������pTst
						}
					}
				}
				break; //�ҵ����µ�������pTst
			}
		}
		pCur = pTst; //��ǰ���ƶ����µ�������pTst
	} while (i < 7); //�ҵ�������ʱһ����i<7
	// step.3-----����-----------------------------------------//
	return N; //�����������
}
//������
void DrawContour(BYTE *pGryImg, int width, int x, int y, BYTE *pCode, int N, BYTE color)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int i, dADD[8];
	BYTE *pCur;

	*(pGryImg + y * width + x) = color; // Ϊ���ܹ������պϵ�����
	for (i = 0; i < 8; i++) dADD[i] = dy[i] * width + dx[i];
	for (pCur = pGryImg + y * width + x, i = 0; i < N; i++)
	{
		pCur += dADD[pCode[i]];
		*pCur = color;
	}
	return;
}
//��Ӿ���
void ContourRect(int x0, int y0, BYTE *pCode, int N, int *x1, int *x2, int *y1, int *y2)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int i;

	*y1 = *y2 = y0;
	*x1 = *x2 = x0;
	for (i = 0; i < N - 1; i++)
	{
		x0 = x0 + dx[pCode[i]];
		y0 = y0 + dy[pCode[i]];

		if (x0 > *x2) *x2 = x0;
		else if (x0 < *x1) *x1 = x0;

		if (y0 > *y2) *y2 = y0;
		else if (y0 < *y1) *y1 = y0;
	}
	return;
}
//�ܳ�
double ContourPerimeter(BYTE *pChainCode, int n)
{
	int no, ne, i;

	no = 0;
	for (i = 0; i < n; i++)
	{
		no += pChainCode[i] & 0x01; //���������
	}
	ne = (n - no); //ż�������
	return ne + no * sqrt(2.0);
}
//���
double ContourArea(BYTE *pChainCode, int n)
{
	//static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	//static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	//int i, yi, ci;
	//double S = 0.0;

	//
	//yi = 0; //����ٶ�һ����ֵ����,��Ϊ���������޹�
	//for (i = 0; i < n; i++)
	//{
	//	ci = pChainCode[i];
	//	// ������Ըĳ���������һ����Ҫdouble
	//	S += (yi + dy[ci] / 2.0)*dx[ci];
	//	yi += dy[ci]; //��һ�������������
	//}

	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy2[8] = { 0,-2,-2,-2, 0, 2, 2, 2 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int i, yi, ci;
	int S = 0;

	yi = 0; //����ٶ�һ����ֵ����,��Ϊ���������޹�
	for (i = 0; i < n; i++)
	{
		ci = pChainCode[i];
		// ������Ըĳ���������һ����Ҫdouble
		S += (yi + dy[ci])*dx[ci];
		yi += dy2[ci]; //��һ�������������
	}

	return fabs(S / 2.0); //��������ʱ������ʱΪ��,˳ʱ������ʱΪ��,����ȡ����ֵ
}
//������Χ�����ظ���
int ContourPixels(BYTE *pChainCode, int n)
{
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	// ö��ÿһ�����
	static int typeLUT[8][8] = { 1,1,1,3,3,2,2,1,//0
								 1,1,1,3,3,3,2,1,//1
								 2,2,2,0,0,0,0,2,//2
								 2,2,2,0,0,0,0,3,//3
								 3,2,2,0,0,0,0,3,//4
								 3,3,2,0,0,0,0,3,//5
								 1,1,1,2,2,2,2,1,//6
								 1,1,1,3,2,2,2,1 //7
	};
	int i, ci_1, ci, type, yi;
	int num = 0;

	num = 0;
	yi = 0; //����ٶ�һ����ֵ����,��Ϊ���������޹�
	ci_1 = pChainCode[n - 1]; //���Ľ�������(����ĩβ������)
	for (i = 0; i < n; i++)
	{
		ci = pChainCode[i]; //��������
		type = typeLUT[ci_1][ci]; //����������
		if (type == 0) num -= yi;
		else if (type == 1) num += yi + 1;
		else if (type == 3) num += 1;
		yi += dy[ci]; //��һ�������������
		ci_1 = ci; //��һ��������Ľ�������
	}
	return abs(num); //��������ʱ������ʱΪ��,˳ʱ������ʱΪ��,����ȡ����ֵ
}
//�����Ƿ�������Χ
bool isPixelInContour(int x0, int y0, BYTE *pChainCode, int n, int x, int y)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int DY[8][8] = { 0 ,1 ,1 ,1 ,1 ,0 ,0 ,0,
					 0 ,1 ,1 ,1 ,1 ,0 ,0 ,0,
					 0 ,1 ,1 ,1 ,1 ,0 ,0 ,0,
					 0 ,1 ,1 ,1 ,1 ,0 ,0 ,0,
					-1, 0 ,0 ,0 ,0,-1,-1,-1,
					-1, 0 ,0 ,0 ,0,-1,-1,-1,
					-1, 0 ,0 ,0 ,0,-1,-1,-1,
					-1, 0 ,0 ,0 ,0,-1,-1,-1,
	};
	int CY[8][8] = { 0, 0, 0, 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0, 1, 0, 0,
					 0, 0, 0, 0, 0, 1, 1, 0,
					 0, 0, 0, 0, 0, 1, 1, 1,
					 1, 0, 0, 0, 0, 1, 1, 1,
					 1, 1, 0, 0, 0, 1, 1, 1,
					 1, 1, 1, 0, 0, 1, 1, 1,
					 1, 1, 1, 1, 0, 1, 1, 1,
	};
	int sum, i, V, J;
	int pre, cur;

	pre = pChainCode[n - 1];
	for (sum = 0, i = 0; i < n; i++)
	{
		cur = pChainCode[i];
		V = ((x0 - x) >= 0) && ((y0 - y) == 0);
		J = ((x0 - x) == 0) && ((y0 - y) == 0);
		sum += V * DY[pre][cur] + J * CY[pre][cur];
		//Next
		x0 = x0 + dx[cur];
		y0 = y0 + dy[cur];
		pre = cur;
	}
	//������ʱ,������������ڰ�Χ���ص�sumֵ����1,���������ص�sumֵΪ0
	//������ʱ,����������������ص�sum��ֵ����0,�����ڰ�Χ���ص�sumֵ��-1
	return (sum != 0);
}
//��������㷨
void FillContour(BYTE *pGryImg, int width, int height,//�����ͼ��
	int x0, int y0, //�������
	bool isOuter, //�Ƿ���������
	BYTE *pCode, //����
	int N, //�������
	int regionC, //�����ڲ�����ɫ
	int contourC, //��������ɫ
	int nonC //�����ڵ���ɫ
)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int dADD[8]; //��ַƫ����
	BYTE *pBegin, *pCur, *pTst; //�������,��ǰ��,����
	int inCode, outCode, i;
	int sumC, sumDy, direction;

	// step.1-----��ʼ��----------------------------------------//
	pBegin = pGryImg + y0 * width + x0; //�������ĵ�ַ
	//��ͬ�����Ӧ�ĵ�ַƫ����
	for (i = 0; i < 8; i++) dADD[i] = dy[i] * width + dx[i];
	// step.2-----���������������������仯��֮�͸���ֵ----------//
	for (sumC = 2; sumC <= 253; sumC++) //��sumC
	{
		if ((regionC >= sumC - 2) && (regionC <= sumC + 2)) continue;
		if ((nonC >= sumC - 2) && (nonC <= sumC + 2)) continue;
		break;
	}
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++) *pCur = sumC;
	// step.3-----�������������������������仯��֮��------------//
	inCode = pCode[N - 1]; //����������������
	for (i = 0, pCur = pBegin; i < N; i++)
	{
		outCode = pCode[i]; //�Ӹ����������������
		*pCur += dy[inCode] + dy[outCode]; //���صĻҶ�ֵ���Ǳ仯��֮�� 
		inCode = outCode; //��ǰ������ĳ����������һ��������Ľ�����
		pCur += dADD[outCode]; //ָ����һ��������ĵ�ַ
	}
	// step.4-----�������������յ���б��--------------------//
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++)
	{
		sumDy = *pCur;
		if ((sumDy == sumC + 1) || (sumDy == sumC + 2)) *pCur = regionC; //���Ϊ������
		else if ((sumDy == sumC - 1) || (sumDy == sumC - 2)) *pCur = nonC; //���Ϊ����յ�
	}
	// step.5-----�����������������յ�֮��������------------//
	direction = isOuter ? 1 : -1; //�������Ǵ�������,������+1;��������֮
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++)
	{
		if (*pCur == regionC) //�ҵ�һ��������
		{
			pTst = pCur;
			while (*pTst != nonC) //һֱ��䵽�յ�
			{
				*pTst = regionC;
				pTst += direction;
			}
			*pCur = nonC; //��ˮƽ���Ѿ�������,�����ظ����
		}
	}
	// step.6-----�����������ɫ���и�ֵ-------------------------//
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++) *pCur = contourC;
	// step.7-----����-----------------------------------------//
	return;
}
// �������
void getChainCode_fill(BYTE *pSrcImg, BYTE *pShowImg, BYTE *pChainCode, int width, int height)
{
	const int PixelNum = width * height, MAX_CONOUR_LENGTH = PixelNum;
	BYTE *pCur;
	int x, y;
	int N, S;
	int x1, x2, y1, y2;

	memset(pShowImg, 0, sizeof(*pShowImg) * PixelNum);
	for (y = 1, pCur = pSrcImg + width; y < height - 1; y++) //��������
	{
		pCur++; //����������
		for (x = 1; x < width - 1; x++, pCur++) //��������
		{
			if ((*pCur == 255) && (*(pCur - 1) <= 1)) //����һ��������
			{   //�������������㷨,��������������
				N = TraceContour(pSrcImg, width, height,
					x, y, //�������
					true, //��������
					pChainCode, //����������������
					MAX_CONOUR_LENGTH //����Ĵ�С
				);
				//���
				S = ContourPixels(pChainCode, N);
				//printf("\nS=%d", S);
				//��Ӿ���
				ContourRect(x, y, pChainCode, N, &x1, &x2, &y1, &y2);
				if ((x1 > 1) && (y1 > 1) && (x2 < width - 2) && (y2 < height - 2) && //ȥ��������
					(S > 200) && //ȥ����ȱ�� 
					(S < 400) //ȥ��ճ����
					)
				{
					FillContour(pShowImg, width, height,//�����ͼ��
						x, y, //�������
						true, //�Ƿ���������
						pChainCode, //����
						N, //�������
						255, //�����ڲ�����ɫ
						254, //��������ɫ
						250 //�����ڵ���ɫ
					);
				}
				else DrawContour(pShowImg, width, x, y, pChainCode, N, 253);
			}
			else if ((*pCur == 0) && (*(pCur - 1) >= 254)) //����һ��������
			{   //�������������㷨,��������������
				N = TraceContour(pSrcImg, width, height,
					x - 1, y,  //�������,ע����x-1
					false, //��������
					pChainCode, //����������������
					MAX_CONOUR_LENGTH //����Ĵ�С
				);
				//����
				DrawContour(pShowImg, width, x - 1, y, pChainCode, N, 253);
			}
		}  // x
		pCur++; //�������Ҳ��
	}  // y
}

// ����ת�Ǳ��ж�һ������������ʱ�뷽��Ļ���˳ʱ�뷽��
bool isChainCodeClockWise(BYTE *pCode, int num)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };  // ˳ʱ��
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	static int cornerLUT[][8]{
		 0, 1, 2, 3, 0,-3,-2,-1,
		-1, 0, 1, 2, 3, 0,-3,-2,
		-2,-1, 0, 1, 2, 3, 0,-3,
		-3,-2,-1, 0, 1, 2, 3, 0,
		 0,-3,-2,-1, 0, 1, 2, 3,
		 3, 0,-3,-2,-1, 0, 1, 2,
		 2, 3, 0,-3,-2,-1, 0, 1,
		 1, 2, 3, 0,-3,-2,-1, 0,
	};  // 1��ʾ45��

	int sum = 0, pre = *pCode;
	for (int i = 0; i < num; i++)
	{
		sum += cornerLUT[pre][pCode[i]];
		pre = pCode[i];
	}
	return sum > 0;
}
