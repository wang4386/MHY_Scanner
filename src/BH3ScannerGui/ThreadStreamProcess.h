﻿#pragma once
#include <QThread>
#include "Mihoyosdk.h"
#include "ScreenScan.h"
#include "QRScanner.h"
#include "VideoProcessor.h"
#include "ThreadSacn.h"
class ThreadStreamProcess  : public QThread
{
	Q_OBJECT
public:
	ThreadStreamProcess(QObject *parent = nullptr);
	~ThreadStreamProcess();
	void biliInitStream(int uid, std::string access_key, std::string uName);
	void stop();
	void run();
	cv::Mat sendImg;
signals:
	void loginSResults(const bool& b);
private:
	std::string LoginData;
	bool stopStream =false;
	Mihoyosdk m1;
	ThreadSacn ts1;
};