#pragma once
#include <stdio.h>
#include<iostream>
#include<string>
#include"FFMPEG_Decode.h"

#include"include/jrtplib/rtpsession.h"
#include"include/jrtplib/rtperrors.h"
#include"include/jrtplib/rtpudpv4transmitter.h"
#include"include/jrtplib/rtpsessionparams.h"
#include"include/jrtplib/rtpsourcedata.h"
#include"include/jrtplib/rtppacket.h"

using namespace std;
using std::string;


void checkerr(int err);

typedef struct
{
	uint8_t *m_buffer;
	long m_length;
	int m_current_size;
}CVideoData;

class RTPReciever :public RTPSession
{
public:
	FILE*fp;
	RTPReciever(string file_name);
	~RTPReciever();
	CVideoData* m_pVideoData;

	RTPSessionParams							sessparams;
	RTPUDPv4TransmissionParams		transparams;
	uint16_t												portbase ;
	int														status;
	uint64_t												flag;
	int														f_flag;

public:
	void StartGetData( FFMPEG_Decode &decode);
	void ProcessPacket(const RTPPacket *packet,  FFMPEG_Decode &decode);
	void SessInit();
};

