#include <opencv2\opencv.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <string.h>
#include <queue>
#include <math.h>

using namespace cv;
using namespace std;

String file_path = "D:/pic/";
String file_name = "6.jpg";


const float EPS = 1e-6;
Mat load_pic(String & input_path)
{
	Mat src = imread(input_path);
	if (src.empty())
	{
		printf("Cannot Find image...\n");
		return src;
	}
	return src;
}




String out_img_name = file_name + "_processed";
Mat src = load_pic(file_path += file_name), dst;
Mat  Lz, GM, GD;
const int col = src.cols, row = src.rows , point_num = col*row;
const float pi_1 = 1.0/3.1415926535;
int mouse_time = 0;
int mousex = -1, mousey = -1;
float *** costmap;
float weight1 = 0.53, weight2 = 0.23, weight3 = 0.24;
float sqrt2 = 1.414;
float len_val[8] = { sqrt2, 1, sqrt2, 1, 1, sqrt2, 1, sqrt2 };
int direct[8][2] = { {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1} };
float straight_fact = 0.0;
/*
0   1    2
3   口   4
5   6    7

*/
struct vertex
{
	int x, y,fx,fy;
	float cost;
	bool operator<(const vertex & a)const
	{
		return this->cost > a.cost;
	}
	vertex()
	{
		x = 0, y = 0, fx=-1, fy=-1;
		cost = 99999.0;
	}
	vertex(int i, int j, int f1,int f2, float cost1)
	{
		x = i, y = j, cost = cost1, fx = f1, fy = f2;
	}
};


float*** get_mat() 
{
	float *** ret = new float ** [row];
	for (int i = 0; i < row; i++)
	{
		ret[i] = new float * [col];
		for (int j = 0; j < col; j++)
		{
			ret[i][j] = new float[8];
		}

	}
	return ret;
}

/*获得laplacian Zero-Crossing*/
void getLz() 
{
	Mat kernel_laplacian = (Mat_<char>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
	Mat mean, std;
	//Mat t1 = Mat(src);
	Mat t1;
	Rect rect;
	src.convertTo(t1, CV_32FC1, 1 / 255.0);
	float fact_std = 1;
	int height = 2, width = 2;
	Mat t2;
	for (int i = 0; i < row ; i += 3)
	{
		width = 2;
		if (i + 1 >= row)
		{
			height = row - i ;
		}
		for (int j = 0; j < col ; j += 3)
		{
			
			if (j + 1 >= col)
			{
				width = col - j ;
			}
			t2 = t1(Rect(j, i, width, height));
			meanStdDev(t2, mean, std);
			for (int u = 0; u < height; u++)
			{
				for (int v = 0; v < width; v++)
				{
					for (int k = 0; k < 3; k++)
					{
						t1.at<Vec3f>(i + u, j + v)[k] -= mean.at<double>(0, 0);
						t1.at<Vec3f>(i + u, j + v)[k] /= fact_std * sqrt(std.at<double>(0, 0) + EPS);
						//t1.at<Vec3f>(i + u, j + v)[k] /= fact_std * (std.at<double>(0, 0) + EPS);
					}
				}
			}


		}
	}
	Mat temp;
	filter2D(t1, Lz, t1.depth(), kernel_laplacian);
	// temp.convertTo(Lz, CV_32FC1, 1 / 255.0);
	
	

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				if (Lz.at<Vec3f>(i, j)[k] > EPS)
				{
					Lz.at<Vec3f>(i, j)[k] = 1.0;
				}
				else
				{
					Lz.at<Vec3f>(i, j)[k] = 0.0;
				}
			}
			// 通过0号通道纪律均值
			// Lz.at<Vec3f>(i, j)[0] = (Lz.at<Vec3f>(i, j)[0]+Lz.at<Vec3f>(i, j)[1]+ Lz.at<Vec3f>(i, j)[2])/3;
			Lz.at<Vec3f>(i, j)[0] = max(Lz.at<Vec3f>(i, j)[0],max(Lz.at<Vec3f>(i, j)[1], Lz.at<Vec3f>(i, j)[2]));
		}
	}


}

/*获得Gradient Magnitude 和 Gradient Direction*/
void getGM_GD(Mat & GM, Mat & GD)
{
	Mat kernel_Ix = (Mat_<char>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
	Mat kernel_Iy = (Mat_<char>(3, 3) << 1, 2, 1, 0, 0, 0, -1, -2, -1);
	Mat Ix, Iy;
	filter2D(src, Ix, src.depth(), kernel_Ix);
	filter2D(src, Iy, src.depth(), kernel_Iy);
	Ix.convertTo(Ix, CV_32FC1, 1 / 255.0);
	Ix.convertTo(Iy, CV_32FC1, 1 / 255.0);
	GM = Ix.clone();
	float num_max[3] = { EPS, EPS, EPS };
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				float a1 = Ix.at<Vec3f>(i, j)[k] * Ix.at<Vec3f>(i, j)[k];
				float a2 = Iy.at<Vec3f>(i, j)[k] * Iy.at<Vec3f>(i, j)[k];
				GM.at<Vec3f>(i, j)[k] = sqrt(a1+ a2);
				if (GM.at<Vec3f>(i, j)[k] > num_max[k])
				{
					num_max[k] = GM.at<Vec3f>(i, j)[k];
				}
			}
		}
	}
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			int maxx = EPS, maxy = EPS;

			for (int k = 0; k < 3; k++)
			{
				GM.at<Vec3f>(i, j)[k] = 1.0 - GM.at<Vec3f>(i, j)[k] / num_max[k];
				if (maxx < Ix.at<Vec3f>(i, j)[k]) maxx = Ix.at<Vec3f>(i, j)[k];
				if (maxy < Iy.at<Vec3f>(i, j)[k]) maxy = Iy.at<Vec3f>(i, j)[k];
			}
			// 通过0号通道记录
			//GM.at<Vec3f>(i, j)[0] = (GM.at<Vec3f>(i, j)[0]+GM.at<Vec3f>(i, j)[1]+ GM.at<Vec3f>(i, j)[2])/3;
			GM.at<Vec3f>(i, j)[0] = max(GM.at<Vec3f>(i, j)[0], max(GM.at<Vec3f>(i, j)[1], GM.at<Vec3f>(i, j)[2]));
			// 取最大的梯度通道
			Ix.at<Vec3f>(i, j)[1] = sqrt(maxx*maxx + maxy * maxy);
			if (Ix.at<Vec3f>(i, j)[1] < 0.0001)
			{
				Ix.at<Vec3f>(i, j)[1] = 0.0001;
			}
			else
			{
				Ix.at<Vec3f>(i, j)[0] = maxx / Ix.at<Vec3f>(i, j)[1] / sqrt2;
				Iy.at<Vec3f>(i, j)[0] = maxy / Ix.at<Vec3f>(i, j)[1] / sqrt2;
			}

		}
	}

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				int tx = i + direct[k][0];
				int ty = j + direct[k][1];
				if (tx < 0 || tx >= row || ty < 0 || ty >= col)
				{
					costmap[i][j][k] = 99999.0;
					continue;
				}
				if (Ix.at<Vec3f>(i, j)[1] > 0.0001)
				{
					float temp = (weight3 * (pi_1 *(acos(abs(Iy.at<Vec3f>(i, j)[0] * direct[k][0] - Ix.at<Vec3f>(i, j)[0] * direct[k][1]) / len_val[k])
						+ acos(abs(Iy.at<Vec3f>(tx, ty)[0] * direct[k][0] - Ix.at<Vec3f>(tx, ty)[0] * direct[k][1]) / len_val[k])))) ;
					costmap[i][j][k] = weight1 * Lz.at<Vec3f>(tx, ty)[0]
						+ weight2 / len_val[k] * GM.at<Vec3f>(tx, ty)[0]
						+ temp;
				}
				else
				{
					costmap[i][j][k] = weight1 * Lz.at<Vec3f>(tx, ty)[0]
						+ weight2 / len_val[k] * GM.at<Vec3f>(tx, ty)[0] +weight3;
				}
			}
		}
	}
	
	
}




priority_queue<vertex> que;
int sx, sy;
bool ** vis = new bool *[row];
int ** fax = new int *[row], **fay = new int *[row];
void ini()
{
	mousex = -1, mousey = -1;
	for (int i = 0; i < row; i++)
	{
		vis[i] = new bool[col];
		fax[i] = new int[col];
		fay[i] = new int[col];
		for (int j = 0; j < col; j++)
		{
			vis[i][j] = false;
		}
	}
}
bool compute_flag = false;
void find_path(int dx, int dy)
{
	
	if (vis[dx][dy]) return;
	compute_flag = true;
	while (!que.empty())
	{
		vertex tv = que.top();
		que.pop();
		if (vis[tv.x][tv.y])continue;
		else vis[tv.x][tv.y] = true;
		fax[tv.x][tv.y] = tv.fx, fay[tv.x][tv.y] = tv.fy;
		if (tv.x == dx && tv.y == dy)
		{
			break;
		}
		for (int i = 0; i < 8; i++)
		{
			int tx = tv.x + direct[i][0];
			int ty = tv.y + direct[i][1];
			if (tx < 0 || tx >= row || ty < 0 || ty >= col||vis[tx][ty])
				continue;
			//que.push(vertex(tx, ty, tv.x, tv.y, tv.cost+costmap[tv.x][tv.y][i]+ straight_fact * (abs(tx-dx)+abs(tx-dy))));
			que.push(vertex(tx, ty, tv.x, tv.y, tv.cost+costmap[tv.x][tv.y][i]));
		}
	}
	compute_flag = false;
}

void draw_path(int x, int y)
{
	compute_flag = true;
	while (fax[x][y] != sx || fay[x][y] != sy)
	{
		int tx = fax[x][y];
		int ty = fay[x][y];
		dst.at<Vec3b>(x, y)[0] = 0;
		dst.at<Vec3b>(x, y)[1] = 255;
		dst.at<Vec3b>(x, y)[2] = 0;
		x = tx, y = ty;
	}
	//cout << sx << "  " << sy << endl;
	//cout << x << "  " << y << endl;
	imshow(out_img_name, dst);
	compute_flag = false;
}
void onMouse(int event, int x, int y, int flags, void* param)
{
	if (event == 1 && !compute_flag)
	{
		cout << "event1" << x << "  " << y << endl;
		if (mouse_time == 1)
		{
			draw_path(y, x);
			for (int i = 0; i < row; i++)
			{
				for (int j = 0; j < col; j++)
				{
					vis[i][j] = false;
					fax[i][j] = -1;
					fay[i][j] = -1;
				}
			}
			while (!que.empty())
			{
				que.pop();
			}
		}
		sx = y, sy = x;
		mousex = x, mousey = y;
		mouse_time = 1;
		que.push(vertex(y, x, y, x, 0.0));
	}
	if (event == 0 && mousex != -1&& !( y == sx && x == sy ) &&!compute_flag)
	{
		//cout << "event0" <<y<<"  "<<x<< endl;
		find_path(y, x);
		
	}
}


int main()
{

	//std::cout << ("Please Input the file Name:");
	//std::cin >> file_name;
	printf("请在源文件中修改读取图片路径\n");

	if (src.empty())
	{
		return 0;
	}
	ini();
	costmap = get_mat();
	getLz();
	getGM_GD(GM, GD);
	dst = src;
	namedWindow(out_img_name);
	imshow(out_img_name, src);
	setMouseCallback(out_img_name, onMouse);
	waitKey();
	imwrite(file_path + out_img_name, dst);
	return 0;
}
