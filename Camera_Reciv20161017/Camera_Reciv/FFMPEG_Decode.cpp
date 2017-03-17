#include "stdafx.h"
#include "FFMPEG_Decode.h"
#include<iostream>
#include"include/opencv/highgui.h"
using namespace std;

FFMPEG_Decode::FFMPEG_Decode()
{
	pCodec = NULL;			//解码器指针
	pCodecCtx = NULL;		//用于编码器的设置
	pFrame = NULL;			 //解码后的数据帧
	//packet = (AVPacket*)malloc(sizeof(AVPacket));
	packet = NULL;
	codec_id = AV_CODEC_ID_H264;
	pCVFrame =NULL;
	out_buffer = NULL;

	screen = NULL;
	sdlRender = NULL;
	sdlTexture = NULL;
	frame_count = 0;
}


FFMPEG_Decode::~FFMPEG_Decode()
{
	free(packet);
	free(pCVFrame);
	free(out_buffer);
	av_frame_free(&pFrame);
	av_frame_free(&pFrameRGB);
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	sws_freeContext(img_convert_ctx);
}

void FFMPEG_Decode::DecodeInit()
{
	avcodec_register_all();  //注册所有的编码器

	pCodec = avcodec_find_decoder(codec_id);  //查找对应的解码器
	if (!pCodec)
	{
		cout << " Codec not found!" << endl;
		exit(-1);
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);  //为AVCodecContext分配内存
	if (!pCodecCtx)
	{
		cout << "Could not allocate video codec context!" << endl;
		exit(-1);
	}


	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)    //打开解码器
	{
		cout << "Could not open codec" << endl;
		exit(-1);
	}

	pFrame = av_frame_alloc();  //为解码后的数据分配内存
	//pFrameRGB = avcodec_alloc_frame();
	pFrameRGB = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	packet = (AVPacket*)malloc(sizeof(AVPacket));
	av_init_packet(packet);
	//memset(in_buffer + IN_BUFSIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);

}

void FFMPEG_Decode::DecodeFrame(uint8_t *frame, uint32_t frame_len)
{
	int len=0;
	packet->data = frame;
	packet->size = frame_len;
	while (packet->size>0)
	{
		len = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (len < 0)
		{
			fprintf(stderr, "Eerror while decodeing frame %d\n", frame_count);
			cout << "Decode err" << endl;
			exit(-1);
		}
		if (pCVFrame == NULL)
		{
			pCVFrame = cvCreateImage(cvSize(pCodecCtx->width, pCodecCtx->height), 8, 3);
			avpicture_alloc((AVPicture*)pFrameRGB, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
			cvNamedWindow("camera");
			//SDL_INIT();
		}

		if (got_picture)
		{
			//fflush(stdout);

			/*RGB图片显示*/
			img_convert_ctx = sws_getContext(pFrame->width, pFrame->height, PIX_FMT_YUV420P, \
				pFrame->width, pFrame->height, PIX_FMT_RGB24, \
				2, NULL, NULL, NULL);

			/*YUV图片显示*/
			//img_convert_ctx = sws_getContext(pFrame->width, pFrame->height, PIX_FMT_YUV420P, \
			//	pFrame->width, pFrame->height, PIX_FMT_YUV420P, \
			//	2, NULL, NULL, NULL);

			if (img_convert_ctx != NULL)
			{
				/*Opencv显示*/
				GetOpencvFrame();
				if (pCVFrame)
				{
					cvShowImage("camera", pCVFrame);
					cvWaitKey(1);
				}
				/*SDL显示*/
				//GetSDLFrame();
			}
			//count++;
		}
		packet->size -= len;
		//packet->data += len;
	}
	//if (pFrame)
	//	av_free(pFrame);
	//if (packet)
	//	av_free_packet(packet);
}

void FFMPEG_Decode::GetOpencvFrame()
{
	pFrameRGB = av_frame_alloc();
	out_buffer = new uint8_t[avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height)];
	avpicture_fill((AVPicture*)pFrameRGB, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	uchar *p = NULL;
	p = pFrame->data[1];
	pFrame->data[1] = pFrame->data[2];
	pFrame->data[2] = p;
	sws_scale(img_convert_ctx,
		pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
		pFrameRGB->data, pFrameRGB->linesize);//格式转换，YUV420转换成RGB
	memcpy(pCVFrame->imageData, out_buffer, pCodecCtx->width*pCodecCtx->height * 3);
	pCVFrame->widthStep = pCodecCtx->width * 3;// (3通道的)
	pCVFrame->origin = 0;
	delete[] out_buffer;
	av_frame_free(&pFrameRGB);
}

void FFMPEG_Decode::GetSDLFrame()
{
	sws_scale(img_convert_ctx,
		pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
		pFrameYUV->data, pFrameYUV->linesize);

	SDL_UpdateTexture(sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0]);
	SDL_RenderClear(sdlRender);
	SDL_RenderCopy(sdlRender, sdlTexture, &sdlRect, &sdlRect);
	SDL_RenderPresent(sdlRender);
	SDL_Delay(100);

}

void FFMPEG_Decode::SDL_INIT()
{
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture*)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		cout << "Couldn't initialize SDL - " << SDL_GetError() << endl;
		exit(-1);
	}
	screen = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, \
		pCodecCtx->width, pCodecCtx->height, SDL_WINDOW_OPENGL);
	if (!screen)
	{
		cout << "Couldn't create SDL window - " << SDL_GetError() << endl;
		exit(-1);
	}

	sdlRender = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = pCodecCtx->width;
	sdlRect.h = pCodecCtx->height;
}