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

// 邻域运算，边缘检测梯度算子4邻域8bit图像
void AEDOpGrad1_8bit(BYTE *pSrc, int width, int height, BYTE *pDst)
{
	BYTE *pCur, *pDes;
	int dx, dy;
	int x, y, i, m;
	BYTE LUT[1024];  // 局部查表

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
		*(pDes++) = 0;  // 尾列不做,边缘强度赋0
		pCur++;
	}
	memset(pDes, 0, width);  // 尾行不做,边缘强度赋0
	return;
}
// 邻域运算，边缘检测索贝尔算子4邻域8bit图像
void AEDOpSobel_8bit(BYTE *pGryImg, int width, int height, BYTE *pSbImg)
{
	BYTE *pGry, *pSb;
	int dx, dy;
	int x, y;

	memset(pSbImg, 0, width); //首行不做,边缘强度赋0
	for (y = 1, pGry = pGryImg + width, pSb = pSbImg + width; y < height - 1; y++)
	{
		*(pSb++) = 0;  //首列不做,边缘强度赋0
		pGry++;
		for (x = 1; x < width - 1; x++, pGry++)
		{
			//求dx
			dx = *(pGry - 1 - width) + (*(pGry - 1) * 2) + *(pGry - 1 + width);
			dx -= *(pGry + 1 - width) + (*(pGry + 1) * 2) + *(pGry + 1 + width);
			//求dy
			dy = *(pGry - width - 1) + (*(pGry - width) * 2) + *(pGry - width + 1);
			dy -= *(pGry + width - 1) + (*(pGry + width) * 2) + *(pGry + width + 1);
			//结果
			*(pSb++) = min(255, abs(dx) + abs(dy));
		}
		*(pSb++) = 0; //尾列不做,边缘强度赋0
		pGry++;
	}
	memset(pSb, 0, width); //尾行不做,边缘强度赋0
	return;
}


// 邻域运算，边缘检测Laplacian算子4邻域8bit图像
void AEDOpLaplacian4_8bit(BYTE *pSrc, BYTE *pTmp, int width, int height, BYTE *pDst)
{
	BYTE *pCur, *pDes;
	int grad;
	int x, y;

	memset(pTmp, 0, width);  // 首行不做,边缘强度赋0
	for (y = 1, pCur = pSrc + width, pDes = pTmp + width; y < height - 1; y++)
	{
		*(pDes++) = 0;  // 首列不做,边缘强度赋0
		pCur++;
		for (x = 1; x < width - 1; x++, pCur++)
		{
			// grad更新
			grad = -((int)*pCur) * 4;
			grad += pCur[-width];
			grad += pCur[-1];
			grad += pCur[+1];
			grad += pCur[+width];
			*(pDes++) = grad > 0;
		}
		*(pDes++) = 0; //尾列不做,边缘强度赋0
		pCur++;
	}
	memset(pDes, 0, width); //尾行不做,边缘强度赋0

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


// 邻域运算，边缘检测沈俊算子4邻域8bit图像
void AEDOpShenJun4_8bit(
	BYTE *pSrcImg, //原始灰度图像 
	BYTE *pTmpImg, //辅助图像
	int width, int height,
	double a0,
	BYTE *pDstImg
)
{
	BYTE * pCur, * pDes, * pEnd;
	int LUT[511], *A;
	int x, y, i, pre;
	// 初始化
	A = LUT + 255;
	for (A[0] = 0, i = 1; i < 256; i++)  // 向上取整
	{
		A[i] = a0 * i + 0.5;	
		A[-i] = -a0 * i - 0.5;  
	}
	// 差分
	// 横向
	for (y = 0, pCur = pSrcImg, pDes = pTmpImg; y < height; y++)
	{
		pre = *pDes++ = *pCur++;
		// 从左到右
		for (x = 1; x < width; x++)
		{
			pre = *pDes++ = pre + A[*pCur++ - pre];
		}
		// 从右到左
		pDes--;
		for (x = 1; x < width; x++)
		{
			pDes--;
			pre = *pDes = pre + A[*pDes - pre];
		}
		pDes += width;
	}
	// 纵向
	for (x = 0, pDes=pTmpImg; x < width; x++)
	{
		// 从上到下
		pDes = pTmpImg + x, pre = *pDes;
		for (y = 1; y < height; y++)
		{
			pDes += width;
			pre = *pDes = pre + A[*pDes - pre];
		}
		// 从下到上
		for (y = 1; y < height; y++)
		{
			pDes -= width;
			pre = *pDes = pre + A[*pDes - pre];
		}
	}


	// 求二阶梯度的符号
	for (pCur = pTmpImg, pEnd = pTmpImg + width * height, pDes = pSrcImg; pCur<pEnd; pDes++)
	{
		*(pCur++) = (*pCur <= *pDes);
	}

	// 求边缘
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
	//printf("求边缘ok\n");
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
		// 处理左侧
		for (x = 0, sum = 0; x < halfm; x++, sum = 0)
		{
			for (f = halfm - x, pCur = pY; f < m; f++)
			{
				sum += pFilter[f] * *pCur++;
			}
			sum >>= offset;
			*pRes++ = sum;
		}
		// 处理中间
		for (x = halfm, sum = 0; x < width - halfm; x++, sum = 0)
		{
			for (f = 0, pCur = pY + x - halfm; f < m; f++)
			{
				sum += pFilter[f] * *pCur++;
			}
			sum >>= offset;
			*pRes++ = sum;
		}
		// 处理右侧
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
	memset(pSbImg, 0, width); //首行不做,边缘强度赋0
	for (y = 1, pGry = pGryImg + width, pSb = pSbImg + width; y < height - 1; y++)
	{
		*(pSb++) = 0;  //首列不做,边缘强度赋0
		*(pDx++) = 0;
		*(pDy++) = 0;
		pGry++;
		for (x = 1; x < width - 1; x++, pGry++)
		{
			//求dx
			dx = *(pGry - 1 - width) + (*(pGry - 1) * 2) + *(pGry - 1 + width);
			dx -= *(pGry + 1 - width) + (*(pGry + 1) * 2) + *(pGry + 1 + width);
			//求dy
			dy = *(pGry - width - 1) + (*(pGry - width) * 2) + *(pGry - width + 1);
			dy -= *(pGry + width - 1) + (*(pGry + width) * 2) + *(pGry + width + 1);
			//结果
			*(pSb++) = min(255, abs(dx) + abs(dy));
			*(pDx++) = dx;
			*(pDy++) = dy;

		}
		*(pSb++) = 0; //尾列不做,边缘强度赋0
		*(pDx++) = 0;
		*(pDy++) = 0;
		pGry++;
	}
	memset(pSb, 0, width); //尾行不做,边缘强度赋0
	return;
}

// 对梯度幅值进行非极大值抑制
void GradMagThresholding(BYTE *pSrcImg, int width, int height, int *pDx, int *pDy)
{
	BYTE *pCur;
	int *pDX, *pDY;
	BYTE Cur;
	int DX, DY;
	int x, y;
	BYTE btan, binvtan;
	// 不做边缘
	for (y = 1, pCur = pSrcImg + width, pDX = pDx + width, pDY = pDy + width; y < height - 1; y++)
	{
		pCur++, pDX++, pDY++; 
		for (x = 1; x < width - 1; x++, pCur++, pDX++, pDY++)
		{
			Cur = *pCur, DX = *pDX, DY = *pDY;
			// 取周围的8个点作比较
			if (!Cur)continue; // 模为0，则继续
			if (!DX)  // 纵向梯度
			{
				if (Cur < pCur[-width] || Cur < pCur[width])*pCur = 0;
			}
			else if (!DY) // 纵向梯度
			{
				if (Cur < pCur[-1] || Cur < pCur[1])*pCur = 0;
			}
			else 
			{
				btan = DY / DX;
				if (!btan)  // abs(tan)小于1
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

	for (y = 1, pCur = pSrcImg + width; y < height - 1; y++)  // 不处理边缘
	{
		pCur++;
		for (x = 1; x < width - 1; x++)
		{
			BYTE &Cur = *pCur++;
			if (Cur > uBound)Cur = 255;  // 该点是强连接点
			else if (Cur < lBound)Cur = 0;  // 该点无连接
			else  // 该点时弱连接点
			{
				// 如果周围有强连接点，则弱->强
				if (pCur[-width - 1] == 255 || pCur[-width] == 255 || pCur[-width + 1] == 255 ||
					pCur[-1] == 255 || pCur[1] > uBound ||
					pCur[width - 1] > uBound || pCur[width] > uBound || pCur[width + 1] > uBound)
				{
					Cur = 255;
				}
				else Cur = 0;  // 否则弱->无
			}
		}
		pCur++;
	}
}

void AEDOpCanny_8bit(
	BYTE *pSrcImg,							// 原图像
	BYTE * pTmpImg,		
	int *dx, int *dy,						// sobel算子处理得到的梯度
	int width, int height,					// 图像宽高
	int *pFilter, double sigma, int Fsize,	// 高斯滤波器的参数
	int uBound, int lBound,					// 用双阈值算法检测和连接边缘
	BYTE * pDstImg							// 目标图像
)
{
	// 高斯平滑
	// sobel算子处理
	// 对梯度幅值进行非极大值抑制
	// 用双阈值算法检测和连接边缘


	// 初始化
	Fsize |= 1;
	getGuassFilter_8bit(sigma, Fsize, pFilter);

	// 施展高斯平滑
	AGuassFilter1d_8bit(pSrcImg, width, height, pFilter, Fsize, pDstImg);
	Transpose(pDstImg, width, height, pTmpImg);
	AGuassFilter1d_8bit(pTmpImg, height, width, pFilter, Fsize, pDstImg);
	Transpose(pDstImg, height, width, pTmpImg);
	
	// sobel算子处理
	AEDOpSobe4Canny(pTmpImg, width, height, dx, dy, pDstImg);

	// 对梯度幅值进行非极大值抑制
	GradMagThresholding(pDstImg, width, height, dx, dy);

	// 用双阈值算法检测和连接边缘
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

	for (x = 0; x < width; x++)*pDes++ = *pCur++;  // 边缘处理
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
	for (x = 0; x < width; x++)*pDes++ = *pCur++;  // 边缘处理

}

//_________hw5____________________________________

//得到直方图
void getHist8b(BYTE *pSrcImg, int pixleNum, int *hist)
{
	BYTE *pCur = pSrcImg, *pEnd = pSrcImg + pixleNum;
	memset(hist, 0, sizeof(int) * 256);

	while (pCur < pEnd)hist[*pCur++]++;
}

int getOtsuThreshold8b(int *hist, int HistLen)
{
	int ming = 0, maxg = HistLen - 1;
	// 得到灰度最小值和最大值
	for (; ming < HistLen && !hist[ming]; ming++);
	for (; maxg && !hist[maxg]; maxg--);
	if (maxg == ming)return ming;
	int maxThreshold = 0;  // 阈值，以及最值
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
	// 得到像素数目大致数量
	while (offset && !((1 << offset)&numH))offset--;
	// 枚举出最好灰度
	for (i = ming, pN = pNum; i < maxg; i++, pN++)
	{	
		if (!hist[i])continue;
		sumH -= *pN;
		numH -= hist[i];
		sumL += *pN; 
		numL += hist[i];
		// 如果longlong超过1e15则,转化为double会有精度丢失
		// 除以一个pixelnum保证double精度，对于图片总像素大于1e7的情况
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

	// step.1-----确定搜索范围:最小值----------------------------//
	gmin = 0;
	while (histogram[gmin] == 0) ++gmin;
	// step.2-----确定搜索范围:最大值----------------------------//
	gmax = nSize - 1;
	while (histogram[gmax] == 0) --gmax;
	// step.3-----搜索最佳阈值-----------------------------------//
	if (gmin == gmax) return gmin; //不满足二类分布
	max = 0;
	thre = 0;
	//初始化u1
	s1 = n1 = 0;
	//初始化u2
	for (s2 = n2 = 0, i = gmin; i <= gmax; i++)
	{
		s2 += histogram[i] * i;
		n2 += histogram[i];
	}
	//搜索
	for (i = gmin, n = n2; i < gmax; i++)
	{   //加速
		if (!histogram[i]) continue;
		//更新s1s2
		s1 += histogram[i] * i;
		s2 -= histogram[i] * i;
		//更新n1n2
		n1 += histogram[i];
		n2 -= histogram[i];
		//评价函数
		dist = (s1*1.0 / n1 - s2 * 1.0 / n2);
		f = dist * dist*(n1*1.0 / n)*(n2*1.0 / n);
		if (f > max)
		{
			max = f;
			thre = i;
		}
	}
	// step.4-----返回-------------------------------------------//
	return thre + 1; //二值化时是用>=thre,所以要+1
}
// 二值化
void PGry2Bin(BYTE *pSrcImg, int pixelNum, int threshold, BYTE *pDstImg)
{
	BYTE *pCur = pSrcImg, *pEnd = pSrcImg + pixelNum, *pDst = pDstImg;

	while (pCur < pEnd) *pDst++ = (*pCur++ < threshold) - 1;  // 1 - 1 = 0; 0 - 1 = -1(0xffffffff->255)，省去了乘法;

}


void GuassianBlur(BYTE *pSrcImg, BYTE *pTmpImg, int width, int height, double sigma, int Fsize, int *pFilter, BYTE *pDstImg)
{
	Fsize |= 1;
	getGuassFilter_8bit(sigma, Fsize, pFilter);

	// 施展高斯平滑
	AGuassFilter1d_8bit(pSrcImg, width, height, pFilter, Fsize, pTmpImg);
	Transpose(pTmpImg, width, height, pDstImg);
	AGuassFilter1d_8bit(pDstImg, height, width, pFilter, Fsize, pTmpImg);
	Transpose(pTmpImg, height, width, pDstImg);
}
// 一个简单的最小值滤波
void AMinFilter(BYTE * pSrcImg, int width, int height, int fw, int fh, BYTE * pDstImg)
{
	int x, y, tx;
	BYTE * ty, minPix;
	BYTE *pCur = pSrcImg, *pDst = pDstImg;
	int halfx = fw / 2, halfy = fh / 2;
	fw |= 1, fh |= 1;

	// 特殊处理上面
	for (y = 0; y < halfy * width; y++)
			*pDst++ = *pCur++;
	for (y = halfy; y < height - halfy; y++)
	{
		for (x = 0; x < halfx; x++)  // 特殊处理左边
			*pDst++ = *pCur++;
		for (; x < width - halfx; x++, pCur++)  // 正常处理
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
		for (; x < width; x++)  // 特殊处理右边
			*pDst++ = *pCur++;
	}  // y
	// 特殊处理下面
	for (y = 0; y < halfy * width; y++)
		*pDst++ = *pCur++;
}
// 一个简单的最大值滤波
void AMaxFilter(BYTE * pSrcImg, int width, int height, int fw, int fh, BYTE * pDstImg)
{
	int x, y, tx;
	BYTE * ty, maxPix;
	BYTE *pCur = pSrcImg, *pDst = pDstImg;
	int halfx = fw / 2, halfy = fh / 2;
	fw |= 1, fh |= 1;

	// 特殊处理上面
	for (y = 0; y < halfy * width; y++)
		*pDst++ = *pCur++;
	for (y = halfy; y < height - halfy; y++)
	{
		for (x = 0; x < halfx; x++)  // 特殊处理左边
			*pDst++ = *pCur++;
		for (; x < width - halfx; x++, pCur++)  // 正常处理
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
		for (; x < width; x++)  // 特殊处理右边
			*pDst++ = *pCur++;
	}  // y
	// 特殊处理下面
	for (y = 0; y < halfy * width; y++)
		*pDst++ = *pCur++;
}
// 直方图k均值聚类
void Hist_kMeans(int *hist, int len, int class_num, double *Center, double bound, int max_iter, int *iniCenter)
{
	int i, g, iter;
	int  num[256];
	double sum[256];
	int hmax = len - 1, hmin = 0;
	double dist, dist1, tc;
	bool boundFlag;

	// 初始化
	for (; hmin < len && !hist[hmin]; hmin++);
	for (; hmax > -1 && !hist[hmax]; hmax--);
	if (hmax - hmin < class_num) return;
	if (iniCenter)
	{
		// 假设这个初始化的中心不会在hmin，hmax外面
		for (i = 0; i < class_num; i++)Center[i] = iniCenter[i];
	}
	else
	{
		int step = (hmax - hmin) / (class_num - 1);
		for (i = 0, g = hmin; i < class_num; i++, g += step) Center[i] = g;
	}

	// 执行
	for (iter = 0; iter < max_iter; iter++)
	{
		// 分类
		g = 0; // g指向前一个灰度值所在的类别
		for (i = 0; i < class_num; i++)
		{
			sum[i] = 0;
			num[i] = 0;
		}
		for (i = hmin; i <= hmax; i++)
		{
			if (!hist[i])continue;
			// 得到最合适的中心
			dist = abs(i - Center[g]);
			while (g < class_num - 1 && (dist1 = abs(i - Center[g + 1])) < dist)
			{
				dist = dist1;
				g++;
			}  // g
			sum[g] += i * hist[i];
			num[g] += hist[i];
		}  // i

		// 得到新的中心
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
	// 霍夫变换
	for (theta = 0; theta < 180; theta++) //步长为1度
	{
		cosV = (int)(cos(theta * PI / 180) * (1 << 13)); //放大8192倍
		sinV = (int)(sin(theta * PI / 180) * (1 << 13));
		pCenter = pCount + (maxThro * 2)*theta + maxThro; //加上偏移maxThro
		for (i = 0; i < N; i++)
		{
			thro = ((int(X[i] * cosV + Y[i] * sinV)) >> 13); //缩小8192倍,thro的步长为1
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



// 轮廓跟踪算法
int TraceContour
(
	BYTE *pBinImg, int width, int height,//二值图像
	int x0, int y0, //轮廓起点
	bool isOuter, //是否是外轮廓
	BYTE *pChainCode, //外部申请好的一个内存空间，用来存放链码
	int maxChainCodeNum //可以存放的最大链码个数
)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };  // 顺时针
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	static int initCode[8] = { 7, 7, 1, 1, 3, 3, 5, 5 };
	int dADD[8]; //地址偏移量
	BYTE *pBegin, *pCur, *pTst; //轮廓起点,当前点,检查点
	int code, beginCode, returnCode, i;
	int N;

	// step.1-----初始化----------------------------------------//
	//不同链码对应的地址偏移量
	for (code = 0; code < 8; code++) dADD[code] = dy[code] * width + dx[code];
	pBegin = pBinImg + y0 * width + x0; //轮廓起点的地址
	pCur = pBegin; //当前点设置到轮廓起点
	if (isOuter) //外轮廓时的初始化
	{
		*(pCur - 1) = 1;  //左侧是背景点,标记为灰度值1
		code = 7; //初始化为7
	}
	else //内轮廓时的初始化
	{
		*(pCur + 1) = 1; //右侧是背景点,标记为灰度值1
		code = 3; //初始化为3
	}
	beginCode = initCode[code]; //从起点检查的第一个链码
	// step.2-----轮廓跟踪--------------------------------------//
	N = 0; //链码个数初始化为0
	do {
		*pCur = 254; //是轮廓点,标记为灰度值254
		for (i = 0, code = initCode[code]; i < 7; i++, code = (code + 1) % 8)
		{
			pTst = pCur + dADD[code]; //得到要检查的轮廓点的地址
			if (*pTst < 2) *pTst = 1; //是背景点,标记为灰度值1
			else //是轮廓点
			{
				if (N < maxChainCodeNum) pChainCode[N++] = code; //保存链码
				if (pTst == pBegin) //回到起点的处理
				{
					//找出剩余链码的起始序号
					returnCode = (code + 4) % 8; //转换为于从起点出发已经检查过的链码
					for (i = 0, code = beginCode; i < 7; i++, code = (code + 1) % 8)
					{
						if (code == returnCode)
						{
							i++; //剩余链码的起始序号
							code = (code + 1) % 8; //剩余链码的起始值
							break;
						}
					}
					//检查剩余链码
					for (; i < 7; i++, code = (code + 1) % 8)
					{
						pTst = pBegin + dADD[code];
						if (*pTst < 2) *pTst = 1; //是背景点,标记为灰度值1
						else
						{   //保存链码
							if (N < maxChainCodeNum) pChainCode[N++] = code;
							break; //从起点开始,找到了新的轮廓点pTst
						}
					}
				}
				break; //找到了新的轮廓点pTst
			}
		}
		pCur = pTst; //当前点移动到新的轮廓点pTst
	} while (i < 7); //找到轮廓点时一定有i<7
	// step.3-----结束-----------------------------------------//
	return N; //返回链码个数
}
//画轮廓
void DrawContour(BYTE *pGryImg, int width, int x, int y, BYTE *pCode, int N, BYTE color)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int i, dADD[8];
	BYTE *pCur;

	*(pGryImg + y * width + x) = color; // 为了能够处理不闭合的轮廓
	for (i = 0; i < 8; i++) dADD[i] = dy[i] * width + dx[i];
	for (pCur = pGryImg + y * width + x, i = 0; i < N; i++)
	{
		pCur += dADD[pCode[i]];
		*pCur = color;
	}
	return;
}
//外接矩形
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
//周长
double ContourPerimeter(BYTE *pChainCode, int n)
{
	int no, ne, i;

	no = 0;
	for (i = 0; i < n; i++)
	{
		no += pChainCode[i] & 0x01; //奇数码个数
	}
	ne = (n - no); //偶数码个数
	return ne + no * sqrt(2.0);
}
//面积
double ContourArea(BYTE *pChainCode, int n)
{
	//static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	//static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	//int i, yi, ci;
	//double S = 0.0;

	//
	//yi = 0; //随意假定一个初值即可,因为面积与起点无关
	//for (i = 0; i < n; i++)
	//{
	//	ci = pChainCode[i];
	//	// 这里可以改成整数，不一定需要double
	//	S += (yi + dy[ci] / 2.0)*dx[ci];
	//	yi += dy[ci]; //下一个轮廓点的坐标
	//}

	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy2[8] = { 0,-2,-2,-2, 0, 2, 2, 2 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int i, yi, ci;
	int S = 0;

	yi = 0; //随意假定一个初值即可,因为面积与起点无关
	for (i = 0; i < n; i++)
	{
		ci = pChainCode[i];
		// 这里可以改成整数，不一定需要double
		S += (yi + dy[ci])*dx[ci];
		yi += dy2[ci]; //下一个轮廓点的坐标
	}

	return fabs(S / 2.0); //在轮廓逆时针走向时为正,顺时针走向时为负,所以取绝对值
}
//轮廓包围的像素个数
int ContourPixels(BYTE *pChainCode, int n)
{
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	// 枚举每一种情况
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
	yi = 0; //随意假定一个初值即可,因为面积与起点无关
	ci_1 = pChainCode[n - 1]; //起点的进入链码(即最末尾的链码)
	for (i = 0; i < n; i++)
	{
		ci = pChainCode[i]; //出发链码
		type = typeLUT[ci_1][ci]; //轮廓点类型
		if (type == 0) num -= yi;
		else if (type == 1) num += yi + 1;
		else if (type == 3) num += 1;
		yi += dy[ci]; //下一个轮廓点的坐标
		ci_1 = ci; //下一个轮廓点的进入链码
	}
	return abs(num); //在轮廓逆时针走向时为正,顺时针走向时为负,所以取绝对值
}
//像素是否被轮廓包围
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
	//外轮廓时,轮廓点和轮廓内包围像素的sum值都是1,轮廓外像素的sum值为0
	//内轮廓时,轮廓点和轮廓外像素的sum的值都是0,轮廓内包围像素的sum值是-1
	return (sum != 0);
}
//轮廓填充算法
void FillContour(BYTE *pGryImg, int width, int height,//待填充图像
	int x0, int y0, //轮廓起点
	bool isOuter, //是否是外轮廓
	BYTE *pCode, //链码
	int N, //链码个数
	int regionC, //区域内部的颜色
	int contourC, //轮廓点颜色
	int nonC //不存在的颜色
)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
	static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1 };
	int dADD[8]; //地址偏移量
	BYTE *pBegin, *pCur, *pTst; //轮廓起点,当前点,填充点
	int inCode, outCode, i;
	int sumC, sumDy, direction;

	// step.1-----初始化----------------------------------------//
	pBegin = pGryImg + y0 * width + x0; //轮廓起点的地址
	//不同链码对应的地址偏移量
	for (i = 0; i < 8; i++) dADD[i] = dy[i] * width + dx[i];
	// step.2-----轮廓点的所有链码纵坐标变化量之和赋初值----------//
	for (sumC = 2; sumC <= 253; sumC++) //求sumC
	{
		if ((regionC >= sumC - 2) && (regionC <= sumC + 2)) continue;
		if ((nonC >= sumC - 2) && (nonC <= sumC + 2)) continue;
		break;
	}
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++) *pCur = sumC;
	// step.3-----计算轮廓点的所有链码纵坐标变化量之和------------//
	inCode = pCode[N - 1]; //进入轮廓起点的链码
	for (i = 0, pCur = pBegin; i < N; i++)
	{
		outCode = pCode[i]; //从该轮廓点出发的链码
		*pCur += dy[inCode] + dy[outCode]; //像素的灰度值就是变化量之和 
		inCode = outCode; //当前轮廓点的出发码就是下一个轮廓点的进入码
		pCur += dADD[outCode]; //指向下一个轮廓点的地址
	}
	// step.4-----对填充起点和填充终点进行标记--------------------//
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++)
	{
		sumDy = *pCur;
		if ((sumDy == sumC + 1) || (sumDy == sumC + 2)) *pCur = regionC; //标记为填充起点
		else if ((sumDy == sumC - 1) || (sumDy == sumC - 2)) *pCur = nonC; //标记为填充终点
	}
	// step.5-----按行在填充起点和填充终点之间进行填充------------//
	direction = isOuter ? 1 : -1; //外轮廓是从左向右,所以是+1;内轮廓反之
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++)
	{
		if (*pCur == regionC) //找到一个填充起点
		{
			pTst = pCur;
			while (*pTst != nonC) //一直填充到终点
			{
				*pTst = regionC;
				pTst += direction;
			}
			*pCur = nonC; //该水平段已经填充过了,避免重复填充
		}
	}
	// step.6-----对轮廓点的颜色进行赋值-------------------------//
	for (i = 0, pCur = pBegin; i < N; pCur += dADD[pCode[i]], i++) *pCur = contourC;
	// step.7-----结束-----------------------------------------//
	return;
}
// 链码填充
void getChainCode_fill(BYTE *pSrcImg, BYTE *pShowImg, BYTE *pChainCode, int width, int height)
{
	const int PixelNum = width * height, MAX_CONOUR_LENGTH = PixelNum;
	BYTE *pCur;
	int x, y;
	int N, S;
	int x1, x2, y1, y2;

	memset(pShowImg, 0, sizeof(*pShowImg) * PixelNum);
	for (y = 1, pCur = pSrcImg + width; y < height - 1; y++) //从上向下
	{
		pCur++; //跳过最左侧点
		for (x = 1; x < width - 1; x++, pCur++) //从左向右
		{
			if ((*pCur == 255) && (*(pCur - 1) <= 1)) //发现一条外轮廓
			{   //调用轮廓跟踪算法,进行外轮廓跟踪
				N = TraceContour(pSrcImg, width, height,
					x, y, //轮廓起点
					true, //是外轮廓
					pChainCode, //用来存放链码的数组
					MAX_CONOUR_LENGTH //数组的大小
				);
				//面积
				S = ContourPixels(pChainCode, N);
				//printf("\nS=%d", S);
				//外接矩形
				ContourRect(x, y, pChainCode, N, &x1, &x2, &y1, &y2);
				if ((x1 > 1) && (y1 > 1) && (x2 < width - 2) && (y2 < height - 2) && //去掉贴边者
					(S > 200) && //去掉残缺者 
					(S < 400) //去掉粘连者
					)
				{
					FillContour(pShowImg, width, height,//待填充图像
						x, y, //轮廓起点
						true, //是否是外轮廓
						pChainCode, //链码
						N, //链码个数
						255, //区域内部的颜色
						254, //轮廓点颜色
						250 //不存在的颜色
					);
				}
				else DrawContour(pShowImg, width, x, y, pChainCode, N, 253);
			}
			else if ((*pCur == 0) && (*(pCur - 1) >= 254)) //发现一条内轮廓
			{   //调用轮廓跟踪算法,进行内轮廓跟踪
				N = TraceContour(pSrcImg, width, height,
					x - 1, y,  //轮廓起点,注意是x-1
					false, //是内轮廓
					pChainCode, //用来存放链码的数组
					MAX_CONOUR_LENGTH //数组的大小
				);
				//调试
				DrawContour(pShowImg, width, x - 1, y, pChainCode, N, 253);
			}
		}  // x
		pCur++; //跳过最右侧点
	}  // y
}

// 利用转角表判定一条轮廓线是逆时针方向的还是顺时针方向
bool isChainCodeClockWise(BYTE *pCode, int num)
{
	static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };  // 顺时针
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
	};  // 1表示45度

	int sum = 0, pre = *pCode;
	for (int i = 0; i < num; i++)
	{
		sum += cornerLUT[pre][pCode[i]];
		pre = pCode[i];
	}
	return sum > 0;
}
