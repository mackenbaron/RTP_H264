#include "stdafx.h"
#include "RTPReciever.h"
#include "FFMPEG_Decode.h"
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<string>

using namespace std;
using std::string;
#define MAX_SIZE 5000000


void checkerr(int err)
{
	if (err < 0)
	{
		cerr << "ERROR£º" << RTPGetErrorString(err) << endl;
		exit(-1);
	}
}

RTPReciever::RTPReciever(string file_name)
{
	fopen_s(&fp,file_name.c_str(),"wb");
	m_pVideoData = (CVideoData*)malloc(sizeof(CVideoData));
	m_pVideoData->m_buffer = (uint8_t*)malloc(sizeof(uint8_t)*MAX_SIZE);
	m_pVideoData->m_current_size = 0;
	m_pVideoData->m_length = 0;

	portbase = 9992;
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
					cout << "Get a Packet" << endl;
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
		if (rtppack->HasMarker())  //if it is the last fragment
		{
			m_pVideoData->m_length = m_pVideoData->m_current_size + rtppack->GetPayloadLength();
			memcpy(m_pVideoData->m_buffer + m_pVideoData->m_current_size, rtppack->GetPayloadData(), rtppack->GetPayloadLength());
			fwrite(m_pVideoData->m_buffer, m_pVideoData->m_length, 1, fp);
			cout << "sucess store one frame" << endl;
			decode.DecodeFrame(m_pVideoData->m_buffer,m_pVideoData->m_length);
			//cout << "sucess decode one frame" << endl;
			m_pVideoData->m_current_size = 0;
		}
		else
		{
			size_t payloadlength = rtppack->GetPayloadLength();
			cout << "payloadlength:" << payloadlength << endl;
			memcpy(m_pVideoData->m_buffer + m_pVideoData->m_current_size, rtppack->GetPayloadData(), rtppack->GetPayloadLength());
			m_pVideoData->m_current_size += rtppack->GetPayloadLength();
		}
	}
}