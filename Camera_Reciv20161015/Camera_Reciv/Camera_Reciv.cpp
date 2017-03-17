// Camera_Reciv.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<iostream>
#include"FFMPEG_Decode.h"
#include"RTPReciever.h"
#include"include/opencv/highgui.h"
//#include"vld.h"
#define _CRT_SECURE_NO_WARNINGS
using namespace std;
using std::string;
static string file_name = "I:/视频项目程序/Camera_Reciv20161015/1.h264";    //存储视频文件

#ifndef WIN32  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#else  
#include <winsock2.h>  
#endif



int main(int argc,char *argv[])
{
#ifdef WIN32  
	WSADATA dat;
	int err = WSAStartup(MAKEWORD(2, 2), &dat);
	if (err != 0)
	{
		cout << "socket err" << endl;
		exit(-1);
	}
#endif // WIN32  
	FFMPEG_Decode decode;
	RTPReciever sess(file_name);

	sess.SessInit();
	decode.DecodeInit();
	cout << "waiting for data....." << endl;
	cout << "CVideoData size = " << sizeof(CVideoData) << endl;
	cout << "AVpacket size=" << sizeof(AVPacket) << endl;
	cout << "string size = " << sizeof(string) << endl;
	while (1)
	{
		sess.StartGetData(decode);
	}
#ifdef WIN32  
	WSACleanup();
#endif // WIN32
	return 0;
}
