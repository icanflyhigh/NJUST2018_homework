#include <opencv2\opencv.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <string.h>

using namespace cv;
int main()
{
	String file_path = "D:/À„∑®øŒ…Ë/pic/";
	Mat src = imread(file_path+"1.png");
	if (src.empty())
	{
		printf("Cannot Find image...\n");
		return 0;
	}
	char win_name[] = "openCV Image Demo";
	namedWindow(win_name, WINDOW_AUTOSIZE);
	imshow(win_name, src);
	namedWindow("output window", WINDOW_AUTOSIZE);
	Mat output_image;
	cvtColor(src, output_image, COLOR_BGR2HLS);
	imshow("output window", output_image);
	imwrite(file_path, output_image);
	
	return 0;
}