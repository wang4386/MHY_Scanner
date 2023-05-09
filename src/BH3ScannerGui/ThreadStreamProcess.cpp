﻿#include "ThreadStreamProcess.h"
#include <QImage>

ThreadStreamProcess::ThreadStreamProcess(QObject* parent)
	: QThread(parent)
	,ts1(this)
{

}

ThreadStreamProcess::~ThreadStreamProcess()
{
	cv::destroyAllWindows();
}

void ThreadStreamProcess::biliInitStream(int uid, std::string access_key, std::string uName)
{
	LoginData = m1.verify(uid, access_key);
	m1.setUserName(uName);
}

void ThreadStreamProcess::stop()
{
	stopStream = true;
}

static int a = 0;

void ThreadStreamProcess::run()
{
	QThread::msleep(3000);
	stopStream = false;
	VideoProcessor vp;
	vp.OpenVideo("./cache/output.flv");//错误判断
	int64_t latestTimestamp = av_gettime_relative();
	if (vp.avformatContext->streams[vp.index]->start_time != AV_NOPTS_VALUE)
	{
		int64_t streamTimestamp = av_rescale_q(vp.avformatContext->streams[vp.index]->start_time,
			vp.avformatContext->streams[vp.index]->time_base, { 1, AV_TIME_BASE });
		if (streamTimestamp > latestTimestamp)
		{
			latestTimestamp = streamTimestamp;
		}
	}
	av_seek_frame(vp.avformatContext, -1, latestTimestamp, AVSEEK_FLAG_BACKWARD);
	int f = 0;
	cv::namedWindow("Video");
	while (true)
	{
		
		//int64_t video_duration_sec = vp.avformatContext->duration / AV_TIME_BASE;
		//size_t timeInSeconds = av_q2d(vp.avstream->time_base);
		f++;
		if (stopStream)
		{
			cv::destroyWindow("Video");
			break;
		}
		int op1 = vp.read(vp.avPacket);
		if (vp.avPacket->stream_index != vp.index)
		{
			continue;
		}
		int op2 = vp.SendPacket(vp.avPacket);
		if(op2!=0)
		{
			av_seek_frame(vp.avformatContext, -1, latestTimestamp, AVSEEK_FLAG_BACKWARD);
			QThread::msleep(300);
		}
		while (true)
		{
			int op3 = vp.ReceiveFrame(vp.avframe);
			if (op3 != 0)
			{
				cv::waitKey(1000);
				break;
			}
			// 转换像素格式
			sws_scale(vp.swsCtx, vp.avframe->data, vp.avframe->linesize, 0,
				vp.avCodecContext->height, vp.pFrameBGR->data, vp.pFrameBGR->linesize);
			cv::Mat img(vp.avCodecContext->height, vp.avCodecContext->width, CV_8UC3, vp.pFrameBGR->data[0]);
			//if (f > vp.fps)//0.8
			{
				if (!ts1.isRunning())
				{
					cv::Rect roi(0, 0, 1280, 720);
					cv::Mat crop_img = img(roi);
					ts1.getImg(crop_img);
#ifdef _DEBUG
					imshow("Video", crop_img);
					std::cout << "命中次数" << a++ << std::endl;
#endif // _DEBUG
					ts1.start();
				}
				cv::waitKey(1);
				f = 0;
			}
			break;
		}
		if (ts1.uqrcode.find("biz_key=bh3_cn") != std::string::npos)//ts1.uqrcode is private
		{
			int retcode = m1.scanCheck(ts1.uqrcode,LoginData);
			emit loginSResults(retcode == 0);
			ts1.uqrcode.clear();
			break;
		}
		av_packet_unref(vp.avPacket);
	}
}