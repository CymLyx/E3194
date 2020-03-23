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

//�ҵ�����ȡ���������ĵ�

//����ȡ������С�����εı߽���ÿ���ܳ���������ȡһ��������꣬������ȡ�ĸ����ƽ�����꣨��ΪС�����εĴ������ģ�
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}

//��ͼ�����͸�ӱ任
void LocateAndWarpQRcode(Mat InImage, Mat &OutImage)
{
	Point QRPoints[4];
	Mat src = InImage;
	Mat src_gray;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	//Ԥ����
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //ģ����ȥ��ë��
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//Ѱ������ 
	//��һ������������ͼ�� 2ֵ����
	//�ڶ����������ڴ�洢����FindContours�ҵ��������ŵ��ڴ����档
	//�����������ǲ㼶��**[Next, Previous, First_Child, Parent]** ��vector
	//���ĸ����������ͣ��������ṹ
	//����������ǽڵ����ģʽ��������ȫ��Ѱ��
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//����ɸѡ
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		//hierarchy[i][2] != -1 ��ʾ���������������
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//���������0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//�ҵ���λ����Ϣ
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

		

		//�����½�Ϊ0�����Ͻ�Ϊ1�����Ͻ�Ϊ2�����½�Ϊ3��point����,�ٴ浽QRPoints��
		//���Ͻǵĺ����������С�����½Ǻ���������������Ƚ���������ȷ��
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
		//������ȷ�����½Ǻ����Ͻ�
		for (int i = 0;i < 4;i++)
		{
			if (QRPoints[3].x - point[i].x > 200 && point[i].y - QRPoints[1].y > 200)QRPoints[0] = point[i];
			if (point[i].x - QRPoints[1].x > 200 && QRPoints[3].y - point[i].y > 200)QRPoints[2] = point[i];
		}
		for (int i = 0;i < 4;i++)cout << "(" << QRPoints[i].x << "," << QRPoints[i].y << ")" << endl;

		vector<Point2f>src_corners(4);
		for (int i = 0;i < 4;i++)src_corners[i] = QRPoints[i];


		//Ϊ������룬���������Ǿ��ε������������зŴ�Ϊ����
		//����߽��е���
		if (QRPoints[0].x < QRPoints[1].x)QRPoints[1].x = QRPoints[0].x;
		else QRPoints[0].x = QRPoints[1].x;

		//���ұ߽��е���
		if (QRPoints[2].x > QRPoints[3].x)QRPoints[3].x = QRPoints[2].x;
		else QRPoints[2].x = QRPoints[3].x;

		//���ϱ߽��е���
		if (QRPoints[1].y < QRPoints[2].y)QRPoints[2].y = QRPoints[1].y;
		else QRPoints[1].y = QRPoints[2].y;

		//�Եױ߽��е���
		if (QRPoints[0].y > QRPoints[3].y)QRPoints[3].y = QRPoints[0].y;
		else QRPoints[0].y = QRPoints[3].y;

		for (int i = 0;i < 4;i++)cout << "(" << QRPoints[i].x << "," << QRPoints[i].y << ")" << endl;
        //��ͼƬ����͸�ӱ任
		vector<Point2f>dst_corners(4);
		for (int i = 0;i < 4;i++)dst_corners[i] = QRPoints[i];
		
		

		
		Mat resultImg;
		Mat warpmatrix = getPerspectiveTransform(src_corners, dst_corners);
		warpPerspective(InImage, resultImg, warpmatrix, InImage.size(), CV_INTER_LINEAR);

		OutImage = resultImg;
		imwrite("�任.jpg", resultImg);
	}
}

//��͸�ӱ任���ͼƬ���ж�λ
void OnlyLocateQRcode(Mat image, Point QRPoints[4])
{
	Mat src = image;
	Mat src_gray;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	//Ԥ����
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //ģ����ȥ��ë��
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//Ѱ������ 
	//��һ������������ͼ�� 2ֵ����
	//�ڶ����������ڴ�洢����FindContours�ҵ��������ŵ��ڴ����档
	//�����������ǲ㼶��**[Next, Previous, First_Child, Parent]** ��vector
	//���ĸ����������ͣ��������ṹ
	//����������ǽڵ����ģʽ��������ȫ��Ѱ��
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//����ɸѡ
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		//hierarchy[i][2] != -1 ��ʾ���������������
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//���������0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//�ҵ���λ����Ϣ
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



		//���½�Ϊ0�����Ͻ�Ϊ1�����Ͻ�Ϊ2�����½�Ϊ3
		QRPoints[0] = point[0];
		QRPoints[1] = point[3];
		QRPoints[2] = point[2];
		QRPoints[3] = point[1];


		//Ϊ������룬���������Ǿ��ε������������зŴ�Ϊ����
		//����߽��е���
		if (QRPoints[0].x < QRPoints[1].x)QRPoints[1].x = QRPoints[0].x;
		else QRPoints[0].x = QRPoints[1].x;

		//���ұ߽��е���
		if (QRPoints[2].x > QRPoints[3].x)QRPoints[3].x = QRPoints[2].x;
		else QRPoints[2].x = QRPoints[3].x;

		//���ϱ߽��е���
		if (QRPoints[1].y < QRPoints[2].y)QRPoints[2].y = QRPoints[1].y;
		else QRPoints[1].y = QRPoints[2].y;

		//�Եױ߽��е���
		if (QRPoints[0].y > QRPoints[3].y)QRPoints[3].y = QRPoints[0].y;
		else QRPoints[0].y = QRPoints[3].y;



		//�����ĸ���λ����������������ά����ĸ���������
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

		imwrite("��λ.jpg", src_all);
	}
}


//����ʱ������ÿ��ɫ������ɫ���ǰ�ɫ���Ǵ���ֹͣ�Ļ�ɫ����Ϊ��ɫ����0����Ϊ��ɫ����1������Ϊ-1��
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
	//������ɫռ�ȴ���50%����ɫ��Ӧ��ֵ
	double checkValue = 0;
	if (blackNum / count >= 0.5)return 0;
	else return 1;
}

//��ά�����
void QRcode_Decode(Mat image, Point QRPoints[4], vector<int>&decodeByte)
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	imwrite("�Ҷ�.jpg", grey);
	//���Ҫ����Ķ�ά��ÿ��ɫ��ĸߺͳ�
	double rows = double(QRPoints[0].y - QRPoints[1].y) / 53;
	double cols = double(QRPoints[2].x - QRPoints[1].x) / 53;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;

	//���жϸ�ͼƬΪ��������ż���ţ�flagΪ0��Ϊ������Ϊ1��Ϊż��
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
	if (flag == 0)cout << "������" << endl;
	else cout << "ż����" << endl;



	//��ʼ�����ά�룬�������txt��
	//�Ƚ����еĶ��������ݴ���һ��int��������
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