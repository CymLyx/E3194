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

//��ȡ����Ƶ����ΪͼƬ��ffmpeg������ָ��
string GetInstruct()
{
	string finalInstrcut;
	cout << "��������Ƶ·����";
	string FilePath;
	cin >> FilePath;
	cout << "����������ͼƬ���ƣ�";
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

	//��������
	WinExec("cmd", SW_HIDE);
	LPCSTR instruct = decodeInstruct.c_str();
	system(instruct);

	return 0;
}