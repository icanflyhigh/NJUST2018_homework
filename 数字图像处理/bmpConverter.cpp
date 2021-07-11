#include "bmpConverter.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <nmmintrin.h>
#include <ctime>
#include "Img.h"
#include "switcher.h"
#include "BMPCore.h"
#define BTYE BYTE
using namespace std;
const float fEPS = 1e-6;
const double dEPS = 1e-10;
const double PI = 3.1415926535;
const double EPS = 1e-9;
const int INF = 1e9 + 7;
void setImageBoundary(BYTE *pBinImg, int width, int height, BYTE color);



bmpConverter::bmpConverter(const char * orgfile)
{
	if (!BmpFile2Img(orgfile))
	{
		puts("文件读取失败");
	}
	else
	{
		printf("已读取文件 %s\n", orgfile);
	}
}

bmpConverter::bmpConverter():Img()
{
}

// 灰度图像反转
void bmpConverter::InvertImg()
{
	
	//BYTE *pCur, *pEnd = pImg + width * height * channel;
	//for (pCur = pImg; pCur < pEnd; pCur++) *pCur = ~ *pCur;
	int sum = width * height * channel;
	int res = sum - sum / 4 * 4;
	int *pCur= (int *)pImg, *pEnd = (int *)(pImg + sum - res);
	while(pCur < pEnd) *(pCur++) = ~*pCur;
	BYTE * pRes = (BYTE *)pEnd;
	// 将不到一个4个字节的数据逐字节取反
	while(res--)*(pRes++) = ~*(pRes);
	return;
}

// 读取bmp
bool bmpConverter::BmpFile2Img(const char * DstFile)
{
	
	delete pImg;
	pImg = nullptr;
	//BmpHeader记录的真实的宽度,
	//读出的是真实的宽度,去掉了4对齐的扩展.
	FILE *fp;
	unsigned int size;
	int Suc = 1, w, h;
	int k, extend, tmp[4], err_type;

	// Open File
	width = height = 0;
	if (err_type = fopen_s(&fp, DstFile, "rb"))
	{
		printf("error type: %d \n", err_type);
		return false;
	}
	// Read Struct Info
	if (fread((void *)&FileHeader, 1, sizeof(FileHeader), fp) != sizeof(FileHeader)) Suc = 0;
	if (fread((void *)&BmpHeader, 1, sizeof(BmpHeader), fp) != sizeof(BmpHeader)) Suc = 0;
	if (!Suc ||
		(FileHeader.bfOffBits < sizeof(FileHeader) + sizeof(BmpHeader))
		)
	{
		fclose(fp);
		return false;
	}
	// Read Image Data
	width = w = BmpHeader.biWidth;
	height = h = BmpHeader.biHeight;
	if (BmpHeader.biBitCount < 16)channel = 1;
	else channel = BmpHeader.biBitCount / 8;
#ifdef DEBUG
	printf("读取%d通道图像, 每个像素比特数%d\n", channel, int(BmpHeader.biBitCount));
#endif // DEBUG
	// 无法处理1,4bit的问题
	w = w * channel; // 真实的比特数
	extend = (w + 3) / 4 * 4 - w;
	if (!(size = BmpHeader.biSizeImage)) size = width * height * channel;
	
	fseek(fp, FileHeader.bfOffBits, SEEK_SET);
	if ((pImg = new BYTE[size]) != nullptr)
	{
		for (int i = 1; i <= h; i++)  // 0,1,2,3,4(5): 400-499
		{
			if ((int)fread(pImg + (h - i)*w, sizeof(BYTE), w, fp) != w)
			{
				fclose(fp);
				delete pImg;
				pImg = NULL;
				return false;
			}
			//读掉扩充的数据
			if (fread(&tmp, 1, extend, fp) != extend)
			{
				fclose(fp);
				delete pImg;
				pImg = nullptr;
				return false;

			}
		}
	}
	fclose(fp);
	return true;
}
// 保存bmp
bool bmpConverter::Img2Bmp(const char * DstFile, int bitCnt, char RGB_MOD)
{
	if (pImg == nullptr)
	{
		// 未读取图像
		puts("保存失败，未读取图像");
		return false;
	}
	bool ret = false;
	switch (bitCnt)
	{
	case 0:
		ret = Img2Bmp(DstFile, BmpHeader.biBitCount? BmpHeader.biBitCount:-1, RGB_MOD);
		break;
	case 8:
		ret = Img28bitBmp(DstFile, RGB_MOD);
		break;
	case 24:
		ret = Img224bitBmp(DstFile);
		break;
	default:
		puts("无法处理的位宽");
		break;
	}
	return ret;
}

bmpConverter::~bmpConverter()
{
}

bmpConverter::bmpConverter(Img a):
	Img(a)
{
}

bmpConverter::bmpConverter(bmpConverter & tb):
	Img(tb),
	FileHeader(tb.FileHeader), BmpHeader(tb.BmpHeader)
{
}

bmpConverter::bmpConverter(bmpConverter &&tb):
Img(tb),
FileHeader(tb.FileHeader), BmpHeader(tb.BmpHeader)
{
}
// 得到调色板
void getBord(BYTE * board, char mod = 0)
{
	int tr = -1, tb = -1, tg = -1, i, p = 0;

	if (mod == 'r') tb = 0, tg = 0;
	else if (mod == 'g') tr = 0, tb = 0;
	else if (mod == 'b') tr = 0, tg = 0;
	if (mod != 'k')
	{
		for (i = 0; i < 256; i++)
		{
			
			board[p++] = i & tb;
			board[p++] = i & tg;
			board[p++] = i & tr; //blue,green,red, alpha
			board[p++] = 0;
			//if (fwrite((void *)p, 1, 4, fp) != 4) { Suc = false; break; }
		}
	}
	else
	{
		for (i = 0; i < 256; i++)
		{

			if (i != 254)
			{
				board[p++] = i;//blue,green,red, alpha
				board[p++] = i;
			}
			else
			{
				board[p++] = 0;
				board[p++] = 0;
			}
			board[p++] = i;
			board[p++] = i;
			//if (fwrite((void *)p, 1, 4, fp) != 4) { Suc = false; break; }
		}
	}


}
// 8bit 转bmp
bool bmpConverter::Img28bitBmp(const char * DstFile, char mod)
{
#ifdef DEBUG
	printf("正在保存8bit 图像 %s\n", DstFile);
#endif // DEBUG
	//BYTE * temp_save=nullptr;
	if (channel == 3)
	{
		RGB2Gry();
	}

	//BmpHeader记录的真实的宽度
	//当每行字节个数不是4的倍数时,需要进行4对齐
	FILE * fp;
	int extend;
	bool Suc = true;
	BYTE *pCur;

	// Open File
	if (fopen_s(&fp, DstFile, "w+b")) { return false; }
	// Fill the FileHeader
	FileHeader.bfType = ((WORD)('M' << 8) | 'B');
	FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * 4L;
	FileHeader.bfSize = FileHeader.bfOffBits + width * height;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	if (fwrite((void *)&FileHeader, 1, sizeof(FileHeader), fp) != sizeof(FileHeader)) Suc = false;
	// Fill the ImgHeader
	BmpHeader.biSize = 40;
	BmpHeader.biWidth = width;
	BmpHeader.biHeight = height;
	BmpHeader.biPlanes = 1;
	BmpHeader.biBitCount = 8;
	BmpHeader.biCompression = 0;
	BmpHeader.biSizeImage = 0;
	BmpHeader.biXPelsPerMeter = 0;
	BmpHeader.biYPelsPerMeter = 0;
	BmpHeader.biClrUsed = 0;
	BmpHeader.biClrImportant = 0;
	if (fwrite((void *)&BmpHeader, 1, sizeof(BmpHeader), fp) != sizeof(BmpHeader)) 
		Suc = false;
	// write Pallete

	// 根据mod预设颜色
	BYTE board[256 * 4];
	getBord(board, mod);
	if (fwrite((void *)board, 1, 4 * 256, fp) != 4 * 256) { Suc = false;}
	// write image data
	extend = (width + 3) / 4 * 4 - width; //每行字节个数需要按4的倍数对齐
	if (extend == 0)
	{
		for (pCur = pImg + (height - 1)*width; pCur >= pImg; pCur -= width)
		{
			if (fwrite((void *)pCur, 1, width, fp) != (unsigned int)width) Suc = false; //真实的数据
		}
	}
	else
	{
		for (pCur = pImg + (height - 1)*width; pCur >= pImg; pCur -= width)
		{
			if (fwrite((void *)pCur, 1, width, fp) != (unsigned int)width) Suc = false; //真实的数据
			//每行按4字节对齐进行扩充
			if (fwrite((void *)(pCur + width - 1), 1, extend, fp) != extend) Suc = false;
			
		}
	}
	// return;
	fclose(fp);
	if (Suc)printf("%s 保存成功\n", DstFile);
	return Suc;
}
// 24bit转bmp
bool bmpConverter::Img224bitBmp(const char * DstFile)
{
	//BmpHeader记录的真实的宽度
//当每行字节个数不是4的倍数时,需要进行4对齐
	FILE *fp;
	bool Suc = true;
	int i, extend, w = channel * width;
	BYTE *pCur;
	// Open File
	if (fopen_s(&fp, DstFile, "w+b")) { return false; }
	// Fill the FileHeader
	FileHeader.bfType = ((WORD)('M' << 8) | 'B');
	FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	FileHeader.bfSize = FileHeader.bfOffBits + width * height * 3L;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	if (fwrite((void *)&FileHeader, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER)) Suc = false;
	// Fill the ImgHeader
	BmpHeader.biSize = 40;
	BmpHeader.biPlanes = 1;
	BmpHeader.biCompression = 0;
	BmpHeader.biSizeImage = width * height * channel;
	BmpHeader.biXPelsPerMeter = 0;
	BmpHeader.biYPelsPerMeter = 0;
	BmpHeader.biClrUsed = 0;
	BmpHeader.biClrImportant = 0;
	if (fwrite((void *)&BmpHeader, 1, sizeof(BITMAPINFOHEADER), fp) != sizeof(BITMAPINFOHEADER)) Suc = false;
	// write image data

	extend = ((w + 3) / 4 * 4) - w; //每行字节个数需要按4的倍数对齐
	if (!extend)
	{
		for (pCur = pImg + (height - 1) * w; pCur >= pImg; pCur -= w)
		{
			if (fwrite((void *)pCur, 1, w, fp) != (unsigned int)(w)) Suc = false; //真实的数据
		}
	}
	else
	{
		for (pCur = pImg + (height - 1) * w; pCur >= pImg; pCur -= w)
		{
			if (fwrite((void *)pCur, 1, w, fp) != (unsigned int)(w)) Suc = false; //真实的数据
			 //扩充的数据
			if (fwrite((void *)(pCur + w- 3), 1, extend, fp) != extend) Suc = false;
		}
	}
	// return;
	fclose(fp);
	if (Suc)printf("%s 保存成功\n", DstFile);
	return Suc;
}


//________homework2的代码__________________________

// 14bit直方图归一化
void bmpConverter::PHistogramEqualize14bit(short * pRawImg)
{
	int hist[1 << 14], sum = width * height, A = 0;
	pImg = new BYTE[sum];
	BYTE LUT[1 << 14], * pCur = pImg;
	// 得到直方图
	short * spCur = pRawImg, *spDes = pRawImg + sum;
	for (int i = 0; i < (1 << 14); i++) hist[i] = 0;
	while (spCur < spDes) hist[*(spCur++)]++;
	for (int i = 0; i < (1 << 14); i++)LUT[i] = 255 * (A += hist[i]) / sum;
	for (spCur = pRawImg; spCur < spDes;)*(pCur++) = LUT[*(spCur++)];

}
// 14bit直方图归一化2
void bmpConverter::PHistogramEqualize14bit2(short * pRawImg)
{
	int hist[1 << 14], sum = width * height, A = 0;
	pImg = new BYTE[sum];
	BYTE LUT[1 << 14], *pCur = pImg;
	// 得到直方图
	short * spCur = pRawImg, *spDes = pRawImg + sum;
	for (int i = 0; i < (1 << 14); i++) hist[i] = 0;
	while (spCur < spDes) hist[*(spCur++)]++;
	int maxp = (1 << 14) - 1, minp = 0;// 求最小灰度和最大灰度
	while (!hist[minp])minp++;
	while (!hist[maxp])maxp--;
	double c = 255 / log(1 + maxp - minp + 1e-8);
#ifdef DEBUG
	printf("gmax : %d   gmin : %d   c %llf\n", maxp, minp, c);
#endif // DEBUG

	for (int i = minp; i <= maxp; i++)LUT[i] = c * log(1 + i - minp);
	for (spCur = pRawImg; spCur < spDes;)*(pCur++) = LUT[*(spCur++)];
	P2avgstd8Bit(100, 100);
}

// 读取14bit raw数据，已经写死
bool bmpConverter::read14bitRaw(const char * DstFile)
{
	delete pImg;
	pImg = nullptr;
	width = 640, height = 480, channel = 1;
	BmpHeader.biBitCount = 8;
	FILE *fp;
	int err = fopen_s(&fp, DstFile, "rb"); // "./pic/H0204IR14bit.raw"
	if (err)
	{
		printf("file open err %d \n", err);
		return false;
	}
	short * pRawImg = new short[width*height];
	fread(pRawImg, sizeof(short), width * height, fp);
	fclose(fp);
	PHistogramEqualize14bit2(pRawImg);
	return true;
}



// RGB转灰度图
void bmpConverter::RGB2Gry(bool table_chk, bool inplace)
{
	if (channel != 3 || !pImg)return;
	int sum = width * height;
	BYTE * temp_save = new BYTE[sum];
	if (!temp_save)
	{
#ifdef DEBUG
		puts("程序错误，内存申请失败");
#endif // DEBUG
		return;
	}
#ifdef DEBUG
	//cout << (int)(temp_save) <<"113" <<sizeof(temp_save)<< endl;
	////cout << (int)pImg << endl;
#endif // DEBUG
	BYTE * p1 = temp_save, *p2 = pImg - 1;
	if (table_chk)
	{
		BYTE r8[256], g8[256], b8[256];
		int r_ratio = 0.299 * 1024, g_ratio = 0.587 * 1024;
		for (int i = 0; i < 256; i++)
		{
			r8[i] = r_ratio * i >> 10;
			g8[i] = g_ratio * i >> 10;
			b8[i] = i - r8[i] - g8[i];
		}
		while (sum--)*(p1++) = (b8[*(++p2)] + g8[*(++p2)] + r8[*(++p2)]);//blue,green,red
	}
	else
	{
		while (sum--)*(p1++) = ((*(++p2)) * 0.114 + 0.587 * (*(++p2)) + 0.299 * (*(++p2)));//blue,green,red
	}
	if (inplace)
	{
		swap(temp_save, pImg);
		channel = 1;
	}

#ifdef DEBUG
	//cout << int(temp_save) << endl;
	//cout << (int)pImg << endl;
#endif // DEBUG
	delete temp_save;
	return;
}

// 线性拉伸（仿射变换）
void bmpConverter::PAffine(double k, double b)
{
	if (!pImg)return;
	BYTE LUT[256], * pCur = pImg, *pDes = pImg + width * height * channel;
	for (int i = 0; i < 256; i++)LUT[i] = (std::min)(255.0, (std::max)(0.0, k * i + b));
	while (pCur < pDes)*(pCur++) = LUT[*pCur];


}
// 8bit均值标准差规定化
void bmpConverter::P2avgstd8Bit(double mean, double stddev)
{
	if (BmpHeader.biBitCount != 8)return;
	int tot = width * height;
	int hist[256];
	getHistGram8bit(hist);
	double avg, k ,b, sdev = 0;
	if (tot > 16777210)
	{
		unsigned long long  sum = 0;
		for (int i = 0; i < 256; i++) sum += hist[i] * i;
		avg = double(sum) / tot;
	}
	else
	{
		unsigned int sum = 0; 
		for (int i = 0; i < 256; i++) sum += hist[i] * i;
		avg = double(sum) / tot;
	}
	for (int i = 0; i < 256; i++) sdev += hist[i] * (i - avg) * (i - avg);
	k = stddev / sqrt(sdev / (tot - 1));
	b = mean - avg * k;
	PAffine(k, b);
	return;
}
// 得到8bit的直方图
void bmpConverter::getHistGram8bit(int * hist)
{
	for (int i = 0; i < 256; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height;pCur < pDest;)
		hist[*(pCur++)]++;
}
// 8bit直方图标准化实现
void bmpConverter::PHistogramEqualize8bit()
{
	if (BmpHeader.biBitCount != 8)
	{ 
#ifdef DEBUG
		puts("图像位数错误");
#endif // DEBUG
		return; 
	}
	int hist[256], LUT[256], A = 0, sum = width * height;
	getHistGram8bit(hist);
	for (int i = 0, A = hist[1]; i < 256; i++)LUT[i] = 255 * (A += hist[i]) / sum;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height; pCur < pDest;)
		*(pCur++) = LUT[*pCur];

}
// 得到24bit直方图
void bmpConverter::getHistGram24bit(int * hist)
{
	for (int i = 0; i < 256*3; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, p = 0, *pDest = pImg + width * height * 3; pCur < pDest; p = (p + 1)%3)
		hist[*(pCur++) * 3 + p]++;
}
// 24bit直方图标准化实现（三个通道分别实现）
void bmpConverter::PHistogramEqualize24bit()
{
	if (BmpHeader.biBitCount != 24)
	{
#ifdef DEBUG
		puts("图像位数错误");
#endif // DEBUG
		return;
	}
	int hist[256 * 3], LUTR[256], LUTG[256], LUTB[256], sum = width * height, AR=0, AG=0, AB=0;
	getHistGram24bit(hist);
	for (int i = 0, p=0; i < 256; i++)
	{
		LUTB[i] = 255 * (AB += hist[p++]) / sum;//blue,green,red
		LUTG[i] = 255 * (AG += hist[p++]) / sum;//blue,green,red
		LUTR[i] = 255 * (AR += hist[p++]) / sum;//blue,green,red
	}
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * channel; pCur < pDest;)
	{
		*(pCur++) = LUTB[*pCur];//blue,green,red
		*(pCur++) = LUTG[*pCur];//blue,green,red
		*(pCur++) = LUTR[*pCur];//blue,green,red
	}
		


}
// 24bit直方图标准化实现（三个通道在一起实现）
void bmpConverter::PHistogramEqualize24bit1()
{
	if (BmpHeader.biBitCount != 24)
	{
#ifdef DEBUG
		puts("图像位数错误");
#endif // DEBUG
		return;
	}
	int hist[256 * 3], LUTR[256], LUTG[256], LUTB[256], sum = width * height * channel, A = 0;
	//for (; !hist[minp]; minp++);
	//minp /= 3;
	getHistGram24bit(hist);
	for (int i = 0, p = 0; i < 256;i++)
	{
		LUTB[i] = 255 * (A += hist[p++]) / sum;//blue,green,red
		//LUTB[i] = (LUTB[i] - minp) * 255 / (255 - minp);//blue,green,red
		LUTG[i] = 255 * (A += hist[p++]) / sum;//blue,green,red
		//LUTG[i] = (LUTG[i] - minp) * 255 / (255 - minp); 
		LUTR[i] = 255 * (A += hist[p++]) / sum;//blue,green,red
		//LUTR[i] = (LUTR[i] - minp) * 255 / (255 - minp); 
	}
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * channel; pCur < pDest;)
	{
		*(pCur++) = LUTB[*pCur];//blue,green,red
		*(pCur++) = LUTG[*pCur];//blue,green,red
		*(pCur++) = LUTR[*pCur];//blue,green,red
	}
}
// 得到24bit的平均直方图
void bmpConverter::getHistGram24bitavg(int * hist)
{
	for (int i = 0; i < 256; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * 3; pCur < pDest;)
		hist[(*(pCur++) + *(pCur++) + *(pCur++)) / 3]++;

}
// 24bit直方图标准化实现（三个通道的均值实现）
void bmpConverter::PHistogramEqualize24bit2()
{
	if (BmpHeader.biBitCount != 24)
	{
#ifdef DEBUG
		puts("图像位数错误");
#endif // DEBUG
		return;
	}
	int hist[256], LUT[256], sum = width * height * channel, A = 0;
	getHistGram24bitavg(hist);
	for (int i = 0; i < 256; i++)LUT[i] = 255 * (A += hist[i]) / sum;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * channel; pCur < pDest;)
		*(pCur++) = LUT[*pCur];
}

//_____________________hw3___________________________
// RGB转灰度的测试函数
void bmpConverter::RGB2Gry2(bool table_chk, bool inplace)
{
	if (channel != 3 || !pImg)return;
	int sum = width * height;
	BYTE * temp_save = new BYTE[sum];
	if (!temp_save)
	{
#ifdef DEBUG
		puts("程序错误，内存申请失败");
#endif // DEBUG
		return;
	}
#ifdef DEBUG
	//cout << (int)(temp_save) <<"113" <<sizeof(temp_save)<< endl;
	////cout << (int)pImg << endl;
#endif // DEBUG
	BYTE * p1 = temp_save, *p2 = pImg - 1;
	if (table_chk)
	{
		int r_ratio = 0.299 * 1024, g_ratio = 0.587 * 1024, b_ratio = 0.144 * 1024;
		while (sum--)*(p1++) = ((b_ratio  * int (*(++p2))) + g_ratio * (*(++p2)) + r_ratio * (*(++p2))) >> 10;//blue,green,red
	}
	else
	{
		while (sum--)*(p1++) = ((*(++p2)) * 0.114 + 0.587 * (*(++p2)) + 0.299 * (*(++p2)));//blue,green,red
	}
	if (inplace)
	{
		swap(temp_save, pImg);
		channel = 1;
	}

#ifdef DEBUG
	//cout << int(temp_save) << endl;
	//cout << (int)pImg << endl;
#endif // DEBUG
	delete temp_save;
	return;
}
// 二维均值滤波，列积分
void bmpConverter::AavgFilter2d8bit_col_cal(BYTE * pImg, int width, int height, int m, int n, BYTE  * pDes)
{
	BYTE *pAdd, *pDel, *pRes;
	int halfx, halfy;
	int x, y;
	int sum, c;
	int sumCol[1<<15]; //约定图像宽度不大于2^15

	// step.1------------初始化--------------------------//
	m = m | 1; //奇数化
	n = n | 1; //奇数化
	halfx = m / 2; //滤波器的半径x
	halfy = n / 2; //滤波器的半径y
	c = (1 << 23) / (m * n); //乘法因子
	memset(sumCol, 0, sizeof(int) * width);
	for (y = 0, pAdd = pImg; y <= halfy; y++)
	{
		for (x = 0; x < width; x++) sumCol[x] += *(pAdd++);
	}
	// step.2------------滤波----------------------------//
	for (y = 0, pRes = pDes, pDel = pImg; y < height; y++)
	{
		for (sum = 0, x = 0; x <= halfx; x++) sum += sumCol[x];
		//滤波
		//pRes += halfx; //跳过左侧
		for (x = 0; x < width; x++)
		{
			//求灰度均值
			*(pRes++) = sum * c >> 23; //用整数乘法和移位代替除法
			//换列,更新灰度和
			if(x >= halfx)sum -= sumCol[x - halfx]; //减左边列
			if(x < width - halfx )sum += sumCol[x + halfx + 1]; //加右边列
		}
		//pRes += halfx;//跳过右侧
		//换行,更新sumCol
		if (y >= halfy)
		{
			for (x = 0; x < width; x++)sumCol[x] -= *(pDel++); //减上一行
		}
		if (y < height - halfy - 1)
		{
			for (x = 0; x < width; x++)sumCol[x] += *(pAdd++); //加下一行
		}
	}
	return;
}
// 计算补充滤波器宽度的积分图
void bmpConverter::AgetCalGraph(BYTE * pImg, int width, int height, int m, int n, int * pSum)
{
	BYTE *pCur ;
	int *pRes;
	int x, y, halfx, halfy, w, i, rowsum;
	halfx = m / 2;
	halfy = n / 2;
	pCur = pImg;
	pRes = pSum;
	w = width + halfx * 2 + 1; // 由于积分图方块特性所以要多加1
	// 上侧padding
	for (i = 0; i < (halfy+ 1) * w; i++) *(pRes++) = 0;
	for (y = 0; y < height; y++)
	{
		// 左侧padding
		for (x = 0; x < halfx + 1; x++) *pRes++ = 0;
		// 正常处理
		for (x = 0, rowsum = 0; x < width; x++)
		{
			rowsum += *pCur++;
			*(pRes++) = *(pRes - w) + rowsum;
			//pRes--;
		}
		// 右侧padding
		for (x = 0; x < halfx; x++)*pRes++ = *(pRes - 1);
	}
	// 下侧padding
	for (i = 0; i < (halfy) * w; i++) *(pRes++) = *(pRes - w);
	return;
}
// 2d均值滤波 使用积分图
void bmpConverter::AavgFilter2d8bit_calGraph(int * pSum, int m, int n, BYTE * pDes)
{
	int *pY1, *pY2;
	BYTE *pRes;
	int halfx, halfy, w;
	int y, x1, x2;
	int sum, c;
	// step.1------------初始化--------------------------//
	m = m | 1; //奇数化
	n = n | 1; //奇数化
	halfx = m / 2; //滤波器的半径x
	halfy = n / 2; //滤波器的半径y
	w = width + 2 * halfx + 1;
	c = (1 << 23) / (m * n); //乘法因子
	// step.2------------滤波----------------------------//
	for (
		y = 0, pRes = pDes, pY1 = pSum, pY2 = pSum + (n) * w;
		y < height;
		y++, pY1 += w, pY2 += w
		)
	{
		for (x1 = 0, x2 = m; x1 < width; x1++, x2++) //可以简化如此，但不太容易读
		{
			sum = *(pY2 + x2) - *(pY2 + x1) - *(pY1 + x2) + *(pY1 + x1);
			*(pRes++) = (sum * c) >> 23; //用整数乘法和移位代替除法
#ifdef DEBUG
			//if (sum < 0)
			//{
			//	printf("%d  %d\n", y, x1);
			//	printf("%d  %d %d  %d\n", *(pY2 + x2), *(pY2 + x1), *(pY1 + x2), *(pY1 + x1));
			//	printf("%d\n", *(pY2 + x2 - 1));
			//}
#endif // DEBUG
		}
	}
	// step.3------------返回----------------------------//
	return;
}
/*
二维均值滤波
m: filter width
n: filter height
mod: col or row
inplace: inplace
*/ 
Img bmpConverter::AavgFilter2d1c(int m, int n, bool inplace, char mod)
{
	if (!pImg)
	{
#ifdef DEBUG
		puts("未载入图像，无法操作");
#endif // DEBUG
		return Img();
	}
	if (channel != 1)
	{
#ifdef DEBUG
		puts("不是单通道图像");
#endif // DEBUG
		return Img();
	}
	if (m > width || n > height)
	{
#ifdef DEBUG
		puts("滤波器大小错误");
#endif // DEBUG
		return Img();
	}
	BYTE* pSrc = new BYTE[width * height];
	switch (mod)
	{
	default:
	case 'c':
	case 'C':
		AavgFilter2d8bit_col_cal(pImg, width, height, m, n, pSrc);
		break;
	case 'G':
	case 'g':
	{
		int * pSum = new int[(width + (m | 1)) * (height + (n | 1))];
		AgetCalGraph(pImg, width, height, m, n, pSum);
		AavgFilter2d8bit_calGraph(pSum, m, n, pSrc);
		break;
	}
		
	}
	
	if (inplace)
	{
		delete pImg;
		pImg = pSrc;
		return Img();
	}
	else
	{
		return Img(pSrc, width, height);
	}
	
}
/* 单通道图像反转
* mod:  'n':正常
		's':使用SSE

*/
Img bmpConverter::PInvert1c(char mod, bool inplace)
{
	if (channel != 1)
	{
#ifdef DEBUG
		puts("非单通道，无法执行");
#endif // DEBUG
		return Img();
	}
	_declspec(align(16)) BYTE * pDes = new BYTE[((width * height) | 15) + 1];// 凑齐16的整数倍
	switch (mod)
		{
		default:
		case 'n':
		case 'N':
		case '0':
			PInvert8bit(pImg, width * height, pDes);
			break;
		case 's':
		case 'S':
		case '1':
			PInvert8bit_SSE(pImg, width * height, pDes);
			break;
		}
	if (inplace)
	{
		delete pImg;
		pImg = pDes;
		return Img();
	}
	else
	{
		return Img(pDes, width, height);
	}
}
// 不适用SSE的灰度图像反转
void bmpConverter::PInvert8bit(BYTE * pImg, int sum, BYTE * pDes)
{
	BYTE * pCur = pImg, * pEnd = pImg + sum, *pRes = pDes;
	while (pCur < pEnd) *(pRes++) = ~*pCur++;
}
// 使用SSE的灰度图像反转
void bmpConverter::PInvert8bit_SSE(BYTE * pImg, int sum, BYTE * pDes)
{
	BYTE * pCur, * pRes;
	__m128i * pCurSSE, *pDesSSE;
	__m128i inver;
	int res, t;

	inver = _mm_set_epi32(-1, -1, -1, -1);
	res = sum & 15, t = sum / 16;
	pCur = pImg;
	pCurSSE = (__m128i *)pCur;
	pDesSSE = (__m128i *)pDes;

	while (t--)*pDesSSE++ = _mm_sub_epi32(inver, *pCurSSE++);
	pCur = pImg + sum, pRes = pDes + sum;
	while (res--) *(pRes--) = ~*pCur--;
}
// 二维维均值滤波单通道
Img bmpConverter::AMedianFilter2d1c(int m, int n, bool inplace, char mod)
{
	if (!pImg)
	{
#ifdef DEBUG
		puts("未载入图像，无法操作");
#endif // DEBUG
		return Img();
	}
	if (channel != 1)
	{
#ifdef DEBUG
		puts("不是单通道图像");
#endif // DEBUG
		return Img();
	}
	if (m > width || n > height)
	{
#ifdef DEBUG
		puts("滤波器大小错误");
#endif // DEBUG
		return Img();
	}
	BYTE* pSrc;

	pSrc = new BYTE[width * height];
	switch (mod)
	{
		default:
		case 'c':
		case 'C':
			AMedianFilter8bit_col_cal(pImg, width, height, m, n, pSrc);
			break;
		case 'G':
		case 'g':
		{

			break;
		}
	}

	if (inplace)
	{
		delete pImg;
		pImg = pSrc;
		return Img();
	}
	else
	{
		return Img(pSrc, width, height);
	}
}
// 二维维均值滤波单通道使用列积分
void bmpConverter::AMedianFilter8bit_col_cal(BYTE * pImg, int width, int height, int m, int n, BYTE * pDes)
{
	BYTE *pCur, *pRes;
	int halfx, halfy, x, y, i, j, y1, y2, res;
	int histogram[256];
	int wSize, j1, j2;
	int num, med, v;
	int dbgCmpTimes = 0; //搜索中值所需比较次数的调试

	m = m | 1;  // 奇数化
	n = n | 1;  // 奇数化
	halfx = m / 2;  // x半径
	halfy = n / 2;  // y半径
	wSize = m * n;  // 邻域内像素总个数
	for (y = 0, pRes = pDes; y < height; y++)
	{
		// step.1----初始化直方图
		y1 = y - halfy;
		y2 = y + halfy + 1;
		memset(histogram, 0, sizeof(int) * 256);
		if (y1 < 0)			histogram[0] -= y1 * m, y1 = 0;
		if (y2 > height)	histogram[0] += (y2 - height) * m, y2 = height;
		for (i = y1, pCur = pImg + y1 * width; i < y2; i++, pCur += width)
		{
			for (j = 0; j <= halfx; j++)
			{
				histogram[pCur[j]]++;
			}
		}
		//step.2-----初始化中值
		num = 0; //记录着灰度值从0到中值的个数
		for (i = 0; i < 256; i++)
		{
			num += histogram[i];
			if (num * 2 > wSize)
			{
				med = i;
				break;
			}
		}
		// 滤波
		// pRes += halfx; // 没有处理图像左边界侧的像素
		res = n - y2 + y1;
		for (x = 0; x < width; x++)
		{
			// 赋值
			*(pRes++) = med;
#ifdef DEBUG
			//if (!x || x == 1 || x == 250)
			//{
			//	printf("%d \n", med);
			//	printf("num: %d\n", num);
			//	//int tot = 0;
			//	//for (int i = 0; i < 256; i++)tot += histogram[i];
			//	//printf("tot : %d\n", tot);
			//}
#endif // DEBUG

			// step.3-----直方图递推: 减去当前邻域最左边的一列,添加邻域右侧的一个新列
			j1 = x - halfx;  // 最左边列
			j2 = x + halfx + 1;  // 右边的新列
			if (j1 < 0)  // 如果左边超过了
			{
				histogram[0] -= n;
				num -= n;
			}
			else
			{
				for (i = y1, pCur = pImg + y1 * width; i < y2; i++, pCur += width)
				{
					// 减去最左边列
					v = pCur[j1];
					histogram[v]--;  //更新直方图
					if (v <= med) num--; //更新num
				}
				histogram[0] -= res; // 处理padding部分
				num -= res;
			}
			if (j2 >= width)  // 如果右边超过了
			{
				histogram[0] += n;
				num += n;
			}
			else
			{
				for (i = y1, pCur = pImg + y1 * width; i < y2; i++, pCur += width)
				{
					// 加上最右边列
					v = pCur[j2];
					histogram[v]++;  // 更新直方图
					if (v <= med) num++; // 更新num
				}
				histogram[0] += res; // 处理padding部分
				num += res;
			}
			// step.4-----更新中值
			if (num * 2 < wSize) // 到上次中值med的个数不够了,则med要变大
			{
				for (med = med + 1; med < 256; med++)
				{
					dbgCmpTimes += 2; //总的比较次数,调试用
					num += histogram[med];
					if (num * 2 >= wSize) break;
				}
				dbgCmpTimes += 1; //总的比较次数,调试用
			}
			else //到上次中值med的个数多了,则med要变小
			{
				while ((num - histogram[med]) * 2 > wSize)//若减去后,仍变小
				{
					dbgCmpTimes++; //总的比较次数,调试用
					num -= histogram[med--];
				}
				dbgCmpTimes += 2; //总的比较次数,调试用
			}
		}
		//pRes += halfx;//没有处理图像右边界侧的像素
	}
	printf("dbg: %lf \n", 1.0 * dbgCmpTimes / width / height);
	return;
}
// 一维维高斯滤波单通道
void bmpConverter::AGuassFilter1d(BYTE * pSrc, int width, int height, int * pFilter, int m, BYTE * pDes)
{
	int offset = 17;  
	// 偏移量设置为17的原因，将求和后再位移，因为先位移会损失精度很多次，按照比例来说会有50%的计算损失1亮度
	// 所以累积就是 m/2的亮度损失，m为滤波器的宽，在实验之后，放入PS也观察到了相似的结果，是8亮度，相对于m=19的滤波器
	// 偏移16的缺点，滤波器的精度下降，所有之和只为，0.999931，按照平均亮度是128算（实际上原图的平均亮度只有61），亮度损失只有1不到
	// 从宏观上看，相对于23的offset图片整体的亮度提升了
	int halfm, y,x, f, sum = 0;
	BYTE  *pCur, *pRes, *pY;

	halfm = m / 2;

	for (y = 0, pY = pSrc, pRes = pDes; y < height; y++, pY += width)
	{
		// 处理左侧
		for (x = 0, sum = 0; x < halfm; x++, sum = 0)
		{
			for (f = halfm - x, pCur = pY; f < m; f++)
			{
				sum += pFilter[f] * *pCur++ ;
			}
			sum >>= offset;
#ifdef DEBUG
			if (sum < 0 || sum > 255)
			{
				printf("1%d %d %d\n", sum, y, x);
			}
#endif // DEBUG
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
#ifdef DEBUG
			if (sum < 0 || sum > 255)
			{
				printf("2%d %d %d\n", sum, y, x);
			}
#endif // DEBUG
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
#ifdef DEBUG
			if (sum < 0 || sum > 255)
			{
				printf("2%d %d %d\n", sum, y, x);
			}
#endif // DEBUG
		}
	}
}
// 计算pSrc的转置
void bmpConverter::T(BYTE * pSrc, int w, int h, BYTE * pDes)
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

// 计算高斯核
void getGuassFilter(double dev, int m, int * pFilter, int offset=17)
{
	int halfm;
	double sum;
	double dF[1024];

	halfm = m / 2;
	sum = dF[halfm] = 1;

	for (int i = 1; i <= halfm; i++)
	{
		dF[halfm + i] = dF[halfm - i] = exp( - i * i / 2.0 / dev / dev);
		sum += dF[halfm + i] + dF[halfm + i];
	}
	for (int i = 0; i <= halfm; i++)
	{
		pFilter[halfm + i] = pFilter[halfm - i] = (1 << offset) * dF[halfm + i] / sum;
	}
}
// 二维高斯滤波单通道
Img bmpConverter::AGuassFilter2d1c(double dev, int m, bool inplace, char mod)
{
	if (!pImg)
	{
#ifdef DEBUG
		puts("未载入图像，无法操作");
#endif // DEBUG
		return Img();
	}
	if (channel != 1)
	{
#ifdef DEBUG
		puts("不是单通道图像");
#endif // DEBUG
		return Img();
	}
	if (m > width || m > height || m >= 80)
	{
#ifdef DEBUG
		puts("滤波器大小错误");
#endif // DEBUG
		return Img();
	}
	BYTE* pSrc, * pSrc2, * pSrcT, * pSrc2T;
	int pFilter[128];
	m |= 1;

	getGuassFilter(dev, m, pFilter);
	pSrc = new BYTE[width * height];


#ifdef DEBUG
	//double fsum = 0;
	//int offset = 17;
	//for (int i = 0; i < m; i++)
	//{
	//	fsum += 1.0 * pFilter[i] / (1 << offset);
	//	printf("%lf \n", 1.0 * pFilter[i] / (1 << offset));
	//}
	//cout << fsum << endl;
#endif // DEBUG
	switch (mod)
	{
	default:
	case 'c':
	case 'C':
	{
		pSrc2 = new BYTE[width * height];
		pSrcT = new BYTE[width * height];
		pSrc2T = new BYTE[width * height];

		AGuassFilter1d(pImg, width, height, pFilter, m, pSrc);
		T(pSrc, width, height, pSrcT);
		AGuassFilter1d(pSrcT, height, width, pFilter, m, pSrc2);
		T(pSrc2, height, width, pSrc2T);
		delete pSrc;
		delete pSrc2;
		delete pSrcT;
		pSrc = pSrc2T;
		break;
	}
		
	case 'G':
	case 'g':
	{

		break;
	}
	}

	if (inplace)
	{
		delete pImg;
		pImg = pSrc;
		return Img();
	}
	else
	{
		return Img(pSrc, width, height);
	}
}

//__________________________________hw4______________________________
void getEdg(BYTE * pSrc, int width, int height, BYTE threshold)
{
	BYTE * pCur, *pEnd;
	for (pCur = pSrc, pEnd = pSrc + width * height; pCur < pEnd; pCur++)
	{
		*pCur = *pCur > threshold ? 255 : 0;
	}
}
// 边缘检测，一阶梯度+沈俊算子
Img bmpConverter::AEadgeDectGrad_SJ2d1c(double a0, bool inplace)
{
	if (!pImg)return Img();
	BYTE *pDstImg, *pTmpImg;
	BYTE threshold;
	int i;

	threshold = 10;
	pDstImg = new BYTE[width * height];
	pTmpImg = new BYTE[width * height];
	a0 = (max)(0.01, (min)(a0, 0.99));

	AEDOpShenJun4_8bit(pImg, pTmpImg, width, height, a0, pDstImg);
	AEDOpGrad1_8bit(pImg, width, height, pTmpImg);
	getEdg(pTmpImg, width, height, threshold);

	for (i = 0; i < width * height; i++)
	{
		pDstImg[i] = pDstImg[i] & pTmpImg[i];
	}

	delete pTmpImg;
	if (inplace)
	{
		delete pImg;
		pImg = pDstImg;
		return Img();
	}
	else
	{
		return Img(pDstImg, width, height);
	}

	return Img();
}

Img bmpConverter::AEadgeDectLaplacain_Gd2d1c(double a0, BYTE threshold, bool inplace)
{
	if (!pImg)return Img();
	BYTE *pDstImg, *pTmpImg;
	int i;

	pDstImg = new BYTE[width * height];
	pTmpImg = new BYTE[width * height];
	a0 = (min)(0.01, (max)(a0, 0.99));

	AEDOpShenJun4_8bit(pImg, pTmpImg, width, height, a0, pDstImg);
	AEDOpGrad1_8bit(pImg, width, height, pTmpImg);
	getEdg(pTmpImg, width, height, threshold);

	for (i = 0; i < width * height; i++)
	{
		pDstImg[i] = pDstImg[i] & pTmpImg[i];
	}

	delete pTmpImg;
	if (inplace)
	{
		delete pImg;
		pImg = pDstImg;
		return Img();
	}
	else
	{
		return Img(pDstImg, width, height);
	}

	return Img();
}


Img bmpConverter::AEadgeDectsobel_ShenJun2d1c(double a0, BYTE threshold, bool inplace)
{
	if (!pImg)return Img();
	BYTE *pDstImg, *pTmpImg;
	int i;

	pDstImg = new BYTE[width * height];
	pTmpImg = new BYTE[width * height];
	a0 = (max)(0.01, (min)(a0, 0.99));

	AEDOpShenJun4_8bit(pImg, pTmpImg, width, height, a0, pDstImg);
	AEDOpSobel_8bit(pImg, width, height, pTmpImg);

	getEdg(pTmpImg, width, height, threshold);
	for (i = 0; i < width * height; i++)
	{
		pDstImg[i] = pDstImg[i] & pTmpImg[i];
	}


	delete []pTmpImg;
	if (inplace)
	{
		delete pImg;
		pImg = pDstImg;
		return Img();
	}
	else
	{
		return Img(pDstImg, width, height);
	}
}
// 缩小到1/16，使用均值，已知宽高
void bmpConverter::shrink16(BYTE *pSrcImg, int width, int height, BYTE *pDstImg)
{
	int sum;
	BYTE *pCur, *pDes;
	int x, y;

	for (y = 0, pCur = pSrcImg, pDes = pDstImg; y < height; y += 4)
	{
		for (x = 0; x < width; x += 4)
		{
			// 求和
			//sum = 0, y = 0;
			//for (y = 0; y < 4 * width; y += width)
			//{
			//	sum += pCur[y];
			//	sum += pCur[y + 1];
			//	sum += pCur[y + 2];
			//	sum += pCur[y + 3];
			//}
			//*pDes++ = sum / 16;
			*pDes++ = *pCur;
			pCur += 4;
		}
		pCur += 3 * width;
	}

}


void getMaxAreaCalGraph(int *pSum, int m, int n, int width, int height, int &mx, int &my)
{
	int *pY1, *pY2, *pY3;
	int halfx, halfy, w;
	int y, x1, x2;
	int curSum, befSum, nxtSum, maxSum, c, tempSum;
	int rowSum[1024];

	// 初始化
	m = m | 1; 
	n = n | 1; 
	halfx = m / 2; 
	halfy = n / 2; 
	w = width + 1;
	c = (1 << 23) / (m * n); 
	mx = 0, my = 0;
	maxSum = -1e9;
	for (x1 = width - m; x1 <= width + m; x1++)rowSum[x1] = 0;

	// 最大值区域求取
	//for (y = 2 * n, pY3 = pSum, pY1 = pY3 + w * n, pY2 = pY1 + w * n;	y < height;	y++, pY1 += w, pY2 += w)
	for (y = 2 * n, pY3 = pSum, pY1 = pY3 + w * n, pY2 = pY1 + w * n;	y < height;	y++, pY1 += w, pY2 += w, pY3 += w)
	{
		befSum = 0, curSum = 0;
		for (x1 = 0, x2 = m; x2 <= width; x1++, x2++)
		{
			rowSum[x1] = pY1[x1] - pY1[x2] - pY2[x1] + pY2[x2];
		}
		for (x1 = m; x1 <= width; x1++)
		{
			//tempSum = rowSum[x1] - rowSum[x1 - m] - rowSum[x1 + m];
			tempSum = rowSum[x1] - rowSum[x1 - m] - rowSum[x1 + m] -   (pY3[x1] - pY3[x1 + w] - pY1[x1] + pY1[x1 + w]);
			if (tempSum > maxSum)
			{
				maxSum = tempSum;
				mx = x1, my = y;

			}
		}
	}
	return;
}

void drawSquare(BYTE *pSrcImg, int width, int height, int mx, int my, int w, int h)
{
	BYTE *pCur, *pY, *pX;

	pCur = pSrcImg;
	// 上横线
	for (pCur = pSrcImg + width * my + mx, pX = pCur + w; pCur < pX; ) *pCur++ = 255;
	//下横线
	for (pCur = pSrcImg + width * (my + h) + mx, pX = pCur + w; pCur < pX; ) *pCur++ = 255;
	// 左竖线
	for (pCur = pSrcImg + width * my + mx, pY = pCur + h * width; pCur < pY; pCur += width) *pCur = 255;
	// 右竖线
	for (pCur = pSrcImg + width * (my)+ mx + w, pY = pCur + h * width; pCur <= pY; pCur += width) *pCur = 255;
}
void bmpConverter::num_dect()
{
// 缩小
// 得到边缘强度
// 积分图
// 得到最大强度区域
// 放大

	BYTE *pDstImg, *pTmpImg, *pSrcImg, *pOrgImg;
	int *pCalGraph;
	int winSize, mx, my;
	BYTE threshold;
	double a0;

	// 初始化
	a0 = 0.1;
	threshold = 80;
	winSize = 25;
	pCalGraph = new int[(width / 4 + (winSize | 1)) * (height / 4 + (winSize | 1))];
	pSrcImg = new BYTE[width * height / 16];
	//BYTE * pSbImg = new BYTE[width * height / 16];
	pOrgImg = pImg;

	// 缩小图片
	shrink16(pImg, width, height, pSrcImg);
	width /= 4, height /= 4;
	pImg = pSrcImg;
	// 得到边缘数目
	AEadgeDectsobel_ShenJun2d1c(a0, threshold);
	//AEadgeCanny2d1c(1, 5, 60, 80);
	//AEDOpSobel_8bit(pImg, width, height, pSbImg);
	//delete[] pSrcImg;
	//pImg = pSbImg;
	Img2Bmp("./pic/temp.bmp");
	// 获得边缘数目的积分图
	AgetCalGraph(pImg, width, height, 1, 1, pCalGraph);
	// 原图像100x100的区域
	// 4x4缩小之后，25x25
	// 寻找计算图中值最大区域
	getMaxAreaCalGraph(pCalGraph, winSize, winSize, width, height, mx, my);
	//drawSquare(pImg, width, height, mx, my - winSize, winSize, winSize);
	mx *= 4, my = my * 4, width = width * 4, height = height * 4, winSize = winSize * 4;
	pSrcImg = pImg;
	pImg = pOrgImg;
	// 画出框框
	drawSquare(pImg, width, height, mx, my - winSize, winSize, winSize);
	delete[] pCalGraph;
	delete[] pSrcImg;
}

// canny算子边缘检测
Img bmpConverter::AEadgeCanny2d1c(double sigma, int Fsize, int uBound, int lBound, bool inplace)
{
	BYTE *pDstImg, *pTmpImg;
	int *pFilter, *dx, *dy;

	// 初始化
	sigma = 0.4;
	Fsize = 5;
	uBound = 125;
	lBound = 75;
	pDstImg = new BYTE[width * height];
	dx = new int[width * height];
	dy = new int[width * height];
	pFilter = new int[width * height];
	pTmpImg = new BYTE[width * height];

	AEDOpCanny_8bit(
		pImg,
		pTmpImg,
		dx, dy,
		width, height,
		pFilter, sigma, Fsize,
		uBound, lBound,
		pDstImg
	);

	// 结束
	delete[] pTmpImg;
	delete[] pFilter;
	delete[] dx;
	delete[] dy;
	if (inplace)
	{
		delete[] pImg;
		pImg = pDstImg;
		return Img();
	}
	else
	{
		return Img(pDstImg, width, height);
	}
}


//________hw5___________________
void bmpConverter::PBinOtsu2d1c()
{
	int *hist = new int[256];
	int thres, pixelNum = width * height;

	getHist8b(pImg, pixelNum, hist);
	clock_t start = clock();
	for(int i = 0 ; i< 10000; i++)
	thres = getOtsuThreshold8b(hist, 256);
	clock_t end = clock();
	cout << "1： " << end - start<<endl;
	start = clock();
	for (int i = 0; i < 10000; i++)
	thres = RmwGetOtsuThreshold(hist, 256);
	end = clock();
	cout << "2： " << end - start << endl;

	PGry2Bin(pImg, pixelNum, thres, pImg);

	delete[] hist;
}

void bmpConverter::AAvgSmooth1d(int * hist, int width, int filterSize, int *dstHist)
{
	if (width < filterSize)return;

	filterSize |= 1;
	const int half = filterSize / 2;
	int *pCur = hist + half, *pEnd = hist - half + width, *pDst = dstHist + half;
	int sum = *(pCur - half) - *(pCur + half) + *pCur;
	int i;
	for (i = 1; i <= half; i++)
	{
		sum += pCur[i] + pCur[-i];
		dstHist[i - 1] = hist[i - 1];
		dstHist[ width - i] = hist[width - i];
		cout << dstHist[width - i] << endl;
	}
	
	for (; pCur < pEnd; pCur++)
	{
		sum += *(pCur + half) - *(pCur - half);
		*pDst++ = sum / filterSize; cout << "pdst: " << *(pDst - 1) << endl;
	}

}



void bmpConverter::BinOtsu_test()
{
#if 1
	int *imem = new int[512];
	int *hist = imem, *smoothed_hist = hist + 256;
	int thres, pixelNum = width * height;

	getHist8b(pImg, pixelNum, hist);
	//smoothed_hist = hist;
	AAvgSmooth1d(hist, 256, 5, smoothed_hist);
	thres = getOtsuThreshold8b(smoothed_hist, 256);
	cout << "thres: " << thres << endl;

	PGry2Bin(pImg, pixelNum, thres, pImg);
#else
	int *imem = new int[512];
	int *hist = imem, *scale_hist = hist + 256;
	int thres, pixelNum = width * height;

	memset(scale_hist, 0, sizeof(int) * 256);
	getHist8b(pImg, pixelNum, hist);
	for (int i = 0; i < 256; i++)scale_hist[i / 4] += hist[i];
	thres = getOtsuThreshold8b(scale_hist, 256);
	cout << "thres: " << thres << endl;
	PGry2Bin(pImg, pixelNum, thres, pImg);
#endif // 1

	delete[] imem;

}


void bmpConverter::task_ImgSegmentation1()
{	
	// 在程序当中都采用了统一先将内存开够，再分配内存的思想；
	// 目的是增加内存的利用率，减少碎片，。

	double sigma = 3;
	int pixelNum = height * width, Fsize = 3 * sigma + 1 + EPS;
	BYTE * bMem = new BYTE[width * height * 3];
	BYTE *pBkgImg = bMem, *pTmpImg = pBkgImg + pixelNum, *pMinImg = pTmpImg + pixelNum;
	int Filter[256], hist[256];
	int threshold;
	
	// 最小值滤波
	AMinFilter(pImg, width, height, 3, 3, pMinImg);
	// 得到背景,光照估计
	GuassianBlur(pMinImg, pTmpImg, width, height, sigma, Fsize, Filter, pBkgImg);
	// 相减，图像减法
	for (int i = 0; i < pixelNum; i++)
	{
		pTmpImg[i] = (max)(0, pImg[i] - pBkgImg[i]);
	}
	// Otsu二值化
	getHist8b(pTmpImg, pixelNum, hist);
	threshold = getOtsuThreshold8b(hist, 256);
	PGry2Bin(pTmpImg, pixelNum, threshold, pImg);
	//PMaxFilter(pTmpImg, width, height, 3, 3, pImg);

	delete[] bMem;
}

double MultiSegJudge1(BYTE *pSrcImg, int width, int height, BYTE *pTmpImg)
{
	// 评价函数2个标准，1.评价环线  2.评价弹孔
	// 主要的目标应该是将背景和其他两个分开
	unsigned int eval = 0;
	BYTE *pCur = pSrcImg + width - 1;
	int x, y, bCur;
	for (y = 1; y < height - 1; y++)
	{
		pCur += 2;
		for (x = 1; x < width - 1; x++)
		{
			// 检查当前点与周围点的梯度
			bCur = *pCur++;
			// 使用异或代替相减取绝对值的运算
			eval += ((bCur ^ pCur[-1]) + (bCur ^ pCur[1]) + (bCur ^ pCur[-width]) + (bCur ^ pCur[width])) >> 7;
		}  // x
	}  // y
	
	return eval;
}

double MultiSegJudge2(BYTE *pSrcImg, int width, int height, BYTE *pTmpImg)
{
	
	return 0.0;
}


// 点运算,将区间内的颜色变化到指定颜色
void PColorTrans(BYTE *pSrcImg, int PixelNum, BYTE lowB, BYTE upB, BYTE TargetBry, BYTE *pDstImg)
{
	BYTE *pCur = pSrcImg, *pEnd = pCur + PixelNum, *pDst = pDstImg;

	while (pCur < pEnd)
	{
		if (*pCur >= lowB && *pCur <= upB)
		{
			*pDst = TargetBry;
		}
		*pCur++, pDst++;
	}

}

// 多次分割法
void bmpConverter::task_ImgSegmentation2()
{
	int i;
	int PixelNum = width * height;
	int step1 = 16, step2 = 1;
	int hist[256];
	BYTE *bMem = new BYTE[PixelNum];
	BYTE *pTmpImg = bMem;
	int hmin = 0, hmax = 255;
	BYTE threshold1 = 0, threshold2 = 50;  // 两类的阈值
	double opt1 = -1, opt2 = -1, jval;  // 最优值

	getHist8b(pImg, PixelNum, hist);
	for (; hmin < 256 && !hist[hmin]; hmin++);
	for (; hmax > -1 && !hist[hmax]; hmax--);
	// 跨度为step1的粗搜索
	for (i = hmin + 1; i < hmax; i += step1)
	{
		PGry2Bin(pImg, PixelNum, i, pTmpImg);
		jval = MultiSegJudge1(pTmpImg, width, height, pTmpImg);
		if (jval > opt1)
		{
			opt1 = jval;
			threshold1 = i;
		}
	}
	// 跨度为step2的精搜索
	for (i = (max)(hmin + 1, threshold1 - step1); i < (min)(hmax, threshold1 + step1); i += step2)
	{
		PGry2Bin(pImg, PixelNum, i, pTmpImg);
		jval = MultiSegJudge1(pTmpImg, width, height, pTmpImg);
		if (jval > opt1)
		{
			opt1 = jval;
			threshold1 = i;
		}
	}
	// threshold1 = 170;
	//memset(pTmpImg, -1, sizeof(pTmpImg[0]) * PixelNum);
	PColorTrans(pImg, PixelNum, 0, threshold2, 0, pTmpImg);
	PColorTrans(pImg, PixelNum, threshold2 + 1, threshold1, 128, pTmpImg);
	PColorTrans(pImg, PixelNum, threshold1 + 1, 255, 255, pTmpImg);
	memcpy(pImg, pTmpImg, sizeof(pTmpImg[0]) * PixelNum);

	delete[] bMem;
}


void bmpConverter::task_ImgSegmentation4()
{

	int i, g;
	double dist;
	int hist[256], LUT[256], Filter[256], color[]{ 0, 128, 255 };
	double Center[3];
	double bound = 3, sigma = 3;
	int PixelNum = width * height, max_iter = 100, Fsize = 3 * sigma + 1 + EPS, k = 3;
	BYTE *bMem = new BYTE[PixelNum * 3];
	BYTE *pBkgImg = bMem, *pTmpImg = pBkgImg + PixelNum, *pMinImg = pTmpImg + PixelNum;
	BYTE *pCur = pTmpImg, *pEnd = pCur + PixelNum, *pDst = pImg, *pHistSrc = pTmpImg;

	// 最小值滤波
	AMinFilter(pImg, width, height, 3, 3, pMinImg);
	// 得到背景,光照估计
	GuassianBlur(pMinImg, pTmpImg, width, height, sigma, Fsize, Filter, pBkgImg);
	// 相减，图像减法
	for (int i = 0; i < PixelNum; i++)
	{
		pTmpImg[i] = (max)(0, pImg[i] - pBkgImg[i]);
	}
	// 获得直方图
	getHist8b(pHistSrc, PixelNum, hist);
	// k-means聚类
	Hist_kMeans(hist, 256, k, Center, bound, max_iter);
	g = 0;
	for (i = 0; i < 256; i++)
	{
		dist = 1024.0;
		for (g = 0; g < k; g++)
		{
			if (dist > abs(i - Center[g]))
			{
				dist = abs(i - Center[g]);
				LUT[i] = color[g];
			}
		}  // g
	}  // i
	//pCur = pTmpImg, pDst = pImg;

	while (pCur < pEnd)
		*pDst++ = LUT[*pCur++];

	delete[] bMem;

}

void bmpConverter::k_meansThreshold(int k)
{
	if (k <= 1 && k > 254)return;
	int i, g, step;
	double dist;
	int hist[256], LUT[256], color[256];
	double Center[256];
	double bound = 3.0;
	int PixelNum = width * height, max_iter = 100;
	BYTE *bMem = new BYTE[PixelNum];
	BYTE *pTmpImg = bMem;
	BYTE *pCur = pImg, *pEnd = pCur + PixelNum, *pDst = pImg;

	// 初始化颜色
	step = 255 / (k - 1);
	for (i = 0, g = 0; i < k; i++, g += step)color[i] = g;
	// 获得直方图
	getHist8b(pImg, PixelNum, hist);
	// k-means聚类
	Hist_kMeans(hist, 256, k, Center, bound, max_iter);
	//for (i = 0; i < k; i++)
	//{
	//	printf("threshold[%d]: %lf\n", i + 1, Center[i]);
	//}
	g = 0;
	for (i = 0; i < 256; i++)
	{
		if (!hist[i])continue;
		dist = 1024.0;
		for (g = 0; g < k; g++)
		{
			if (dist > abs(i - Center[g]))
			{
				dist = abs(i - Center[g]);
				LUT[i] = color[g];
			}
		}
	}
	while (pCur < pEnd)
	{
		*pDst++ = LUT[*pCur++];
	}

	delete[] bMem;
}
// 聚类分割法
void bmpConverter::task_ImgSegmentation3()
{
	int i, g;
	double dist;
	int hist[256], LUT[256], color[]{ 0, 128, 255 };
	double Center[3];
	double bound = 3;
	int PixelNum = width * height, max_iter = 100, k = 3;
	BYTE *pCur = pImg, *pEnd = pCur + PixelNum, *pDst = pImg, *pHistSrc = pImg;

	// 获得直方图
	getHist8b(pHistSrc, PixelNum, hist);
	// k-means聚类
	Hist_kMeans(hist, 256, k, Center, bound, max_iter);
	g = 0;
	for (i = 0; i < 256; i++)
	{
		dist = 1024.0;
		for (g = 0; g < k; g++)
		{
			if (dist > abs(i - Center[g]))
			{
				dist = abs(i - Center[g]);
				LUT[i] = color[g];
			}
		}
	}
	//pCur = pTmpImg, pDst = pImg;
	
	while (pCur < pEnd)
		*pDst++ = LUT[*pCur++];
}
//____________hw6___________________________________________
// 得到边缘
void ZEdgDetect(BYTE *pSrcImg, int width, int height, int *X, int *Y, int &n)
{
	BYTE *pCur = pSrcImg, *pY = pSrcImg, *pX = pSrcImg;
	int y, x;

	n = 0;
	for (y = 0; y < height; y++, pY += width)
	{
		pCur = pY;
		for (x = 0; x < width; x++, pCur++)
		{
			if (!*pCur)
			{
				X[n] = x;
				Y[n++] = y;
				break;
			}
		}  // x
		pCur = pY + width - 1;
		for (x = width - 1; x >= 0; x--, pCur--)
		{
			if (!*pCur)
			{
				X[n] = x;
				Y[n++] = y;
				break;
			}
		}  // x
		
	}  // y

	for (x = 0; x < width; x++, pX += 1)
	{
		pCur = pX;
		for (y = 0; y < height; y++, pCur += width)
		{
			if (!*pCur)
			{
				X[n] = x;
				Y[n++] = y;
				break;
			}
		}  // y
		pCur = pX + (height - 1) * width;
		for (y = height- 1; y >= 0; y--, pCur -= width)
		{
			if (!*pCur)
			{
				X[n] = x;
				Y[n++] = y;
				break;
			}
		}  // y

	}  // x


}
// 将无符号int映射到BYTE上
void transUI2B256(int *pSrc, BYTE *pDst, int num)
{
	int mx = *pSrc, mn = *pSrc;
	double c;
	int i;
	// 得到最大，最小值，然后做映射
	for (i = 1; i < num; i++)
	{
		int isrc = pSrc[i];
		if (isrc > mx)
		{
			mx = isrc;
		}
		else if (isrc < mn)
		{
			mn = isrc;
		}
	}
	// 对数映射
	c = 256.0 / log(mx - mn + 1 + EPS);

	for (i = 0; i < num; i++)
	{
		pDst[i] = (log(pSrc[i] - mn + 1 + EPS) * c);
		//if(pDst[i])
		//pDst[i] = (min)(255, pDst[i] + 25);
	}
}
void hough_max_search1(int *pCount, int maxThro, double *A, double *B, double *C)
{
	int *pCur;
	int maxCount, bstTheta, bstThro;
	int threThro = 30, threTheta = 5, mxTho, mnTho;
	int theta, thro, i, deg;
	// 最大值搜索
	maxCount = bstTheta = bstThro = 0;
	for (theta = 0, pCur = pCount; theta < 180; theta++)
	{
		for (thro = 0; thro < maxThro * 2; thro++, pCur++)
		{
			if (*pCur > maxCount)
			{
				maxCount = *pCur;
				bstTheta = theta;
				bstThro = thro;

			}
		}
	}
	
	// 排出最优直线周围直线
	mnTho = (max)(0, bstThro - threThro), mxTho = (min)(2 * maxThro, bstThro + threThro);
	for (deg = 180 + bstTheta - threTheta, pCur = pCount + deg * 2 * maxThro; 
		deg <= 180; 
		deg++, pCur += 2 * maxThro)
	{
		for (i = maxThro * 2 + bstThro - threThro; i <= maxThro * 2; i++)
			pCur[i] = 0;
		for (i = mnTho; i <= mxTho; i++)
			pCur[i] = 0;
		for (i = 0; i <= bstThro + threThro - maxThro * 2; i++)
			pCur[i] = 0;
	}
	for (deg = (max)(0, bstTheta - threTheta), pCur = pCount + deg * 2 * maxThro; 
		deg <= (min)(180, bstTheta + threTheta); 
		deg++, pCur += 2 * maxThro)
	{
		for (i = maxThro * 2 + bstThro - threThro; i <= maxThro * 2; i++)
			pCur[i] = 0;
		for (i = mnTho; i <= mxTho; i++)
			pCur[i] = 0;
		for (i = 0; i <= bstThro + threThro - maxThro * 2; i++)
			pCur[i] = 0;
	}
	for (deg = 0, pCur = pCount + deg * 2 * maxThro;
		deg <= bstTheta + threTheta - 180; 
		deg++, pCur += 2 * maxThro)
	{
		for (i = maxThro * 2 + bstThro - threThro; i <= maxThro * 2; i++)
			pCur[i] = 0;
		for (i = mnTho; i <= mxTho; i++)
			pCur[i] = 0;
		for (i = 0; i <= bstThro + threThro - maxThro * 2; i++)
			pCur[i] = 0;
	}


	pCount[bstTheta * maxThro * 2 + bstThro] = 0;
	
	bstThro -= maxThro; //去掉偏移maxThro
	//x*cos(bstTheta)+y*sin(bstTheta)=bstThro => Ax+By+C=0
	A[0] = cos(bstTheta * PI / 180);
	B[0] = sin(bstTheta * PI / 180);
	C[0] = -bstThro;
}
// 霍夫检测四边形，逐个
void bmpConverter::HoughLine1()
{
	int PixelNum = width * height, pXSize = 2 * width + 2 * height;
	int maxThro = (int)sqrt(1.0*width*width + height * height + 0.5) + 1;
	BYTE *bMem = new BYTE[width * height + maxThro * 360];
	BYTE *pOrgImg = bMem, *pShowImg = pOrgImg + PixelNum;
	int *iMem = new int[width * height * 2 + 2 * pXSize + maxThro * 360];
	int *pX = iMem, *pY = pX + pXSize, num;
	int *pCount = pY + pXSize;
	double A[4], B[4], C[4];
	int i;

	memcpy(pOrgImg, pImg, sizeof(pOrgImg[0]) * PixelNum);
	//获得边缘
	ZEdgDetect(pImg, width, height, pX, pY, num); 
	memset(pOrgImg, -1, sizeof(*pOrgImg) * PixelNum);
	for (i = 0; i < num; i++)
	{
		pOrgImg[pY[i] * width + pX[i]] = 0;
	}
	drawImg("./pic/H0602Bin_edg.bmp", pOrgImg, width, height);
	HoughLine(pImg, width, height, pCount, pX, pY, num);
	transUI2B256(pCount, pShowImg, maxThro * 360);
	drawImg("./pic/Gry.bmp", pShowImg, maxThro * 2, 180);
	// 最大值搜索
	for (i = 0; i < 4; i++)
	{
		hough_max_search1(pCount, maxThro, A + i, B + i, C + i);
		DrawABCLine(pImg, width, height, A[i], B[i], C[i], 0);
	}

	delete[] bMem;
	delete[] iMem;
}

// 遍历更新
inline void MarginTraverse(int *pCur, int *curRho, int maxThro, int threThro, int ThroGap)
{
	int rho, rho1, mx = 0;
	int mxThro2 = 2 * maxThro;
	for (rho = 0; rho < maxThro * 2 - ThroGap + threThro; rho++)
	{
		for (rho1 = (max)(0, rho + ThroGap - threThro); rho1 < (min)(mxThro2, rho + ThroGap + threThro); rho1++)
		{
			//printf("rho1: %d  %d   %d %d\n", rho1, mxThro2, rho + ThroGap + threThro, (min)(mxThro2, rho + ThroGap + threThro));
			if (pCur[rho] + pCur[rho1] > mx)
			{
				mx = pCur[rho] + pCur[rho1];
				curRho[0] = rho;
				curRho[1] = rho1;
			}
		}  // rho1
	}  // rho
}

// 联合求取最大值
void hough_max_search2(int *pCnt, int maxThro, double *A, double *B, double *C)
{
	int threThro = 10, threTheta = 5, mxTho, mnTho;
	int ThroGap = 230, ThetaGap = 90, mxThro2 = 2 * maxThro;
	int sum = 0, mxsum = 0;
	int optRho[4]{ 0, 0, 0, 0 }, optTheta[4]{ 0, 0, 0, 0 };
	int curRho[4], curTheta[4];
	int *pCur, *pCur1;
	int theta, theta1, rho, rho1, i;
	// 死循环了

	for (theta = 0; theta < 75; theta++)
	{
		int mx = 0, mx2 = 0, sum1 = 0;
		// 寻找最大rho0 + rho1
		pCur = pCnt + theta * 2 * maxThro;
		MarginTraverse(pCur, curRho, maxThro, threThro, ThroGap);
		//printf("rho2, rho3\n");
		// 寻找最大rho2 + rho3
		for (theta1 = theta + ThetaGap; theta1 < theta + ThetaGap + threTheta - 180; theta1++)
		{
			pCur1 = pCnt + theta1 * 2 * maxThro;
			MarginTraverse(pCur1, curRho + 2, maxThro, threThro, ThroGap);
			sum1 = pCur1[curRho[2]] + pCur1[curRho[3]] + pCur[curRho[0]] + pCur[curRho[1]];
			if (sum1 > mxsum)
			{
				mxsum = sum1;
				optRho[0] = curRho[0];
				optRho[1] = curRho[1];
				optRho[2] = curRho[2];
				optRho[3] = curRho[3];
				optTheta[0] = optTheta[1] = theta;
				optTheta[2] = optTheta[3] = theta1;
			}
		}
		//printf("seg 1\n");
		for (theta1 = theta + ThetaGap - threTheta; theta1 < (min)(180, theta + ThetaGap + threTheta); theta1++)
		{
			pCur1 = pCnt + theta1 * 2 * maxThro;
			MarginTraverse(pCur1, curRho + 2, maxThro, threThro, ThroGap);
			sum1 = pCur1[curRho[2]] + pCur1[curRho[3]] + pCur[curRho[0]] + pCur[curRho[1]];
			if (sum1 > mxsum)
			{
				mxsum = sum1;
				optRho[0] = curRho[0];
				optRho[1] = curRho[1];
				optRho[2] = curRho[2];
				optRho[3] = curRho[3];
				optTheta[0] = optTheta[1] = theta;
				optTheta[2] = optTheta[3] = theta1;

			}
		}


	}  // theta

	for (i = 0; i < 4; i++)
	{
		//printf("rho: %d  theta: %d   maxThro: %d\n", optRho[i], optTheta[i], maxThro);
		//printf("val %d\n", pCnt[optTheta[i] * mxThro2 + optRho[i]]);
		optRho[i] -= maxThro; //去掉偏移maxThro
//x*cos(bstTheta)+y*sin(bstTheta)=bstThro => Ax+By+C=0
		A[i] = cos(optTheta[i] * PI / 180);
		B[i] = sin(optTheta[i] * PI / 180);
		C[i] = -optRho[i];
	}
	
}
// 一个简单的画出图像的函数
void bmpConverter::drawImg(const char * strDst, BYTE *pSrcImg, int w, int h)
{
	swap(pSrcImg, pImg);
	swap(w, width);
	swap(h, height);
	
	Img2Bmp(strDst, 8, 'k');

	swap(pSrcImg, pImg);
	swap(w, width);
	swap(h, height);

}

// 霍夫检测四边形，同时
void bmpConverter::HoughLine2()
{
	int PixelNum = width * height, pXSize = 2 * width + 2 * height;
	int maxThro = (int)sqrt(1.0*width*width + height * height + 0.5) + 1;
	BYTE *bMem = new BYTE[PixelNum + maxThro * 360];
	BYTE *pOrgImg = bMem, *pShowImg = pOrgImg + PixelNum;
	int *iMem = new int[width * height * 2 + 2 * pXSize + maxThro * 360];
	int *pX = iMem, *pY = pX + pXSize, num;
	int *pCount = pY + pXSize;
	double A[4], B[4], C[4];
	int i;

	//获得边缘
	ZEdgDetect(pImg, width, height, pX, pY, num);
	memset(pOrgImg, -1, sizeof(*pOrgImg) * PixelNum);
	for (i = 0; i < num; i++)
	{
		pOrgImg[pY[i] * width + pX[i]] = 0;
	}
	drawImg("./pic/_edg.bmp", pOrgImg, width, height);
	HoughLine(pImg, width, height, pCount, pX, pY, num);

	//transUI2B256(pCount, pShowImg, maxThro * 360);
	//drawImg("./pic/Gry.bmp", pShowImg, maxThro * 2, 180);

	// 最大值搜索
	hough_max_search2(pCount, maxThro, A, B, C);
	for (i = 0; i < 4; i++)
	{
		printf("A: %lf   B: %lf  c:%lf\n", A[i], B[i], C[i]);
		DrawABCLine(pImg, width, height, A[i], B[i], C[i], 0);
	}
		

	delete[] bMem;
	delete[] iMem;
}

// 计算dw
int wideCount(BYTE *pSrcImg, int *wCount, int width, int height)
{
	BYTE *pCur = pSrcImg, optW = 10;
	int x, y, prex = 0, mxDst = -1, dst;
	int i;

	memset(wCount, 0, sizeof(*wCount) * (max)(width, height));

	for (y = 0; y < height; y++, pCur += width)
	{
		prex = -1;
		for (x = 0; x < width; x++)
		{
			if (pCur[x])
			{
				if (prex != -1)
				{
					dst = x - prex;
					wCount[dst]++;
					if (dst > mxDst)
						mxDst = dst;
					
				}
					
				prex = x;
			}
		}
	}
	for (i = 2; i < mxDst; i++)
	{
		if (wCount[i] > wCount[i - 1])
		{
			optW = i;
			break;
		}
	}
	return optW;
}

// 得到相关点
int getCorPoint(BYTE *pSrcImg, int width, int height, int *pX, int *pY, int optW)
{
	BYTE *pCur = pSrcImg;
	int x, y, prex = 0, mxDst = -1, dst;
	int *pCurX = pX, *pCurY = pY;
	int cnt = 0;
	for (y = 0; y < height; y++, pCur += width)
	{
		prex = -999;
		for (x = 0; x < width; x++)
		{
			if (pCur[x])
			{
				if ((x - prex - optW) < 4 && (x - prex - optW) > -4)
				{
					cnt++;
					*pCurX++ = prex;
					//*pCurX++ = x;
					//*pCurY++ = y;
					*pCurY++ = y;
				}
				prex = x;
			}
		}
	}
	return cnt;
}

void getMostSuitableTrack(int *pLineCount, int maxRho, int optW, double *A, double *B, double *C)
{
	int *pCur;
	int maxCount, bstTheta, bstThro;
	int threThro = 30, threTheta = 5, mxTho, mnTho;
	int theta, thro, i, deg, gap;
	// 最大值搜索
	maxCount = bstTheta = bstThro = 0;
	for (theta = 0, pCur = pLineCount; theta < 180; theta++)
	{
		for (thro = 0; thro < maxRho * 2; thro++, pCur++)
		{
			if (*pCur> maxCount)
			{
				maxCount = *pCur;
				bstTheta = theta;
				bstThro = thro;

			}
		}
	}
	cout << maxCount << endl;
	// 排出最优直线周围直线
	mnTho = (max)(0, bstThro - threThro), mxTho = (min)(2 * maxRho, bstThro + threThro);
	for (deg = 180 + bstTheta - threTheta, pCur = pLineCount + deg * 2 * maxRho;
		deg <= 180;
		deg++, pCur += 2 * maxRho)
	{
		for (i = maxRho * 2 + bstThro - threThro; i <= maxRho * 2; i++)
			pCur[i] = 0;
		for (i = mnTho; i <= mxTho; i++)
			pCur[i] = 0;
		for (i = 0; i <= bstThro + threThro - maxRho * 2; i++)
			pCur[i] = 0;
	}
	for (deg = (max)(0, bstTheta - threTheta), pCur = pLineCount + deg * 2 * maxRho;
		deg <= (min)(180, bstTheta + threTheta);
		deg++, pCur += 2 * maxRho)
	{
		for (i = maxRho * 2 + bstThro - threThro; i <= maxRho * 2; i++)
			pCur[i] = 0;
		for (i = mnTho; i <= mxTho; i++)
			pCur[i] = 0;
		for (i = 0; i <= bstThro + threThro - maxRho * 2; i++)
			pCur[i] = 0;
	}
	for (deg = 0, pCur = pLineCount + deg * 2 * maxRho;
		deg <= bstTheta + threTheta - 180;
		deg++, pCur += 2 * maxRho)
	{
		for (i = maxRho * 2 + bstThro - threThro; i <= maxRho * 2; i++)
			pCur[i] = 0;
		for (i = mnTho; i <= mxTho; i++)
			pCur[i] = 0;
		for (i = 0; i <= bstThro + threThro - maxRho * 2; i++)
			pCur[i] = 0;
	}


	pLineCount[bstTheta * maxRho * 2 + bstThro] = 0;

	bstThro -= maxRho; //去掉偏移maxRho
	//x*cos(bstTheta)+y*sin(bstTheta)=bstThro => Ax+By+C=0
	*A = cos(bstTheta * PI / 180);
	*B = sin(bstTheta * PI / 180);
	*C = -bstThro;
}

void bmpConverter::air_port_track_dect()
{
	const int PixelNum = width * height, pXSize = 2 * width + 2 * height;
	const int maxRho = (int)sqrt(1.0*width*width + height * height + 0.5) + 1;
	const int mxRho2 = maxRho * 2;
	int *iMem = new int[(max)(width, height) + mxRho2 * 180 + PixelNum];
	BYTE *bMem = new BYTE[PixelNum];
	BYTE *pShowImg = bMem;
	int *pWCount = iMem, *pLineCount = pWCount + (max)(width, height);
	int *pX = pLineCount + mxRho2 * 180, *pY = pX + PixelNum / 2, pointNum;
	double A[4], B[4], C[4];
	int optW = 0;
	int i;

	// 计算dw
	optW = wideCount(pImg, pWCount, width, height);

	for (i = 0; i < 100; i++)
	{
		printf("pwCount%d: %d\n", i, pWCount[i]);
	}
	printf("optW: %d\n", optW);
	optW = 23;
	 //pWCount[optW];
	pointNum = getCorPoint(pImg, width, height, pX, pY, optW);
	memset(pShowImg, -1, sizeof(*pShowImg) * PixelNum);
	for (i = 0; i < pointNum; i++)
	{
		pShowImg[width * pY[i] + pX[i]] = 0;
	}
	drawImg("./pic/dot.bmp", pShowImg, width, height);
	// 得到直线的counter
	HoughLine(pImg, width, height, pLineCount, pX, pY, pointNum);
	// 求最合适直线
	for (i = 0; i < 4; i++)
	{
		getMostSuitableTrack(pLineCount, maxRho, optW, A + i, B + i, C + i);
		DrawABCLine(pImg, width, height, A[i], B[i], C[i], 254);
		DrawABCLine(pShowImg, width, height, A[i], B[i], C[i], 254);
		printf("A: %lf B: %lf  C: %lf\n", A[i], B[i], C[i]);
		// 计算右侧的直线
		A[i + 1] = A[i];
		B[i + 1] = B[i];
		C[i + 1] = C[i++] - A[i] * optW;
		printf("A: %lf B: %lf  C: %lf\n", A[i], B[i], C[i]);
		DrawABCLine(pImg, width, height, A[i], B[i], C[i], 254);
		DrawABCLine(pShowImg, width, height, A[i], B[i], C[i], 254);
	}
	drawImg("./pic/dst2.bmp", pShowImg, width, height);
	
}

void bmpConverter::hw6_draw(int x, int y)
{
	int PixelNum = width * height;
	BYTE *pCur = pImg + width / 2;
	int theta = -90, rho;

	memset(pImg, -1, sizeof(*pImg) * PixelNum);
	for (theta = -90; theta <= 90; theta++, pCur += 2 * width)
	{
		rho = sin(theta * PI / 180) * y + cos(theta * PI / 180) * x;
		pCur[rho] = 0;
	}
}

// 获得x，y的均值
void getMidXY(BYTE *pSrcImg, int *pCntX, int *pCntY, int width, int height, int &CX, int &CY)
{
	int *pCnt;
	BYTE *pCur, *pY, *pX;
	int x, y;

	pCnt = pCntX;
	memset(pCnt, 0, sizeof(*pCnt) * width);
	for (y = 0, pY = pSrcImg; y < height; y++, pY += width)
	{
		pCur = pY;
		int tx1 = 0, tx2 = 0;
		for (x = 0; x < width; x++, pCur++)
		{
			if (!*pCur)
			{
				tx1 = x;
				break;
			}
		}  // x
		pCur = pY + width - 1;
		for (x = width - 1; x >= 0; x--, pCur--)
		{
			if (!*pCur)
			{
				tx2 = x;
				break;
			}
		}  // x
		pCnt[(tx1 + tx2 + 1) >> 1]++;
	}  // y
	// 得到最大值
	CX = 0;
	for (x = 1; x < width; x++)
	{
		if (pCnt[x] > pCnt[CX])
			CX = x;
	}
	pCnt = pCntY;
	memset(pCnt, 0, sizeof(*pCnt) * height);
	for (x = 0, pX = pSrcImg; x < width; x++, pX += 1)
	{
		pCur = pX;
		int ty1 = 0, ty2 = height;
		for (y = 0; y < height; y++, pCur += width)
		{
			if (!*pCur)
			{
				ty1 = y;
				break;
			}
		}  // y
		pCur = pX + (height - 1) * width;
		for (y = height - 1; y >= 0; y--, pCur -= width)
		{
			if (!*pCur)
			{
				ty2 = y;
				break;
			}
		}  // y
		pCnt[(ty1 + ty2 + 1) >> 1]++;
	}  // x
	CY = 0;
	for (y = 1; y < height; y++)
	{
		if (pCnt[y] > pCnt[CY])
			CY = y;
	}
}

void DrawCircle(BYTE *pGryImg, int width, int height, int x0, int y0, int r, int color)
{
	int theta, x, y;

	for (theta = 0; theta < 360; theta++) //步长为1度
	{
		x = x0 + (int)(r*cos(theta*3.1415926 / 180));
		y = y0 + (int)(r*sin(theta*3.1415926 / 180));
		if ((x >= 0) && (x < width) &&
			(y >= 0) && (y < height)
			)
		{
			*(pGryImg + y * width + x) = color;
		}
	}
	return;
}


void DrawData2ImgCol(int *pData, int nData,
	double maxScale,
	BYTE *pGryImg, int width, int height,
	int color
)
{   //画在每列中
	int x1, x2, y1;
	int maxV, i;
	int x, y;

	// step.1-------------寻找最大值------------------------//
	maxV = 0;
	for (i = 0; i < nData; i++)
	{
		maxV = max(maxV, pData[i]);
	}
	maxV = (int)(maxV*maxScale);
	// step.2-------------像素填充--------------------------//
	for (i = 0; i < nData; i++)
	{
		y1 = (height - 1) - pData[i] * (height - 1) / (maxV + 1);
		y1 = max(0, y1);
		x1 = i * width / nData;
		x2 = (i + 1)*width / nData;
		for (x = x1; x < x2; x++)
		{
			for (y = height - 1; y > y1; y--) *(pGryImg + y * width + x) = color;
		}
	}
	// step.3-------------结束------------------------------//
	return;
}

void bmpConverter::DCCircle(int &CX, int &CY, int &mR)
{
	int sumX = 0, sumY = 0, pXSize = 2 * width + 2 * height, PixelNum = width * height;
	int mxR = (int)(sqrt(width * width + height * height) / 2) + 1;
	int *iMem = new int[width + height + mxR + pXSize * 2];
	BYTE *bMem = new BYTE [width * height];
	BYTE *pShowImg = bMem;
	int *pCntX = iMem, *pCntY = pCntX + width, *pCntR = pCntY + height, *pX = pCntR + mxR, *pY = pX + pXSize;
	//int *LUT = pY + pXSize;
	int num = 0;
	double maxScale = 0.5;
	int x, y, i;
	// 求中心CX, CY
	getMidXY(pImg, pCntX, pCntY, width, height, CX, CY);
	//getMidXY(pImg, pCntX, pCntX, width, height, CX, CY);
	// 求边缘点
	ZEdgDetect(pImg, width, height, pX, pY, num);
	memset(pCntR, 0, sizeof(*pCntR) * mxR);
	for (i = 0; i < num; i++)
	{
		int r = sqrt((pX[i] - CX) * (pX[i] - CX) + (pY[i] - CY) * (pY[i] - CY)) + 0.5;
		pCntR[r]++;
	}
	mR = 0;
	for (i = 1; i < num; i++)
	{
		if (pCntR[i] > pCntR[mR])
		{
			mR = i;
		}
	}
	// 展示PX
	memset(pShowImg, -1, sizeof(*pShowImg) * PixelNum);
	DrawData2ImgCol(pCntX, width, maxScale, pShowImg, width, height, 0);
	drawImg("./pic/_px1.bmp", pShowImg, width, height);
	// 展示PY
	memset(pShowImg, -1, sizeof(*pShowImg) * PixelNum);
	DrawData2ImgCol(pCntY, height, maxScale, pShowImg, height, width, 0);
	drawImg("./pic/_py1.bmp", pShowImg, height, width);
	// 展示R
	memset(pShowImg, -1, sizeof(*pShowImg) * PixelNum);
	DrawData2ImgCol(pCntR, mxR, maxScale, pShowImg, mxR, height, 0);
	drawImg("./pic/_R.bmp", pShowImg, mxR, height);

	DrawCircle(pImg, width, height, CX, CY, mR, 254);

	delete[] iMem;

}

void setImageBoundary(BYTE *pBinImg, int width, int height, BYTE color)
{
	BYTE *pRow;
	int y;

	memset(pBinImg, color, width);
	memset(pBinImg + (height - 1)*width, color, width);
	for (y = 0, pRow = pBinImg; y < height; y++, pRow += width)
	{
		*pRow = color;
		*(pRow + width - 1) = color;
	}
	return;
}

void bmpConverter::chainCode_fill()
{
	const int PixelNum = width * height, MAX_CONOUR_LENGTH = PixelNum;
	BYTE *bMem = new BYTE[PixelNum * 2];
	BYTE *pChainCode = bMem, *pShowImg = pChainCode + PixelNum;
	setImageBoundary(pImg, width, height, 0);
	getChainCode_fill(pImg, pShowImg, pChainCode, width, height);
	drawImg("./pic/_trace_fill.bmp", pShowImg, width, height);
	delete[] bMem;
}

// 快速3x3膨胀
void bmpConverter::fExpand33()
{
	const int PixelNum = width * height, MAX_CONOUR_LENGTH = PixelNum;
	BYTE *bMem = new BYTE[PixelNum * 2];
	BYTE *pChainCode = bMem, *pShowImg = pChainCode + PixelNum;
	int i;

	setImageBoundary(pImg, width, height, 0);
	getChainCode_fill(pImg, pShowImg, pChainCode, width, height);
	drawImg("./pic/_trace_fill.bmp", pShowImg, width, height);
	//可用于3x3的快速膨胀
	for (i = 0; i < width*height; i++)
	{
		pShowImg[i] = (pImg[i] <= 200) - 1;
	}
	drawImg("./pic/_expand.bmp", pShowImg, width, height);
	delete[] bMem;
}
// 快速3x3腐蚀
void bmpConverter::fcorrosion33()
{
	const int PixelNum = width * height, MAX_CONOUR_LENGTH = PixelNum;
	BYTE *bMem = new BYTE[PixelNum * 2];
	BYTE *pChainCode = bMem, *pShowImg = pChainCode + PixelNum;
	int i;

	setImageBoundary(pImg, width, height, 0);
	getChainCode_fill(pImg, pShowImg, pChainCode, width, height);
	//drawImg("./pic/_trace_fill.bmp", pShowImg, width, height);
	//可用于3x3的快速膨胀
	for (i = 0; i < width*height; i++)
	{
		pShowImg[i] = (pImg[i] != 255) - 1;
	}
	drawImg("./pic/_corrosion.bmp", pShowImg, width, height);
	delete[] bMem;
}


void bmpConverter::test()
{
	int *imem = new int[512];
	int *hist = imem, *scale_hist = hist + 256;
	int thres, pixelNum = width * height;

	memset(scale_hist, 0, sizeof(int) * 256);
	getHist8b(pImg, pixelNum, hist);
	for (int i = 0; i < 256; i++)scale_hist[i / 4] += hist[i];
	thres = getOtsuThreshold8b(scale_hist, 256);
	cout << "thres: "<<thres << endl;

	PGry2Bin(pImg, pixelNum, thres, pImg);
}


