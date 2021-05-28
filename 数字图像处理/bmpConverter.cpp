#include "bmpConverter.h"

#include <cmath>
#define BTYE BYTE
using namespace std;
const double pi = 3.1415926535;
bmpConverter::bmpConverter(const char * orgfile)
{
	if (!BmpFile2Img(orgfile))
	{
		puts("�ļ���ȡʧ��");
	}
	else
	{
		printf("�Ѷ�ȡ�ļ� %s\n", orgfile);
	}
}

bmpConverter::bmpConverter():Img()
{
}

// �Ҷ�ͼ��ת
void bmpConverter::InvertImg()
{
	
	//BYTE *pCur, *pEnd = pImg + width * height * channel;
	//for (pCur = pImg; pCur < pEnd; pCur++) *pCur = ~ *pCur;
	int sum = width * height * channel;
	int res = sum - sum / 4 * 4;
	int *pCur= (int *)pImg, *pEnd = (int *)(pImg + sum - res);
	while(pCur < pEnd) *(pCur++) = ~*pCur;
	BYTE * pRes = (BYTE *)pEnd;
	// ������һ��4���ֽڵ��������ֽ�ȡ��
	while(res--)*(pRes++) = ~*(pRes);
	return;
}

// ��ȡbmp
bool bmpConverter::BmpFile2Img(const char * DstFile)
{
	
	delete pImg;
		
	//BmpHeader��¼����ʵ�Ŀ��,
	//����������ʵ�Ŀ��,ȥ����4�������չ.
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
	printf("��ȡ%dͨ��ͼ��, ÿ�����ر�����%d\n", channel, int(BmpHeader.biBitCount));
#endif // DEBUG
	// �޷�����1,4bit������
	w = w * channel; // ��ʵ�ı�����
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
			//�������������
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
// ����bmp
bool bmpConverter::Img2Bmp(const char * DstFile, int bitCnt, char RGB_MOD)
{
	if (pImg == nullptr)
	{
		// δ��ȡͼ��
		puts("����ʧ�ܣ�δ��ȡͼ��");
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
		puts("�޷������λ��");
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
// �õ���ɫ��
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
			
			board[p++] = 255 - i;
			board[p++] = i;
			board[p++] = i; //blue,green,red, alpha
			board[p++] = 0;
			//if (fwrite((void *)p, 1, 4, fp) != 4) { Suc = false; break; }
		}
	}


}
// 8bit תbmp
bool bmpConverter::Img28bitBmp(const char * DstFile, char mod)
{
#ifdef DEBUG
	printf("���ڱ���8bit ͼ�� %s\n", DstFile);
#endif // DEBUG
	//BYTE * temp_save=nullptr;
	if (channel == 3)
	{
		RGB2Gry();
	}

	//BmpHeader��¼����ʵ�Ŀ��
	//��ÿ���ֽڸ�������4�ı���ʱ,��Ҫ����4����
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
	if (fwrite((void *)&BmpHeader, 1, sizeof(BmpHeader), fp) != sizeof(BmpHeader)) Suc = false;
	// write Pallete

	// ����modԤ����ɫ
	BYTE board[256 * 4];
	getBord(board, mod);
	if (fwrite((void *)board, 1, 4 * 256, fp) != 4 * 256) { Suc = false;}
	// write image data
	extend = (width + 3) / 4 * 4 - width; //ÿ���ֽڸ�����Ҫ��4�ı�������
	if (extend == 0)
	{
		for (pCur = pImg + (height - 1)*width; pCur >= pImg; pCur -= width)
		{
			if (fwrite((void *)pCur, 1, width, fp) != (unsigned int)width) Suc = false; //��ʵ������
		}
	}
	else
	{
		for (pCur = pImg + (height - 1)*width; pCur >= pImg; pCur -= width)
		{
			if (fwrite((void *)pCur, 1, width, fp) != (unsigned int)width) Suc = false; //��ʵ������
			//ÿ�а�4�ֽڶ����������
			if (fwrite((void *)(pCur + width - 1), 1, extend, fp) != extend) Suc = false;
			
		}
	}
	// return;
	fclose(fp);
	if (Suc)printf("%s ����ɹ�", DstFile);
	return Suc;
}
// 24bitתbmp
bool bmpConverter::Img224bitBmp(const char * DstFile)
{
	//BmpHeader��¼����ʵ�Ŀ��
//��ÿ���ֽڸ�������4�ı���ʱ,��Ҫ����4����
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

	extend = ((w + 3) / 4 * 4) - w; //ÿ���ֽڸ�����Ҫ��4�ı�������
	if (!extend)
	{
		for (pCur = pImg + (height - 1) * w; pCur >= pImg; pCur -= w)
		{
			if (fwrite((void *)pCur, 1, w, fp) != (unsigned int)(w)) Suc = false; //��ʵ������
		}
	}
	else
	{
		for (pCur = pImg + (height - 1) * w; pCur >= pImg; pCur -= w)
		{
			if (fwrite((void *)pCur, 1, w, fp) != (unsigned int)(w)) Suc = false; //��ʵ������
			 //���������
			if (fwrite((void *)(pCur + w- 3), 1, extend, fp) != extend) Suc = false;
		}
	}
	// return;
	fclose(fp);
	if (Suc)printf("%s ����ɹ�", DstFile);
	return Suc;
}


//________homework2�Ĵ���__________________________

// 14bitֱ��ͼ��һ��
void bmpConverter::PHistogramEqualize14bit(short * pRawImg)
{
	int hist[1 << 14], sum = width * height, A = 0;
	pImg = new BYTE[sum];
	BYTE LUT[1 << 14], * pCur = pImg;
	// �õ�ֱ��ͼ
	short * spCur = pRawImg, *spDes = pRawImg + sum;
	for (int i = 0; i < (1 << 14); i++) hist[i] = 0;
	while (spCur < spDes) hist[*(spCur++)]++;
	for (int i = 0; i < (1 << 14); i++)LUT[i] = 255 * (A += hist[i]) / sum;
	for (spCur = pRawImg; spCur < spDes;)*(pCur++) = LUT[*(spCur++)];

}
// 14bitֱ��ͼ��һ��2
void bmpConverter::PHistogramEqualize14bit2(short * pRawImg)
{
	int hist[1 << 14], sum = width * height, A = 0;
	pImg = new BYTE[sum];
	BYTE LUT[1 << 14], *pCur = pImg;
	// �õ�ֱ��ͼ
	short * spCur = pRawImg, *spDes = pRawImg + sum;
	for (int i = 0; i < (1 << 14); i++) hist[i] = 0;
	while (spCur < spDes) hist[*(spCur++)]++;
	int maxp = (1 << 14) - 1, minp = 0;// ����С�ҶȺ����Ҷ�
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

// ��ȡ14bit raw���ݣ��Ѿ�д��
bool bmpConverter::read14bitRaw(const char * DstFile)
{
	delete pImg;

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



// RGBת�Ҷ�ͼ
void bmpConverter::RGB2Gry(bool table_chk, bool inplace)
{
	if (channel != 3 || !pImg)return;
	int sum = width * height;
	BYTE * temp_save = new BYTE[sum];
	if (!temp_save)
	{
#ifdef DEBUG
		puts("��������ڴ�����ʧ��");
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

// �������죨����任��
void bmpConverter::PAffine(double k, double b)
{
	if (!pImg)return;
	BYTE LUT[256], * pCur = pImg, *pDes = pImg + width * height * channel;
	for (int i = 0; i < 256; i++)LUT[i] = (std::min)(255.0, (std::max)(0.0, k * i + b));
	while (pCur < pDes)*(pCur++) = LUT[*pCur];


}
// 8bit��ֵ��׼��涨��
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
// �õ�8bit��ֱ��ͼ
void bmpConverter::getHistGram8bit(int * hist)
{
	for (int i = 0; i < 256; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height;pCur < pDest;)
		hist[*(pCur++)]++;
}
// 8bitֱ��ͼ��׼��ʵ��
void bmpConverter::PHistogramEqualize8bit()
{
	if (BmpHeader.biBitCount != 8)
	{ 
#ifdef DEBUG
		puts("ͼ��λ������");
#endif // DEBUG
		return; 
	}
	int hist[256], LUT[256], A = 0, sum = width * height;
	getHistGram8bit(hist);
	for (int i = 0, A = hist[1]; i < 256; i++)LUT[i] = 255 * (A += hist[i]) / sum;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height; pCur < pDest;)
		*(pCur++) = LUT[*pCur];

}
// �õ�24bitֱ��ͼ
void bmpConverter::getHistGram24bit(int * hist)
{
	for (int i = 0; i < 256*3; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, p = 0, *pDest = pImg + width * height * 3; pCur < pDest; p = (p + 1)%3)
		hist[*(pCur++) * 3 + p]++;
}
// 24bitֱ��ͼ��׼��ʵ�֣�����ͨ���ֱ�ʵ�֣�
void bmpConverter::PHistogramEqualize24bit()
{
	if (BmpHeader.biBitCount != 24)
	{
#ifdef DEBUG
		puts("ͼ��λ������");
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
// 24bitֱ��ͼ��׼��ʵ�֣�����ͨ����һ��ʵ�֣�
void bmpConverter::PHistogramEqualize24bit1()
{
	if (BmpHeader.biBitCount != 24)
	{
#ifdef DEBUG
		puts("ͼ��λ������");
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
// �õ�24bit��ƽ��ֱ��ͼ
void bmpConverter::getHistGram24bitavg(int * hist)
{
	for (int i = 0; i < 256; i++)hist[i] = 0;
	for (BYTE * pCur = pImg, *pDest = pImg + width * height * 3; pCur < pDest;)
		hist[(*(pCur++) + *(pCur++) + *(pCur++)) / 3]++;

}
// 24bitֱ��ͼ��׼��ʵ�֣�����ͨ���ľ�ֵʵ�֣�
void bmpConverter::PHistogramEqualize24bit2()
{
	if (BmpHeader.biBitCount != 24)
	{
#ifdef DEBUG
		puts("ͼ��λ������");
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

void bmpConverter::RGB2Gry2(bool table_chk, bool inplace)
{
	if (channel != 3 || !pImg)return;
	int sum = width * height;
	BYTE * temp_save = new BYTE[sum];
	if (!temp_save)
	{
#ifdef DEBUG
		puts("��������ڴ�����ʧ��");
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


// ��ά��ֵ�˲����л���
void bmpConverter::AavgFilter2d8bit_col_cal(int m, int n, BYTE  * pDes)
{
	BYTE *pAdd, *pDel, *pRes;
	int halfx, halfy;
	int x, y;
	int sum, c;
	int sumCol[1<<15]; //Լ��ͼ���Ȳ�����2^15

	// step.1------------��ʼ��--------------------------//
	m = m | 1; //������
	n = n | 1; //������
	halfx = m / 2; //�˲����İ뾶x
	halfy = n / 2; //�˲����İ뾶y
	c = (1 << 23) / (m * n); //�˷�����
	memset(sumCol, 0, sizeof(int) * width);
	for (y = 0, pAdd = pImg; y <= halfy; y++)
	{
		for (x = 0; x < width; x++) sumCol[x] += *(pAdd++);
	}
	// step.2------------�˲�----------------------------//
	for (y = 0, pRes = pDes, pDel = pImg; y < height; y++)
	{
		for (sum = 0, x = 0; x <= halfx; x++) sum += sumCol[x];
		//�˲�
		//pRes += halfx; //�������
		for (x = 0; x < width; x++)
		{
			//��ҶȾ�ֵ
			*(pRes++) = sum * c >> 23; //�������˷�����λ�������
			//����,���»ҶȺ�
			if(x >= halfx)sum -= sumCol[x - halfx]; //�������
			if(x < width - halfx )sum += sumCol[x + halfx + 1]; //���ұ���
		}
		//pRes += halfx;//�����Ҳ�
		//����,����sumCol
		if (y >= halfy)
		{
			for (x = 0; x < width; x++)sumCol[x] -= *(pDel++); //����һ��
		}
		if (y < height - halfy - 1)
		{
			for (x = 0; x < width; x++)sumCol[x] += *(pAdd++); //����һ��
		}
	}
	return;
}
// ���㲹���˲�����ȵĻ���ͼ
void bmpConverter::getCalGraph(int m, int n, int * pSum)
{
	BYTE *pCur ;
	int *pRes;
	int x, y, halfx, halfy, w, i, rowsum;
	halfx = m / 2;
	halfy = n / 2;
	pCur = pImg;
	pRes = pSum;
	w = width + halfx * 2 + 1; // ���ڻ���ͼ������������Ҫ���1
	// �ϲ�padding
	for (i = 0; i < (halfy+ 1) * w; i++) *(pRes++) = 0;
	for (y = 0; y < height; y++)
	{
		// ���padding
		for (x = 0; x < halfx + 1; x++) *pRes++ = 0;
		// ��������
		for (x = 0, rowsum = 0; x < width; x++)
		{
			rowsum += *pCur++;
			*(pRes++) = *(pRes - w) + rowsum;

		}
		// �Ҳ�padding
		for (x = 0; x < halfx; x++)*pRes++ = *(pRes - 1);
#ifdef DEBUG
		/*printf("%d %d\n", *(pRes - 1));*/
#endif // DEBUG
	}
	// �²�padding
	for (i = 0; i < (halfy) * w; i++) *(pRes++) = *(pRes - w);
	return;
}

// 2d��ֵ�˲� ʹ�û���ͼ
void bmpConverter::AavgFilter2d8bit_calGraph(int * pSum, int m, int n, BYTE * pDes)
{
	int *pY1, *pY2;
	BYTE *pRes;
	int halfx, halfy, w;
	int y, x1, x2;
	int sum, c;
	// step.1------------��ʼ��--------------------------//
	m = m | 1; //������
	n = n | 1; //������
	halfx = m / 2; //�˲����İ뾶x
	halfy = n / 2; //�˲����İ뾶y
	w = width + 2 * halfx + 1;
	c = (1 << 23) / (m * n); //�˷�����
	// step.2------------�˲�----------------------------//
	for (
		y = 0, pRes = pDes, pY1 = pSum, pY2 = pSum + (n) * w;
		y < height;
		y++, pY1 += w, pY2 += w
		)
	{
		for (x1 = 0, x2 = m; x1 < width; x1++, x2++) //���Լ���ˣ�����̫���׶�
		{
			sum = *(pY2 + x2) - *(pY2 + x1) - *(pY1 + x2) + *(pY1 + x1);
			*(pRes++) = (sum * c) >> 23; //�������˷�����λ�������
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
	// step.3------------����----------------------------//
	return;
}
/*
��ά��ֵ�˲�
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
		puts("δ����ͼ���޷�����");
#endif // DEBUG
		return Img();
	}
	if (channel != 1)
	{
#ifdef DEBUG
		puts("���ǵ�ͨ��ͼ��");
#endif // DEBUG
		return Img();
	}
	if (m > width || n > height)
	{
#ifdef DEBUG
		puts("�˲�����С����");
#endif // DEBUG
		return Img();
	}
	BYTE* pSrc = new BYTE[width * height];
	switch (mod)
	{
	default:
	case 'c':
	case 'C':
		AavgFilter2d8bit_col_cal(m, n, pSrc);
		break;
	case 'G':
	case 'g':
	{
		int * pSum = new int[(width + (m | 1)) * (height + (n | 1))];
		getCalGraph(m, n, pSum);
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

/* ��ͨ��ͼ��ת
* mod:  'n':����
		's':ʹ��SSE

*/
Img bmpConverter::PInvert1c(char mod, bool inplace)
{
	if (channel != 1)
	{
#ifdef DEBUG
		puts("�ǵ�ͨ�����޷�ִ��");
#endif // DEBUG
		return Img();
	}
	_declspec(align(16)) BYTE * pDes = new BYTE[((width * height) | 15) + 1];// ����16��������
	switch (mod)
		{
		default:
		case 'n':
		case 'N':
		case '0':
			PInvert8bit(pDes);
			break;
		case 's':
		case 'S':
		case '1':
			PInvert8bit_SSE(pDes);
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


void bmpConverter::PInvert8bit(BYTE * pDes)
{
	BYTE * pCur = pImg, * pEnd = pImg + width *height, *pRes = pDes;
	while (pCur < pEnd) *(pRes++) = ~*pCur++;
}



void bmpConverter::PInvert8bit_SSE(BYTE * pDes)
{
	BYTE * pCur, * pRes;
	__m128i * pCurSSE, *pDesSSE;
	__m128i inver;
	int res, t;
	inver = _mm_set_epi32(-1, -1, -1, -1);
	res = width * height & 15, t = width * height / 16;
	pCur = pImg;
	pCurSSE = (__m128i *)pCur;
	pDesSSE = (__m128i *)pDes;

	while (t--)*pDesSSE++ = _mm_sub_epi32(inver, *pCurSSE++);
	pCur = pImg + width * height, pRes = pDes + width * height;
	while (res--) *(pRes--) = ~*pCur--;
}

Img bmpConverter::AMedianFilter2d1c(int m, int n, bool inplace, char mod)
{
	if (!pImg)
	{
#ifdef DEBUG
		puts("δ����ͼ���޷�����");
#endif // DEBUG
		return Img();
	}
	if (channel != 1)
	{
#ifdef DEBUG
		puts("���ǵ�ͨ��ͼ��");
#endif // DEBUG
		return Img();
	}
	if (m > width || n > height)
	{
#ifdef DEBUG
		puts("�˲�����С����");
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
			AMedianFilter8bit_col_cal(m, n, pSrc);
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

void bmpConverter::AMedianFilter8bit_col_cal(int m, int n, BYTE * pDes)
{
	BYTE *pCur, *pRes;
	int halfx, halfy, x, y, i, j, y1, y2, res;
	int histogram[256];
	int wSize, j1, j2;
	int num, med, v;
	int dbgCmpTimes = 0; //������ֵ����Ƚϴ����ĵ���

	m = m | 1;  // ������
	n = n | 1;  // ������
	halfx = m / 2;  // x�뾶
	halfy = n / 2;  // y�뾶
	wSize = m * n;  // �����������ܸ���
	for (y = 0, pRes = pDes; y < height; y++)
	{
		// step.1----��ʼ��ֱ��ͼ
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
		//step.2-----��ʼ����ֵ
		num = 0; //��¼�ŻҶ�ֵ��0����ֵ�ĸ���
		for (i = 0; i < 256; i++)
		{
			num += histogram[i];
			if (num * 2 > wSize)
			{
				med = i;
				break;
			}
		}
		// �˲�
		// pRes += halfx; // û�д���ͼ����߽�������
		res = n - y2 + y1;
		for (x = 0; x < width; x++)
		{
			// ��ֵ
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

			// step.3-----ֱ��ͼ����: ��ȥ��ǰ��������ߵ�һ��,��������Ҳ��һ������
			j1 = x - halfx;  // �������
			j2 = x + halfx + 1;  // �ұߵ�����
			if (j1 < 0)  // �����߳�����
			{
				histogram[0] -= n;
				num -= n;
			}
			else
			{
				for (i = y1, pCur = pImg + y1 * width; i < y2; i++, pCur += width)
				{
					// ��ȥ�������
					v = pCur[j1];
					histogram[v]--;  //����ֱ��ͼ
					if (v <= med) num--; //����num
				}
				histogram[0] -= res; // ����padding����
				num -= res;
			}
			if (j2 >= width)  // ����ұ߳�����
			{
				histogram[0] += n;
				num += n;
			}
			else
			{
				for (i = y1, pCur = pImg + y1 * width; i < y2; i++, pCur += width)
				{
					// �������ұ���
					v = pCur[j2];
					histogram[v]++;  // ����ֱ��ͼ
					if (v <= med) num++; // ����num
				}
				histogram[0] += res; // ����padding����
				num += res;
			}
			// step.4-----������ֵ
			if (num * 2 < wSize) // ���ϴ���ֵmed�ĸ���������,��medҪ���
			{
				for (med = med + 1; med < 256; med++)
				{
					dbgCmpTimes += 2; //�ܵıȽϴ���,������
					num += histogram[med];
					if (num * 2 >= wSize) break;
				}
				dbgCmpTimes += 1; //�ܵıȽϴ���,������
			}
			else //���ϴ���ֵmed�ĸ�������,��medҪ��С
			{
				while ((num - histogram[med]) * 2 > wSize)//����ȥ��,�Ա�С
				{
					dbgCmpTimes++; //�ܵıȽϴ���,������
					num -= histogram[med--];
				}
				dbgCmpTimes += 2; //�ܵıȽϴ���,������
			}
		}
		//pRes += halfx;//û�д���ͼ���ұ߽�������
	}
	printf("dbg: %lf \n", 1.0 * dbgCmpTimes / width / height);
	return;
}

void bmpConverter::AGuassFilter1d(BYTE * pSrc, int width, int height, int * pFilter, int m, BYTE * pDes)
{
	int offset = 17;  
	// ƫ��������Ϊ17��ԭ�򣬽���ͺ���λ�ƣ���Ϊ��λ�ƻ���ʧ���Ⱥܶ�Σ����ձ�����˵����50%�ļ�����ʧ1����
	// �����ۻ����� m/2��������ʧ��mΪ�˲����Ŀ���ʵ��֮�󣬷���PSҲ�۲쵽�����ƵĽ������8���ȣ������m=19���˲���
	// ƫ��16��ȱ�㣬�˲����ľ����½�������֮��ֻΪ��0.999931������ƽ��������128�㣨ʵ����ԭͼ��ƽ������ֻ��61����������ʧֻ��1����
	// �Ӻ���Ͽ��������23��offsetͼƬ���������������
	int halfm, y,x, f, sum = 0;
	BYTE  *pCur, *pRes, *pY;

	halfm = m / 2;

	for (y = 0, pY = pSrc, pRes = pDes; y < height; y++, pY += width)
	{
		// �������
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
		// �����м�
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
		// �����Ҳ�
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

Img bmpConverter::T(BYTE * &pSrc, int w, int h, bool inplace)
{
	BYTE * pDes , * pX, *pCur;
	int x, y;

	pDes = new BYTE[w * h];
	pCur = pDes;

	for (x = 0; x < w; x++)
	{
		for (y = 0, pX = pSrc + x; y < h; y++, pX += w)
		{
			*pCur++ = *pX;
		}
	}

	if (inplace)
	{
		if(pSrc == pImg)swap(width, height);
		delete pSrc;
		pSrc = pDes;
		return Img();
	}
	else
	{
		return Img(pDes, width, height);
	}
}


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

Img bmpConverter::AGuassFilter2d1c(double dev, int m, bool inplace, char mod)
{
	if (!pImg)
	{
#ifdef DEBUG
		puts("δ����ͼ���޷�����");
#endif // DEBUG
		return Img();
	}
	if (channel != 1)
	{
#ifdef DEBUG
		puts("���ǵ�ͨ��ͼ��");
#endif // DEBUG
		return Img();
	}
	if (m > width || m > height || m >= 80)
	{
#ifdef DEBUG
		puts("�˲�����С����");
#endif // DEBUG
		return Img();
	}
	BYTE* pSrc, * pSrc2;
	int pFilter[128];
	m |= 1;

	getGuassFilter(dev, m, pFilter);
	pSrc = new BYTE[width * height];
	pSrc2 = new BYTE[width * height];

#ifdef DEBUG
	double fsum = 0;
	int offset = 17;
	for (int i = 0; i < m; i++)
	{
		fsum += 1.0 * pFilter[i] / (1 << offset);
		printf("%lf \n", 1.0 * pFilter[i] / (1 << offset));
	}
	cout << fsum << endl;
#endif // DEBUG
	switch (mod)
	{
	default:
	case 'c':
	case 'C':
	{
		AGuassFilter1d(pImg, width, height, pFilter, m, pSrc);
		T(pSrc, width, height);
		AGuassFilter1d(pSrc, height, width, pFilter, m, pSrc2);
		T(pSrc2, height, width);
		delete pSrc;
		pSrc = pSrc2;
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