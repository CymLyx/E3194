#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2\imgproc\types_c.h>
#include<fstream>

using namespace cv;

using namespace std;

//找到所提取轮廓的中心点

//在提取的中心小正方形的边界上每隔周长个像素提取一个点的坐标，求所提取四个点的平均坐标（即为小正方形的大致中心）
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}

//对图像进行透视变换
void LocateAndWarpQRcode(Mat InImage, Mat &OutImage)
{
	Point QRPoints[4];
	Mat src = InImage;
	Mat src_gray;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //模糊，去除毛刺
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//寻找轮廓 
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//轮廓筛选
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		//hierarchy[i][2] != -1 表示不是最外面的轮廓
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//最外面的清0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//找到定位点信息
		if (ic >= 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	if (contours2.size()<4)
	{
		for (int i = 0;i < 4;i++)
		{
			QRPoints[i].x = 0;
			QRPoints[i].y = 0;
		}
	}
	else
	{
		Point point[4];
		for (int i = 0; i < contours2.size(); i++)
		{
			point[i] = Center_cal(contours2, i);
		}

		

		//按左下角为0，左上角为1，右上角为2，右下角为3对point排序,再存到QRPoints里
		//左上角的横纵坐标和最小，右下角横纵坐标和最大，因此先将这两个点确定
		int sum[4] = { 0 };
		for (int i = 0;i < 4;i++)sum[i] = point[i].x + point[i].y;
		int min = 999999, max = 0;
		for (int i = 0;i < 4;i++)
		{
			if (min > sum[i])min = sum[i];
			if (max < sum[i])max = sum[i];
		}
		for (int i = 0;i < 4;i++)
		{
			if (sum[i] == min)QRPoints[1] = point[i];
			if (sum[i] == max)QRPoints[3] = point[i];
		}
		//接下来确定左下角和右上角
		for (int i = 0;i < 4;i++)
		{
			if (QRPoints[3].x - point[i].x > 200 && point[i].y - QRPoints[1].y > 200)QRPoints[0] = point[i];
			if (point[i].x - QRPoints[1].x > 200 && QRPoints[3].y - point[i].y > 200)QRPoints[2] = point[i];
		}
		for (int i = 0;i < 4;i++)cout << "(" << QRPoints[i].x << "," << QRPoints[i].y << ")" << endl;

		vector<Point2f>src_corners(4);
		for (int i = 0;i < 4;i++)src_corners[i] = QRPoints[i];


		//为方便解码，若遇到不是矩形的情况，将其进行放大为矩形
		//对左边进行调整
		if (QRPoints[0].x < QRPoints[1].x)QRPoints[1].x = QRPoints[0].x;
		else QRPoints[0].x = QRPoints[1].x;

		//对右边进行调整
		if (QRPoints[2].x > QRPoints[3].x)QRPoints[3].x = QRPoints[2].x;
		else QRPoints[2].x = QRPoints[3].x;

		//对上边进行调整
		if (QRPoints[1].y < QRPoints[2].y)QRPoints[2].y = QRPoints[1].y;
		else QRPoints[1].y = QRPoints[2].y;

		//对底边进行调整
		if (QRPoints[0].y > QRPoints[3].y)QRPoints[3].y = QRPoints[0].y;
		else QRPoints[0].y = QRPoints[3].y;

		for (int i = 0;i < 4;i++)cout << "(" << QRPoints[i].x << "," << QRPoints[i].y << ")" << endl;
        //对图片进行透视变换
		vector<Point2f>dst_corners(4);
		for (int i = 0;i < 4;i++)dst_corners[i] = QRPoints[i];
		
		

		
		Mat resultImg;
		Mat warpmatrix = getPerspectiveTransform(src_corners, dst_corners);
		warpPerspective(InImage, resultImg, warpmatrix, InImage.size(), CV_INTER_LINEAR);

		OutImage = resultImg;
		imwrite("变换.jpg", resultImg);
	}
}

//对透视变换后的图片进行定位
void OnlyLocateQRcode(Mat image, Point QRPoints[4])
{
	Mat src = image;
	Mat src_gray;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //模糊，去除毛刺
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//寻找轮廓 
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//轮廓筛选
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		//hierarchy[i][2] != -1 表示不是最外面的轮廓
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//最外面的清0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//找到定位点信息
		if (ic >= 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	if (contours2.size()<3)
	{
		for (int i = 0;i < 4;i++)
		{
			QRPoints[i].x = 0;
			QRPoints[i].y = 0;
		}
	}
	else
	{
		Point point[4];
		for (int i = 0; i < contours2.size(); i++)
		{
			point[i] = Center_cal(contours2, i);
		}



		//左下角为0，左上角为1，右上角为2，右下角为3
		QRPoints[0] = point[0];
		QRPoints[1] = point[3];
		QRPoints[2] = point[2];
		QRPoints[3] = point[1];


		//为方便解码，若遇到不是矩形的情况，将其进行放大为矩形
		//对左边进行调整
		if (QRPoints[0].x < QRPoints[1].x)QRPoints[1].x = QRPoints[0].x;
		else QRPoints[0].x = QRPoints[1].x;

		//对右边进行调整
		if (QRPoints[2].x > QRPoints[3].x)QRPoints[3].x = QRPoints[2].x;
		else QRPoints[2].x = QRPoints[3].x;

		//对上边进行调整
		if (QRPoints[1].y < QRPoints[2].y)QRPoints[2].y = QRPoints[1].y;
		else QRPoints[1].y = QRPoints[2].y;

		//对底边进行调整
		if (QRPoints[0].y > QRPoints[3].y)QRPoints[3].y = QRPoints[0].y;
		else QRPoints[0].y = QRPoints[3].y;



		//根据四个定位点的中心坐标求出二维码的四个顶点坐标
		double addlen = double(QRPoints[2].x - QRPoints[1].x) / 47 * 3.6;
		QRPoints[0].x -= addlen;
		QRPoints[0].y += addlen;
		QRPoints[1].x -= addlen;
		QRPoints[1].y -= addlen;
		QRPoints[2].x += addlen;
		QRPoints[2].y -= addlen;
		QRPoints[3].x += addlen;
		QRPoints[3].y += addlen;

		line(src_all, QRPoints[0], QRPoints[1], Scalar(0, 0, 255), 2);
		line(src_all, QRPoints[1], QRPoints[2], Scalar(0, 0, 255), 2);
		line(src_all, QRPoints[2], QRPoints[3], Scalar(0, 0, 255), 2);
		line(src_all, QRPoints[3], QRPoints[0], Scalar(0, 0, 255), 2);

		imwrite("定位.jpg", src_all);
	}
}


//解码时，检验每个色块代表黑色还是白色或是代表停止的灰色，若为黑色返回0，若为白色返回1，否则为-1；
int Color_Check(Mat image, int x, int y, double rows, double cols)
{
	
	int grayValue[100000];
	int count = 0;
	for (int i = x;i<x + rows;i++)
		for (int j = y;j < y + cols;j++)
		{
			grayValue[count] = image.at<uchar>(i, j);
			count++;
		}
	double blackNum = 0;
	double whiteNum = 0;
	double elseColor = 0;
	for (int i = 0;i < count;i++)
	{
		if (grayValue[i] <= 212)blackNum++;
		else whiteNum++;
	}
	//返回颜色占比大于50%的颜色对应的值
	double checkValue = 0;
	if (blackNum / count >= 0.5)return 0;
	else return 1;
}

//二维码解码
void QRcode_Decode(Mat image, Point QRPoints[4], vector<int>&decodeByte)
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	imwrite("灰度.jpg", grey);
	//获得要解码的二维码每个色块的高和长
	double rows = double(QRPoints[0].y - QRPoints[1].y) / 53;
	double cols = double(QRPoints[2].x - QRPoints[1].x) / 53;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;

	//先判断该图片为奇数还是偶数张，flag为0则为奇数，为1则为偶数
	int flag = 0;
	double whiteNum = 0;
	double blackNum = 0;
	for (double j = Y + 8 * cols;j < Y + 15 * cols;j = j + cols)
	{
		if (Color_Check(grey, X, j, rows, cols) == 1)whiteNum++;
		else blackNum++;
	}
	if (whiteNum / 7 > 0.5)flag = 0;
	else flag = 1;

	cout << whiteNum << "," << blackNum << endl;
	if (flag == 0)cout << "奇数张" << endl;
	else cout << "偶数张" << endl;



	//开始解码二维码，并输出到txt里
	//先将所有的二进制数据存入一个int类容器里
	for (double j = Y + 15 * cols;j<Y + 45 * cols;j = j + cols)
		decodeByte.push_back(Color_Check(grey, X, j, rows, cols));
	for (double i = X + rows;i < X + 8 * rows;i = i + rows)
		for (double j = Y + 8 * cols;j < Y + 45 * cols;j = j + cols)
			decodeByte.push_back(Color_Check(grey, i, j, rows, cols));
	for (double i = X + 8 * rows;i < X + 45 * rows;i = i + rows)
		for (double j = Y;j < Y + 53 * cols;j = j + cols)
			decodeByte.push_back(Color_Check(grey, i, j, rows, cols));
	for (double i = X + 45 * rows;i < X + 53 * rows;i = i + rows)
		for (double j = Y + 8 * cols;j < Y + 45 * cols;j = j + cols)
			decodeByte.push_back(Color_Check(grey, i, j, rows, cols));
}

int main()
{
	vector<int> decodeByte;
	Mat image = imread("245.jpg");
	Point QRPoints[4];
	Mat finalImage;
	LocateAndWarpQRcode(image, finalImage);
	OnlyLocateQRcode(finalImage, QRPoints);

	QRcode_Decode(finalImage, QRPoints, decodeByte);
	for (int i = 0;i < decodeByte.size();i++)cout << decodeByte[i];
	system("pause");
}