// affine transformation.cpp : 定义控制台应用程序的入口点。  
//  
/**
 * Automatic perspective correction for quadrilateral objects. See the tutorial at
 * http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 */
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2\imgproc\types_c.h>
#include <iostream>  
#include <stdio.h>
#include <string>
#include <cstring>
//#pragma comment(lib,"opencv_core2410d.lib")            
//#pragma comment(lib,"opencv_highgui2410d.lib")            
//#pragma comment(lib,"opencv_imgproc2410d.lib")      

using namespace cv;
using namespace std;

Point2f center(0, 0);
float eps = 1e-6;

int tranS();
// 求两条直线的交点
Point2f computeIntersect(Vec4i a, Vec4i b)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
	float denom, d;

	if (abs(d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) > eps)
	{
		Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	}
	else
		return Point2f(-1, -1);
}

void sortCorners(vector<Point2f>& corners,
	Point2f center)
{
	vector<Point2f> top, bot;

	for (int i = 0; i < corners.size(); i++)
	{
		
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}
	corners.clear();

	if (top.size() == 2 && bot.size() == 2) {
		Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
		Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
		Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
		Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];


		corners.push_back(tl);
		corners.push_back(tr);
		corners.push_back(br);
		corners.push_back(bl);
		for (auto & p : corners)
		{
			printf("%f  %f\n", p.x, p.y);
		}
	}
}

int clickTimes = 0;
Mat tmpsw, src;
vector<Vec4i> lines;
vector<Point2f> corners;
string  file_name;

void get4Point(int event, int x, int y, int flags, void *utsc)
{

	if (event == EVENT_LBUTTONUP)   //响应鼠标左键抬起事件
	{
		circle(tmpsw, Point(x, y), 2.5, Scalar(0, 0, 255), 2.5);  //标记选中点
		imshow("Source Image", tmpsw);
		corners.push_back(Point2f(x, y));
		clickTimes++;
		printf("size %d\n", corners.size());
	}
	if (event == EVENT_LBUTTONUP && clickTimes == 4)
	{
		destroyWindow("Source Image");
		tranS();
	}
}

int tranS()
{
	// Get mass center  
	for (int i = 0; i < corners.size(); i++)
		center += corners[i];
	center *= (1. / corners.size());

	sortCorners(corners, center);
	if (corners.size() != 4) {
		cout << "The corners were not sorted correctly! corner size:"<<corners.size() << endl;
		return -1;
	}
	Mat dst = src.clone();

	// Draw lines  
	//for (int i = 0; i < lines.size(); i++)
	//{
	//	Vec4i v = lines[i];
	//	line(dst, Point(v[0], v[1]), Point(v[2], v[3]), CV_RGB(0, 255, 0));
	//}

	// Draw corner points  
	circle(dst, corners[0], 3, CV_RGB(255, 0, 0), 2);
	circle(dst, corners[1], 3, CV_RGB(0, 255, 0), 2);
	circle(dst, corners[2], 3, CV_RGB(0, 0, 255), 2);
	circle(dst, corners[3], 3, CV_RGB(255, 255, 255), 2);

	// Draw mass center  
	circle(dst, center, 3, CV_RGB(255, 255, 0), 2);

	Mat quad = Mat::zeros(300, 220, CV_8UC3);

	vector<Point2f> quad_pts;
	quad_pts.push_back(Point2f(0, 0));
	quad_pts.push_back(Point2f(quad.cols, 0));
	quad_pts.push_back(Point2f(quad.cols, quad.rows));
	quad_pts.push_back(Point2f(0, quad.rows));

	Mat transmtx = getPerspectiveTransform(corners, quad_pts);
	warpPerspective(src, quad, transmtx, quad.size());
	namedWindow(file_name, WINDOW_NORMAL);
	resizeWindow(file_name, 800, 600);
	imshow(file_name, dst);
	imshow("quadrilateral", quad);
	cout << "in func" << endl;
	waitKey();
	return 0;
}

int main()
{
	file_name = "org.jpg";
	src = imread(file_name);
	if (src.empty())
		return -1;
	
	Mat bw;
	tmpsw = src.clone();
	// 载入图像→灰度化→边缘处理得到边缘图像
	cvtColor(src, bw, CV_BGR2GRAY);
	blur(bw, bw, Size(3, 3));
	Canny(bw, bw, 100, 100, 3);
	//Canny(bw, bw, 100, 150, 3);

	HoughLinesP(bw, lines, 1, CV_PI / 180, 70, 30, 10);

	// Expand the lines  
	for (int i = 0; i < lines.size(); i++)
	{
		Vec4i v = lines[i];
		lines[i][0] = 0;
		lines[i][1] = ((float)v[1] - v[3]) / (v[0] - v[2]) * -v[0] + v[1];
		lines[i][2] = src.cols;
		lines[i][3] = ((float)v[1] - v[3]) / (v[0] - v[2]) * (src.cols - v[2]) + v[3];
	}

	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = i + 1; j < lines.size(); j++)
		{
			Point2f pt = computeIntersect(lines[i], lines[j]);
			if (pt.x >= 0 && pt.y >= 0)
				corners.push_back(pt);
		}
	}

	vector<Point2f> approx;
	approxPolyDP(Mat(corners), approx, arcLength(Mat(corners), true) * 0.02, true);

	if (approx.size() != 4)
	{
		cout << "The object is not quadrilateral!" << endl;
		//imshow(file_name, tmpsw);
		corners.clear();
		namedWindow("Source Image", WINDOW_NORMAL);
		resizeWindow("Source Image", 800, 600);
		imshow("Source Image", tmpsw);
		setMouseCallback("Source Image", get4Point);
	}
	else
	{
		tranS();
	}
	waitKey();
	return 0;
}