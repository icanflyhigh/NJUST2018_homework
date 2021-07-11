#define _CRT_SECURE_NO_WARNINGS
//#include <opencv2/opencv.hpp>
#include "bmpConverter.h"
#include "homework.h"
#include <iostream>
#include <ctime>
using namespace std;
//using namespace cv;
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
	bmpConverter bmpCvt("./pic/H0302Gry.bmp");  // H0302Gry.bmp
	bmpCvt.AMedianFilter2d1c(21, 21, true);
	bmpCvt.Img2Bmp("./pic/H0302Gry_MedF_col_cal.bmp");
}

void hw3_1dGuass()
{
	bmpConverter bmpCvt("./pic/H0303Gry.bmp");  // H0303Gry.bmp
	bmpCvt.AGuassFilter2d1c(3, 19);
	bmpCvt.Img2Bmp("./pic/H0303Gry_Guass2.bmp");
}

//void opencv_guass_blur()
//{
//	cv::Mat src, dst;
//	const char* filename = "./pic/H0303Gry.bmp";
//
//	cv::imread(filename).copyTo(src);
//	if (src.empty()) {
//		throw("Faild open file.");
//	}
//
//	int ksize1 = 19;
//	int ksize2 = 19;
//	double sigma1 = 3;
//	double sigma2 = 0;
//	cv::GaussianBlur(src, dst, cv::Size(ksize1, ksize2), sigma1, sigma2);
//	//高斯模糊的函数
//	//第三，第四，第五参数为高斯模糊的度数
//
//	cv::imshow("src", src);
//	cv::imshow("dst", dst);
//	cv::imwrite("./pic/H0303Gry_cvblur.bmp", dst);
//
//	cv::waitKey();
//	cout << getGaussianKernel(ksize1, sigma1, CV_64F);
//	return ;
//}


void sqrt_cmp()
{
	float a;
	time_t start;
	int cishu = 1000000;

	cin >> a;
	start = clock();
	for (int i = 0; i < cishu; i++)
	{
		fSqrtByCarmack(a);
	}
	cout << "fSqrtByCarmack: " << clock() - start << endl;
	start = clock();
	for (int i = 0; i < cishu; i++)
	{
		sqrtf(a);
	}
	cout << "sqrtf: " << clock() - start << endl;	
	
/*
平台配置x86 debug
结果：
42
fSqrtByCarmack: 33
sqrtf: 27
结论：
自带的更快，原因可能跟硬件有关
*/

}



void hw4_grad1_grad2_shenjun()
{
	bmpConverter bmpCvt("./pic/H0401Gry.bmp");  // H0303Gry.bmp
	bmpCvt.AEadgeDectGrad_SJ2d1c(0.5);
	bmpCvt.Img2Bmp("./pic/H0401js_gd.bmp");
	bmpConverter bmpCvt2("./pic/H0401Gry.bmp");  // H0303Gry.bmp
	bmpCvt2.AEadgeDectLaplacain_Gd2d1c(0.5);
	bmpCvt2.Img2Bmp("./pic/H0401la_gd.bmp");
}


void hw4_num_dect()
{
	// 缩小
	// 得到边缘强度
	// 积分图
	// 得到最大强度区域
	// 放大
	bmpConverter bmpCvt("./pic/H0402Gry.bmp");  // H0303Gry.bmp
	bmpCvt.num_dect();
	bmpCvt.Img2Bmp("./pic/H0402_det_sb.bmp");
	bmpConverter bmpCvt1("./pic/H0403Gry.bmp");  // H0303Gry.bmp
	bmpCvt1.num_dect();
	bmpCvt1.Img2Bmp("./pic/H0403_det_sb.bmp");
	bmpConverter bmpCvt2("./pic/H0404Gry.bmp");  // H0303Gry.bmp
	bmpCvt2.num_dect();
	bmpCvt2.Img2Bmp("./pic/H0404_det_sb.bmp");

}

void hw4_canny()
{
	bmpConverter bmpCvt("./pic/H0401Gry.bmp");  // H0303Gry.bmp
	bmpCvt.AEadgeCanny2d1c(0.4, 6, 125, 75);
	bmpCvt.Img2Bmp("./pic/H0401_canny.bmp");
}

void hw5_otsu()
{
	bmpConverter bmpCvt("./pic/H0501Gry.bmp");
	bmpCvt.PBinOtsu2d1c();
	bmpCvt.Img2Bmp("./pic/H0501Gry_Otsu.bmp");
}


void hw5_ImgSegmentation1()
{
	bmpConverter bmpCvt("./pic/H0502Gry.bmp");
	bmpCvt.task_ImgSegmentation1();
	bmpCvt.Img2Bmp("./pic/test.bmp");
	//bmpCvt.Img2Bmp("./pic/H0502Gry_ImgSegmentation1.bmp");
	//bmpCvt.Img2Bmp("./pic/H0501Gry_guassian.bmp");
}


void hw5_ImgSegmentation2()
{
	bmpConverter bmpCvt("./pic/H0502Gry.bmp");
	bmpCvt.task_ImgSegmentation2();
	bmpCvt.Img2Bmp("./pic/H0502Gry_ImgSegmentation2_1thre.bmp");
}


void hw5_ImgSegmentation3()
{
	bmpConverter bmpCvt("./pic/H0502Gry.bmp");
	bmpCvt.task_ImgSegmentation3();
	//bmpCvt.Img2Bmp("./pic/test.bmp");
	bmpCvt.Img2Bmp("./pic/H0502Gry_ImgSegmentation3_pro2.bmp");
	//bmpCvt.Img2Bmp("./pic/H0501Gry_guassian.bmp");
}

void hw5_kmeans_1()
{
	bmpConverter bmpCvt("./pic/H0501Gry.bmp");
	bmpCvt.k_meansThreshold(2);
	//bmpCvt.Img2Bmp("./pic/test.bmp");
	bmpCvt.Img2Bmp("./pic/H0501Gry_2-means.bmp");

}

void hw5_kmeans_2()
{
	bmpConverter bmpCvt("./pic/H0502Gry.bmp");
	bmpCvt.k_meansThreshold(3);
	//bmpCvt.Img2Bmp("./pic/test.bmp");
	bmpCvt.Img2Bmp("./pic/H0502Gry_3-means.bmp");
}

void hw5_kmeans_3()
{
	bmpConverter bmpCvt("./pic/H0502Gry.bmp");
	bmpCvt.k_meansThreshold(8);
	//bmpCvt.Img2Bmp("./pic/test.bmp");
	bmpCvt.Img2Bmp("./pic/H0502Gry_8-means.bmp");
}

void hw6_hough_1()
{
	bmpConverter bmpCvt1("./pic/H0601Bin.bmp");
	bmpCvt1.HoughLine1();
	bmpCvt1.Img2Bmp("./pic/H0601Bin_1.bmp");
	
	bmpConverter bmpCvt("./pic/H0602Bin.bmp");
	bmpCvt.HoughLine1();
	bmpCvt.Img2Bmp("./pic/H0602Bin_1.bmp");

}

void hw6_hough_2()
{
	bmpConverter bmpCvt("./pic/H0602Bin.bmp");
	bmpCvt.HoughLine2();
	bmpCvt.Img2Bmp("./pic/H0602Bin_2.bmp");
}

void hw6_circle_1()
{
	int x, y, r;
	bmpConverter bmpCvt("./pic/H0603Bin.bmp");

	bmpCvt.DCCircle(x, y, r);
	printf("x: %d\ty: %d\tr: %d\n", x, y, r);

	bmpCvt.Img2Bmp("./pic/H0603Bin_circle_1.bmp", 0, 'k');

}

void hw6_ChainCode_fill()
{
	bmpConverter bmpCvt("./pic/H0605Bin.bmp");
	bmpCvt.chainCode_fill();
	bmpCvt.Img2Bmp("./pic/H0605fill.bmp");
}

void hw6_expand_fcorrosion33()
{
	bmpConverter bmpCvt("./pic/square.bmp");
	bmpCvt.fExpand33();
	bmpConverter bmpCvt1("./pic/square.bmp");
	bmpCvt1.fcorrosion33();
	//bmpCvt.Img2Bmp("./pic/H0605fill.bmp");
}

void hw6_airport()
{
	bmpConverter bmpCvt("./pic/H0606Bin.bmp");
	bmpCvt.air_port_track_dect();
	bmpCvt.Img2Bmp("./pic/H0606Bin_hough.bmp");
}
void hw6_t1_draw()
{
	bmpConverter bmpCvt("./pic/t1.bmp");
	bmpCvt.hw6_draw(128, 128);
	bmpCvt.Img2Bmp("./pic/t1_1.bmp");
	bmpCvt.hw6_draw(0, 255);
	bmpCvt.Img2Bmp("./pic/t1_2.bmp");
	bmpCvt.hw6_draw(255, 0);
	bmpCvt.Img2Bmp("./pic/t1_3.bmp");
	bmpCvt.hw6_draw(255, 255);
	bmpCvt.Img2Bmp("./pic/t1_4.bmp");
}
void test()
{
	//hw5_otsu();
	bmpConverter bmpCvt("./pic/t1.bmp");
	bmpCvt.test();
	bmpCvt.Img2Bmp("./pic/t1_1.bmp");
}