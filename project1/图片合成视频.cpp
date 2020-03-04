#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2\imgproc\types_c.h>
#include<opencv2/imgcodecs/legacy/constants_c.h>
#include<string>
#include<Windows.h>
#include<direct.h>
#include<fstream>
#include<sstream>


using namespace std;
using namespace cv;

string GetInstruct()
{
	string finalInstrcut;
	cout << "请输入图像路径：";
	string FilePath;
	cin >> FilePath;
	cout << "请输入编码后的视频名称：";
	string Name;
	cin >> Name;
	cout << "请输入视频帧率：";
	string zhenlv;
	cin >> zhenlv;

	string str1 = "ffmpeg -f image2 -r ";
	string str2 = " -i ";
	string str3 = " -vcodec mpeg4 ";
	finalInstrcut = str1 + zhenlv + str2 + FilePath + "%d.jpg " + str3 + Name;
	return finalInstrcut;
}

int main()
{
	string decodeInstruct;
	decodeInstruct = GetInstruct();
	WinExec("cmd", SW_HIDE);
	LPCSTR instruct = decodeInstruct.c_str();
	system(instruct);
	return 0;
}