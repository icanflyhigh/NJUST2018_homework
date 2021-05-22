#include "bmpConverter.h"
#include <cmath>
#define BTYE BYTE
using namespace std;

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

bmpConverter::bmpConverter()
{

}

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

// 读取图片
bool bmpConverter::BmpFile2Img(const char * DstFile)
{
	// 如果原先指针不空，则回收内存
	if (pImg)
	{
		delete pImg;
	}
		
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
	if (pImg != nullptr)
	{
		delete pImg;
	}
}

bmpConverter::bmpConverter(bmpConverter & tb):
	width(tb.width), height(tb.height), channel(tb.channel),
	FileHeader(tb.FileHeader), BmpHeader(tb.BmpHeader)
{
	int sum = width * height * channel;
	if (sum)
	{
		pImg = new BYTE[sum];
		BYTE * from = tb.pImg, *to = pImg;
		for (int i = 0; i < sum; i++) *(to++) = *(from++);
	}
	else
	{
		pImg = nullptr;
	}
	
}

bmpConverter::bmpConverter(bmpConverter &&tb):
width(tb.width), height(tb.height), channel(tb.channel),
FileHeader(tb.FileHeader), BmpHeader(tb.BmpHeader)
{
	int sum = width * height * channel;
	if (sum)
	{
		pImg = new BYTE[sum];
		BYTE * from = tb.pImg, *to = pImg;
		for (int i = 0; i < sum; i++) *(to++) = *(from++);
	}
	else
	{
		pImg = nullptr;
	}

}


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
	int i, extend;
	bool Suc = true;
	BYTE p[4], *pCur;

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
	if (fwrite((void *)&BmpHeader, 1, sizeof(BmpHeader), fp) != sizeof(BmpHeader)) Suc = false;
	// write Pallete

	// 根据mod预设颜色
	BYTE tr = -1, tb = -1, tg = -1;
	if (mod == 'r') tb = 0, tg = 0;
	else if (mod == 'g') tr = 0, tb = 0;
	else if (mod == 'b') tr = 0, tg = 0;
	if (mod != 'k')
	{
	for (i = 0; i < 256; i++)
	{
		p[3] = 0;
		p[0] = i & tb;
		p[1] = i & tg;
		p[2] = i & tr; //blue,green,red
		if (fwrite((void *)p, 1, 4, fp) != 4) { Suc = false; break; }
	}
	}
	else
	{
		for (i = 0; i < 256; i++)
		{
			p[3] = 0;
			p[0] = 255 - i;
			p[1] = i;
			p[2] = i; //blue,green,red
			if (fwrite((void *)p, 1, 4, fp) != 4) { Suc = false; break; }
		}
	}

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
			for (i = 0; i < extend; i++) //每行按4字节对齐进行扩充
			{
				if (fwrite((void *)(pCur + width - 1), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);

	return Suc;
}

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
			for (i = 0; i < extend; i++) //扩充的数据
			{
				if (fwrite((void *)(pCur + w - 3 + 0), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	if (Suc)printf("%s 保存成功", DstFile);
	return Suc;
}


//________homework2的代码__________________________
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

// 这是一个完全写死的函数
bool bmpConverter::read14bitRaw(const char * DstFile)
{
	if (pImg != nullptr)
	{
		delete pImg;
	}
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




void bmpConverter::RGB2Gry(bool table_chk, bool inplace)
{
	if (channel != 3 || !pImg)return;
	int sum = width * height;
	BYTE * temp_save = new BYTE[sum];
	if (!temp_save)
	{
		puts("程序错误，内存申请失败");
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
		for (int i = 0; i < 256; i++)
		{
			r8[i] = 0.299 * i;
			g8[i] = 0.587 * i;
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

void bmpConverter::PAffine(double k, double b)
{
	if (!*pImg)return;
	BYTE LUT[256], * pCur = pImg, *pDes = pImg + width * height * channel;
	for (int i = 0; i < 256; i++)LUT[i] = min(255, max(0, k * i + b));
	while (pCur < pDes)*(pCur++) = LUT[*pCur];


}

void bmpConverter::P2avgstd8Bit(double mean, double stddev)
{
	if (BmpHeader.biBitCount != 8)return;
	long long  sum = 0 ; // int 的话，要求总像素数目要求少于16,777,216，方便起见用longlong
	int hist[256];
	getHistGram8bit(hist);
	int tot = width * height;
	double k, b;
	BYTE * pCur = pImg, *pDes = pImg + tot;
	for (int i = 0; i < 256; i++) sum += hist[i] * i;
	double avg = double(sum) / tot;
	pCur = pImg;
	double sdev = 0;
	for (int i = 0; i < 256; i++) sdev += hist[i] * (i - avg) * (i - avg);
	k = stddev / sqrt(sdev / (tot - 1));
	b = mean - avg * k;
	PAffine(k, b);
	return;
}

void bmpConverter::getHistGram8bit(int * hist)
{
	for (int i = 0; i < 256; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height;pCur < pDest;)
		hist[*(pCur++)]++;
}

void bmpConverter::PHistogramEqualize8bit()
{
	if (BmpHeader.biBitCount != 8)
	{ 
		puts("图像位数错误"); return; 
	}
	int hist[256], LUT[256], A = 0, sum = width * height;
	getHistGram8bit(hist);
	for (int i = 0, A = hist[1]; i < 256; i++)LUT[i] = 255 * (A += hist[i]) / sum;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height; pCur < pDest;)
		*(pCur++) = LUT[*pCur];

}

void bmpConverter::getHistGram24bit(int * hist)
{
	for (int i = 0; i < 256*3; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, p = 0, *pDest = pImg + width * height * 3; pCur < pDest; p = (p + 1)%3)
		hist[*(pCur++) * 3 + p]++;
}

void bmpConverter::PHistogramEqualize24bit()
{
	if (BmpHeader.biBitCount != 24)
	{
		puts("图像位数错误"); return;
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

void bmpConverter::PHistogramEqualize24bit1()
{
	if (BmpHeader.biBitCount != 24)
	{
		puts("图像位数错误"); return;
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

void bmpConverter::getHistGram24bitavg(int * hist)
{
	for (int i = 0; i < 256; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * 3; pCur < pDest;)
		hist[(*(pCur++) + *(pCur++) + *(pCur++)) / 3]++;

}

void bmpConverter::PHistogramEqualize24bit2()
{
	if (BmpHeader.biBitCount != 24)
	{
		puts("图像位数错误"); return;
	}
	int hist[256], LUT[256], sum = width * height * channel, A = 0, minp = 0;
	for (; !hist[minp]; minp++);
	minp /= 3;
	getHistGram24bitavg(hist);
	for (int i = 0, p = 0; i < 256; i++)
	{
		LUT[i] = 255 * (A += hist[p++]) / sum;
	}
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * channel; pCur < pDest;)
	{
		*(pCur++) = LUT[*pCur];//blue,green,red
		*(pCur++) = LUT[*pCur];//blue,green,red
		*(pCur++) = LUT[*pCur];//blue,green,red
	}
}



