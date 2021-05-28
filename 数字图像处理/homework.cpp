#define _CRT_SECURE_NO_WARNINGS
#include "bmpConverter.h"
#include "homework.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>
using namespace std;
using namespace cv;
void hw2_time_cmp()
{
	bmpConverter bmpCvt("./pic/Fig0201.bmp");
	int epoch_num = 1000;
	time_t start = clock();
	while (epoch_num--)
	{
		bmpCvt.RGB2Gry(true, false);
	}
	time_t end = clock() - start;
	cout << "时间: " << end << endl;
	epoch_num = 1000;
	start = clock();
	while (epoch_num--)
	{
		bmpCvt.RGB2Gry(false, false);
	}
	end = clock() - start;
	cout << "时间: " << end << endl;
	bmpCvt.RGB2Gry(true, true);
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

void hw3_time_cmp()
{
	bmpConverter bmpCvt("./pic/Fig0201.bmp");
	int epoch_num = 1000;
	time_t start = clock();
	while (epoch_num--)
	{
		bmpCvt.RGB2Gry2(true, false);
	}
	time_t end = clock() - start;
	cout << "时间: " << end << endl;
	epoch_num = 1000;
	start = clock();
	while (epoch_num--)
	{
		bmpCvt.RGB2Gry2(false, false);
	}
	end = clock() - start;
	cout << "时间: " << end << endl;
	bmpCvt.RGB2Gry(true, true);
	bmpCvt.Img2Bmp("./pic/Fig0201Grey.bmp", 8);
}

void hw3_1dAvgF_col_cal()
{
	bmpConverter bmpCvt("./pic/H0302Gry.bmp");// H0302Gry.bmp
	bmpCvt.AavgFilter2d1c(1, 21, true);
	bmpCvt.Img2Bmp("./pic/H0302Gry_F_col_cal.bmp");
}
void hw3_1dAvgF_calGraph()
{
	bmpConverter bmpCvt("./pic/H0302Gry.bmp");// H0302Gry.bmp
	bmpCvt.AavgFilter2d1c(1, 21, true, 'g');
	bmpCvt.Img2Bmp("./pic/H0302Gry_F_calGraph.bmp");
}

void hw3_inv_SSE()
{
	bmpConverter bmpCvt("./pic/H0302Gry.bmp");
	bmpCvt.PInvert1c('s');
	bmpCvt.Img2Bmp("./pic/H0302Gry_inv_SSE.bmp");
}

void hw3_1dMed_col_cal()
{
	bmpConverter bmpCvt("./pic/H0302Gry.bmp");// H0302Gry.bmp
	bmpCvt.AMedianFilter2d1c(21, 21, true);
	bmpCvt.Img2Bmp("./pic/H0302Gry_MedF_col_cal.bmp");
}

void hw3_1dGuass()
{
	bmpConverter bmpCvt("./pic/H0303Gry.bmp");// H0303Gry.bmp
	bmpCvt.AGuassFilter2d1c(3, 19);
	bmpCvt.Img2Bmp("./pic/H0303Gry_Guass2.bmp");
}

void opencv_guass_blur()
{
	cv::Mat src, dst;
	const char* filename = "./pic/H0303Gry.bmp";

	cv::imread(filename).copyTo(src);
	if (src.empty()) {
		throw("Faild open file.");
	}

	int ksize1 = 19;
	int ksize2 = 19;
	double sigma1 = 3;
	double sigma2 = 0;
	cv::GaussianBlur(src, dst, cv::Size(ksize1, ksize2), sigma1, sigma2);
	//高斯模糊的函数
	//第三，第四，第五参数为高斯模糊的度数

	cv::imshow("src", src);
	cv::imshow("dst", dst);
	cv::imwrite("./pic/H0303Gry_cvblur.bmp", dst);

	cv::waitKey();
	cout << getGaussianKernel(ksize1, sigma1, CV_64F);
	return ;
}


