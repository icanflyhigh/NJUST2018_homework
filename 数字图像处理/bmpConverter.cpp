#include "bmpConverter.h"

using namespace std;

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
	// ������һ��4���ֽڵ��������ֽ�ȡ��
	while(res--)*(pRes++) = ~*(pRes);
	return;
}

// ��ȡͼƬ
bool bmpConverter::BmpFile2Img(const char * DstFile)
{
	// ���ԭ��ָ�벻�գ�������ڴ�
	if (pImg)delete pImg;
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
		ret = Img2Bmp(DstFile, BmpHeader.biBitCount, RGB_MOD);
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
	if (pImg != nullptr)delete pImg;
}

bool bmpConverter::Img28bitBmp(const char * DstFile, char mod)
{
#ifdef DEBUG
	printf("���ڱ���8bit ͼ�� %s\n", DstFile);
#endif // DEBUG
	BYTE * temp_save=nullptr;
	if (channel == 3)
	{
		int sum = width * height;
		temp_save = new BYTE[sum];
		BYTE * p1 = temp_save, *p2 = pImg - 1;
		while (sum--)*(p1++) = (*(++p2) + *(++p2) + *(++p2)) / 3;
		swap(temp_save, pImg);
	}

	//BmpHeader��¼����ʵ�Ŀ��
	//��ÿ���ֽڸ�������4�ı���ʱ,��Ҫ����4����
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

	// ����modԤ����ɫ
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
			for (i = 0; i < extend; i++) //ÿ�а�4�ֽڶ����������
			{
				if (fwrite((void *)(pCur + width - 1), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	if (channel == 3)
	{
		delete pImg;
		pImg = temp_save;
	}

	return Suc;
}

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
			for (i = 0; i < extend; i++) //���������
			{
				if (fwrite((void *)(pCur + w - 3 + 0), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	if (Suc)printf("%s ����ɹ�", DstFile);
	return Suc;
}
