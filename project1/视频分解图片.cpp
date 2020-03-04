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

//获取将视频解码为图片的ffmpeg命令行指令
string GetInstruct()
{
	string finalInstrcut;
	cout << "请输入视频路径：";
	string FilePath;
	cin >> FilePath;
	cout << "请输入解码后图片名称：";
	string imageName;
	cin >> imageName;

	string str1 = "ffmpeg -i ";
	string str2 = "%d.jpg";
	finalInstrcut = str1 + FilePath + " " + imageName + str2;
	return finalInstrcut;
}



int main()
{
	string decodeInstruct;
	decodeInstruct = GetInstruct();

	//打开命令行
	WinExec("cmd", SW_HIDE);
	LPCSTR instruct = decodeInstruct.c_str();
	system(instruct);

	return 0;
}