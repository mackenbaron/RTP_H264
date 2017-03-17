#include "stdafx.h"
#include "RTPReciever.h"
#include "FFMPEG_Decode.h"
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<string>

#include "JY901.h"

using namespace std;
using std::string;
#define MAX_SIZE 500000

void checkerr(int err)
{
	if (err < 0)
	{
		cerr << "ERROR：" << RTPGetErrorString(err) << endl;
		exit(-1);
	}
}

RTPReciever::RTPReciever(string file_name)
{
	fopen_s(&fp,file_name.c_str(),"wb");
	m_pVideoData = (CVideoData*)malloc(sizeof(CVideoData));
	m_pVideoData->m_buffer = (uint8_t*)malloc(sizeof(uint8_t)*MAX_SIZE);
	memset(m_pVideoData->m_buffer, 0, MAX_SIZE);
	m_pVideoData->m_current_size = 0;
	m_pVideoData->m_length = 0;

	portbase = 9992;
	flag = 1;
	f_flag = 0;
}

RTPReciever::~RTPReciever()
{
	if (m_pVideoData->m_buffer != NULL)
	{
		free(m_pVideoData->m_buffer);
		m_pVideoData->m_buffer = NULL;
	}
	if (m_pVideoData != NULL)
	{
		free(m_pVideoData);
		m_pVideoData = NULL;
	}
}


void RTPReciever::SessInit()
{
	sessparams.SetOwnTimestampUnit(1.0 / 9000.0);
	sessparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(portbase);
	status = Create(sessparams, &transparams);
	checkerr(status);
}

void RTPReciever::StartGetData( FFMPEG_Decode &decode)
{

	BeginDataAccess();
		if (GotoFirstSourceWithData())
		{
			do {
				RTPPacket *packet;
				packet = GetNextPacket();
		
				while (packet != NULL)
				{
					ProcessPacket(packet, decode);
					//cout << "Get a Packet" << endl;
					DeletePacket(packet);
					packet = GetNextPacket();
				}
			} while (GotoNextSourceWithData());
		}
#ifndef RTP_SUPPORT_THREAD
		int status = Poll();
		checkerr(status);
#endif // RTP_SUPPORT_THREAD
		EndDataAccess();
}

void RTPReciever::ProcessPacket(const RTPPacket *rtppack,  FFMPEG_Decode &decode)
{
	if (rtppack->GetPayloadType() == 96)
	{
			if (rtppack->HasMarker())  //if it is the last fragment  ，最后一个分片的内容为传感器数据
			{
				//解码H264
					m_pVideoData->m_length = m_pVideoData->m_current_size;
					//m_pVideoData->m_length = m_pVideoData->m_current_size + rtppack->GetPacketLength();
					//memcpy(m_pVideoData->m_buffer + m_pVideoData->m_current_size, rtppack->GetPayloadData(), rtppack->GetPayloadLength());
					//fwrite(m_pVideoData->m_buffer, m_pVideoData->m_length, 1, fp);
					decode.DecodeFrame(m_pVideoData->m_buffer, m_pVideoData->m_length);
					m_pVideoData->m_current_size = 0;

				//解码传感器数据包
				//JY901.CopeSerialData(rtppack->GetPayloadData(), rtppack->GetPayloadLength());
				/*printf("Time:20%.2d年%.2d月%.2d日-%.2d：%.2d：%.3f\r\n", \
					(short)JY901.stcTime.ucYear, \
					(short)JY901.stcTime.ucMonth, \
					(short)JY901.stcTime.ucDay, \
					(8 + (short)JY901.stcTime.ucHour) % 24, \
					(short)(JY901.stcTime.ucMinute), \
					fmod((float)JY901.stcTime.ucSecond + (float)JY901.stcTime.usMiliSecond / 1000, float(60.0)));

				printf("Angle:%.3f %.3f %.3f\r\n", \
					(float)JY901.stcAngle.Angle[0] / 32768 * 180, \
					(float)JY901.stcAngle.Angle[1] / 32768 * 180, \
					(float)JY901.stcAngle.Angle[2] / 32768 * 180);

				printf("Longitude:%.6f,Lattitude:%.5f\r\n", \
					(float)(JY901.stcLonLat.lLon / 10000000) + (float)(JY901.stcLonLat.lLon % 10000000) / 1e5 / 60.0, \
					(float)(JY901.stcLonLat.lLat / 10000000) + (float)(JY901.stcLonLat.lLat % 10000000) / 1e5 / 60.0);*/
			}
			else
			{
				memcpy(m_pVideoData->m_buffer + m_pVideoData->m_current_size, rtppack->GetPayloadData(), rtppack->GetPayloadLength());
				m_pVideoData->m_current_size += rtppack->GetPayloadLength();
			}
	}
}