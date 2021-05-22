// author: M@
#include "bmpConverter.h"
#include <iostream>
#include <ctime>
using namespace std;


void hw2_time_cmp()
{
	bmpConverter bmpCvt = bmpConverter("./pic/Fig0201.bmp");
	int epoch_num = 1000;
	time_t start = clock();
	while (epoch_num--)
	{
		bmpCvt.RGB2Gry(true, false);
	}
	cout << "时间: " << clock() - start << endl;
	start = clock();
	epoch_num = 1000;
	while (epoch_num--)
	{
		bmpCvt.RGB2Gry(false, false);
	}
	cout << "时间: " << clock() - start << endl;
	bmpCvt.Img2Bmp("./pic/Fig0201Grey.bmp", 8);
}

void hw2_avg_std_conv()
{
	bmpConverter bmpCvt("./pic/Fig0201Grey.bmp");
	bmpCvt.P2avgstd8Bit(128, 64);
	//bmpCvt.PAffine(1.5, 0);
	bmpCvt.Img2Bmp("./pic/output.bmp");
}

void hw2_HistogramEqualize()
{
	bmpConverter bmpCvt("./pic/Fig0212Plane.bmp");
	bmpCvt.PHistogramEqualize8bit();
	bmpCvt.Img2Bmp("./pic/Fig0212Plane_balanced.bmp");
	bmpCvt.BmpFile2Img("./pic/Fig0216Girl.bmp");
	//bmpCvt.PAffine(5, -50);
	bmpCvt.PHistogramEqualize8bit();
	bmpCvt.Img2Bmp("./pic/Fig0216Girl_balanced.bmp");
}


void hw2_24bitHist()
{
	bmpConverter bmpCvt("./pic/H0201Rgb.bmp");
	bmpCvt.PHistogramEqualize24bit2();
	bmpCvt.Img2Bmp("./pic/H0201Rgb_balanced.bmp");
}


void hw2_14bit_convert()
{
	bmpConverter bmpCvt;
	bmpCvt.read14bitRaw("./pic/H0204IR14bit.raw");
	puts("转化完成");
	bmpCvt.Img2Bmp("./pic/H0204IR14bit.bmp", 8);
}
int main()
{
	//hw2_time_cmp();
	hw2_14bit_convert();
	//bmpCvt.Img2Bmp("./pic/Fig0201Grey.bmp", 8);
	return 0;
}

