#include "MultThread_Run.h"
#include "NewUI_Demo.h"
#include <QLabel>
#include "Everything.h"

extern HANDLE g_bShutDown;
extern QVector<CAMERASTRUCT*> g_vectorCamera;
extern QVector<CCycleBuffer*> g_ImgBuffer;
extern QVector<HANDLE> WriteSign;
extern int g_iCameraTotal;
//日志工具
extern std::shared_ptr<spd::logger> SYS_logger;//系统
extern std::shared_ptr<spd::logger> ALM_logger;//报警
extern std::shared_ptr<spd::logger> OPS_logger;//操作

extern QVector<CBaseCheckAlg*> g_CheckClass;
extern QVector<CONTROLPARAM> g_controlparam;
extern HANDLE g_hCom;
extern QString AppPath;
extern int g_PhotoTimes;
extern int MAX_CAPSULECOUNT;
extern Socket_CPP* g_SocketPLC;
extern QString saveImagePath;//图片存储路径

extern bool g_Deleting;
extern bool g_saveLoopFlag;//Everything delete flag 1删除 0不删
CBaseCheckAlg* EnsureAlg(QString str)
{
	for (QVector<CBaseCheckAlg*>::iterator it = g_CheckClass.begin(); it != g_CheckClass.end(); it++)
	{
		if ((*it)->GetCameraName() == str)
		{
			return *it;
		}
	}
	return NULL;
}
//设置外触发Line1
void SetTriggerLine(int i, int line)
{
#ifdef BASLER
	if (!g_vectorCamera[i]->cb_Camera.IsOpen())
	{
		g_vectorCamera[i]->cb_Camera.Open();
	}
	g_vectorCamera[i]->cb_Camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
	g_vectorCamera[i]->cb_Camera.TriggerMode.SetValue(TriggerMode_On);
	g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(false);
	TriggerModeEnums e = g_vectorCamera[i]->cb_Camera.TriggerMode.GetValue();
	if (line == 0)
	{
		g_vectorCamera[i]->cb_Camera.TriggerSource.SetValue(TriggerSource_Software);
	}
	if (line == 1)
	{
		g_vectorCamera[i]->cb_Camera.TriggerSource.SetValue(TriggerSource_Line1);
	}
	if (line == 2)
	{
		g_vectorCamera[i]->cb_Camera.TriggerSource.SetValue(TriggerSource_Line2);
	}
#endif
#ifdef DAHENG
	CGXFeatureControlPointer m_objFeatureControlPtr = g_vectorCamera[i]->m_objDevicePtr->GetRemoteFeatureControl();
	//将当前功能设置到设备中
	m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("On");
	//将当前功能设置到设备中
	m_objFeatureControlPtr->GetEnumFeature("TriggerSource")->SetValue("Line0");
#endif
}
//设置连续采集
void SetTriggerOff(int i)
{
#ifdef BASLER
	if (!g_vectorCamera[i]->cb_Camera.IsOpen())
	{
		g_vectorCamera[i]->cb_Camera.Open();
	}
	g_vectorCamera[i]->cb_Camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
	g_vectorCamera[i]->cb_Camera.TriggerMode.SetValue(TriggerMode_Off);
	g_vectorCamera[i]->cb_Camera.ExposureAuto.SetValue(ExposureAuto_Off);
	g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(true);
	g_vectorCamera[i]->cb_Camera.AcquisitionFrameRate.SetValue(g_vectorCamera[i]->i_LineRateHZ);
	g_vectorCamera[i]->cb_Camera.ExposureTime.SetValue(g_vectorCamera[i]->i_ExpouseTime);
	g_vectorCamera[i]->cb_Camera.BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
	//g_vectorCamera[i]->cb_Camera.BalanceRatio.SetValue(1.63);//适时更改,改为once白平衡

	if (g_vectorCamera[i]->b_InvertX)
	{
		g_vectorCamera[i]->cb_Camera.ReverseX.SetValue(true);
	}
	else
	{
		g_vectorCamera[i]->cb_Camera.ReverseX.SetValue(false);
	}
#endif
#ifdef DAHENG
	CGXFeatureControlPointer m_objFeatureControlPtr = g_vectorCamera[i]->m_objDevicePtr->GetRemoteFeatureControl();
	//将当前功能设置到设备中
	m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("Off");
	m_objFeatureControlPtr->GetEnumFeature("AcquisitionFrameRateMode")->SetValue("On"); \
		m_objFeatureControlPtr->GetFloatFeature("AcquisitionFrameRate")->SetValue(g_vectorCamera[i]->i_LineRateHZ);
#endif
}

//////////////////////////////////////////////////////////////////////////
//test time

#pragma region MultGetThread
#ifdef BASLER
MultGetThread_Run::MultGetThread_Run(QObject *parent)
	: QObject(parent)
{
	totalcount = 0;
	sizetotal = 0;
	m_LabelShow = nullptr;
	m_bAlRegister = false;
	m_bSave = false;
}

MultGetThread_Run::~MultGetThread_Run()
{

}


void MultGetThread_Run::OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
{

	if (ptrGrabResult->GrabSucceeded())
	{
		CPylonImage target;
		CImageFormatConverter converter;
		converter.OutputPixelFormat = PixelType_BGR8packed;
		converter.OutputBitAlignment = OutputBitAlignment_LsbAligned;
		converter.Convert(target, ptrGrabResult);
		if (totalcount == 0)
		{
			g_vectorCamera[m_iSelfIndex]->i_FrameWidth = target.GetWidth();
			g_vectorCamera[m_iSelfIndex]->i_FrameHeight = target.GetHeight();
			g_vectorCamera[m_iSelfIndex]->i_FrameChannel = 3;
			sizetotal = g_vectorCamera[m_iSelfIndex]->i_FrameChannel * target.GetWidth()*target.GetHeight();
			m_MatGetOnece = Mat(target.GetHeight(), target.GetWidth(), g_vectorCamera[m_iSelfIndex]->i_FrameChannel == 1 ? CV_8UC1 : CV_8UC3);
			//g_ImgBuffer[m_iSelfIndex] = new CCycleBuffer(sizetotal * 5);
		}
		if (m_LabelShow != nullptr && WAIT_OBJECT_0 == WaitForSingleObject(g_controlparam[m_iSelfIndex].hFinishGetImage, 1))
		{
			Mat img = Mat(target.GetHeight(), target.GetWidth(), 3 == 1 ? CV_8UC1 : CV_8UC3);
			memcpy(img.data, (uchar*)target.GetBuffer(), sizetotal);
			if (m_bSave)
			{
				QString strSave = m_strFile + QString::number(m_iSelfIndex) + "_" + QString::number(totalcount) + ".png";
				imwrite(std::string(strSave.toLocal8Bit()), img);
			}

			if (nullptr != m_LabelShow)
			{
				emit SHOWDIRECTIMG(img);
			}
		}
		else
		{
			SYS_logger->info("Thread No.{} Catch one Image No.{}", m_iSelfIndex, totalcount);

			QString strs = "Camera No. " + QString::number(m_iSelfIndex) + " Open";
			strs = "Camera No." + QString::number(m_iSelfIndex) + "WriteList";
			//daily_logger->info(strs.toStdString());
			//WaitForSingleObject(WriteSign[m_iSelfIndex], INFINITE);
			//ResetEvent(WriteSign[m_iSelfIndex]);
			//g_ImgBuffer[m_iSelfIndex]->write((uchar*)target.GetBuffer(), sizetotal);
			memcpy(m_MatGetOnece.data, target.GetBuffer(), sizetotal);
			emit GETONEIMAGEMAT(m_MatGetOnece);
			//SetEvent(WriteSign[m_iSelfIndex]);
			SetEvent(g_controlparam[m_iSelfIndex].h_ReadFirstImg);
		}
		totalcount++;
	}
}
int MultGetThread_Run::ThreadGetImage(int indexcam = 1, bool b_continue = false)//取图函数
{
	if (indexcam != -1)
	{
		if (m_iSelfIndex != indexcam)//可以只开几个，如果是-1就全开
			return -1;
	}
	totalcount = 0;
	int pi = m_iSelfIndex;//1个相机时应该是0
	SYS_logger->info("ThreadGetImage No.{} Started", pi);//以pi的值代替{}
	SetEvent(g_controlparam[m_iSelfIndex].hFinishGetImage);//第一个参数是每个线程控制开关。创建事件/图像获取队列安全退出标志
	CPylonImage target;
	if (MODEL_BASLER == g_vectorCamera[pi]->TypeOFCamera)
	{
		CGrabResultPtr ptrGrabResult;
		try
		{
			SYS_logger->info("ThreadGetImage No.{} Open", pi);
			if (!g_vectorCamera[pi]->cb_Camera.IsOpen())
			{
				g_vectorCamera[pi]->cb_Camera.Open();//打开相机
			}
			g_vectorCamera[pi]->cb_Camera.LineDebouncerTime.SetValue(100.0);	//设置debouncer time
			if (!b_continue)
			{
				int i = g_vectorCamera[pi]->TriggerBy;//触发方式
				switch (i)
				{
				case -1:
				{
					SYS_logger->info("ThreadGetImage No.{} SetTriggerOff", pi);
					SetTriggerOff(pi);//软件触发
					break;
				}
				case 0:
				{
					SYS_logger->info("ThreadGetImage No.{} SetTriggerOff", pi);
					SetTriggerLine(pi, 0);//0，1，2——line0，1，2触发
					break;
				}
				case 1:
				{
					SYS_logger->info("ThreadGetImage No.{} SetTriggerLine1", pi);
					SetTriggerLine(pi, 1);
					break;
				}
				case 2:
				{
					SYS_logger->info("ThreadGetImage No.{} SetTriggerLine1", pi);
					SetTriggerLine(pi, 2);
					break;
				}
				default:
					break;
				}
			}
			else
			{
				SetTriggerOff(pi);//软件触发
			}
			SYS_logger->info("ThreadGetImage No.{} PixelFormat", pi);
			PixelFormatEnums e = g_vectorCamera[pi]->cb_Camera.PixelFormat.GetValue();
			SYS_logger->info("ThreadGetImage No.{} PixelFormat OK", pi);
			g_vectorCamera[pi]->cb_Camera.PixelFormat.SetValue(PixelFormat_BGR8);
		}
		catch (const GenericException &e)//报错
		{
			g_vectorCamera[pi]->cb_Camera.StopGrabbing();//停止抓取
			g_vectorCamera[pi]->cb_Camera.Close();//关闭相机
			SYS_logger->info("ThreadGetImage No.{} GenericException Error", pi);
			ALM_logger->error(e.what());
		}
		int sizetotal;
		int totalcount = 0;
		SYS_logger->info("ThreadGetImage No.{} WAIT_OBJECT_0", pi);
		char*p_ucIMG = NULL;

		// For demonstration purposes only, register another image event handler.
		if (!m_bAlRegister)
		{
			g_vectorCamera[pi]->cb_Camera.RegisterImageEventHandler(this, RegistrationMode_Append, Cleanup_Delete);
			m_bAlRegister = true;
		}
		g_vectorCamera[pi]->cb_Camera.StartGrabbing();

		//image count for count each start 
		totalcount = 0;

		while (g_vectorCamera[pi]->cb_Camera.IsGrabbing())
		{
			// Retrieve grab results and notify the camera event and image event handlers.
			g_vectorCamera[pi]->cb_Camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_Return);
			// Nothing to do here with the grab result, the grab results are handled by the registered event handler.
		}
	}
	//set not signal in hFinishGetImage to close DecodeImage Thread
	ResetEvent(g_controlparam[pi].hFinishGetImage); // for restart again in case of change camera set ROI changed

	g_vectorCamera[pi]->cb_Camera.Close();

	return -1;
}


void MultGetThread_Run::SetMultIndex(int ind)
{
	m_iSelfIndex = ind;
}

void MultGetThread_Run::SetDirectShowDlg(void *dlg)
{
	m_LabelShow = dlg;
}
void MultGetThread_Run::SetSaveImage(bool save)
{
	m_bSave = save;
	if (m_bSave)
	{
		QDateTime current_time = QDateTime::currentDateTime();
		m_strFile = qApp->applicationDirPath() + "/SaveImage/TestSave/" + QString::number(current_time.date().year()) + "_" + QString::number(current_time.date().month()) + "_" + QString::number(current_time.date().day()) + "_" + QString::number(current_time.time().hour()) + "_" + QString::number(current_time.time().minute()) + "_" + QString::number(current_time.time().second()) + "/" + g_vectorCamera[m_iSelfIndex]->c_CameraName + "/";//exe所在目录
		QDir dir(m_strFile);
		//QMessageBox::warning(nullptr, "", m_strFile);
		if (!dir.exists())
		{
			bool res = dir.mkpath(m_strFile);
		}
	}
}
#endif
#ifdef DAHENG
MultGetThread_Run::MultGetThread_Run(QObject *parent)
	: QObject(parent)
{
	QueryPerformanceFrequency(&freq);
	totalcount = 0;
	sizetotal = 0;
	m_LabelShow = nullptr;
	m_bAlRegister = false;
}

MultGetThread_Run::~MultGetThread_Run()
{

}


void MultGetThread_Run::DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam)
{
	if (1)
	{
		GX_VALID_BIT_LIST emValidBits = GX_BIT_0_7;
		BYTE* pBuffer = NULL;
		pBuffer = (BYTE*)objImageDataPointer->ConvertToRGB24(emValidBits, GX_RAW2RGB_NEIGHBOUR, true);
		//if (totalcount == 0)
		{
			g_vectorCamera[m_iSelfIndex]->i_FrameWidth = objImageDataPointer->GetWidth();
			g_vectorCamera[m_iSelfIndex]->i_FrameHeight = objImageDataPointer->GetHeight();
			g_vectorCamera[m_iSelfIndex]->i_FrameChannel = 3;
			sizetotal = g_vectorCamera[m_iSelfIndex]->i_FrameChannel * objImageDataPointer->GetWidth()*objImageDataPointer->GetHeight();
			m_MatGetOnece = Mat(objImageDataPointer->GetHeight(), objImageDataPointer->GetWidth(), g_vectorCamera[m_iSelfIndex]->i_FrameChannel == 1 ? CV_8UC1 : CV_8UC3);
			//g_ImgBuffer[m_iSelfIndex] = new CCycleBuffer(sizetotal * 5);
		}
		if (m_LabelShow != nullptr && WAIT_OBJECT_0 == WaitForSingleObject(g_controlparam[m_iSelfIndex].hFinishGetImage, 1))
		{
			Mat img = Mat(objImageDataPointer->GetHeight(), objImageDataPointer->GetWidth(), 3 == 1 ? CV_8UC1 : CV_8UC3);
			memcpy(img.data, pBuffer, sizetotal);
			int zz = ((QLabel*)m_LabelShow)->frameWidth();
			QSize ss = ((QLabel*)m_LabelShow)->size();
			ss.setWidth(ss.width() - zz * 2);
			ss.setHeight(ss.height() - zz * 2);
			Mat imgsend;
			cv::resize(img, img, Size(ss.width(), ss.height()));
			cvtColor(img, img, COLOR_BGR2RGB);
			QImage disImage = QImage((const unsigned char*)(img.data), img.cols, img.rows, img.step, QImage::Format_RGB888);
			((QLabel*)m_LabelShow)->setPixmap(QPixmap::fromImage(disImage));
		}
		else
		{
			daily_logger->info("Thread No.{} Catch one Image No.{}", m_iSelfIndex, totalcount);
			QString strs = "Camera No. " + QString::number(m_iSelfIndex) + " Open";
			strs = "Camera No." + QString::number(m_iSelfIndex) + "WriteList";
			daily_logger->info(strs.toStdString());
			//WaitForSingleObject(WriteSign[m_iSelfIndex], INFINITE);
			//ResetEvent(WriteSign[m_iSelfIndex]);
			//g_ImgBuffer[m_iSelfIndex]->write((uchar*)target.GetBuffer(), sizetotal);
			memcpy(m_MatGetOnece.data, pBuffer, sizetotal);
			emit GETONEIMAGEMAT(m_MatGetOnece);
			//SetEvent(WriteSign[m_iSelfIndex]);
			SetEvent(g_controlparam[m_iSelfIndex].h_ReadFirstImg);
		}
		totalcount++;
	}
}
int MultGetThread_Run::ThreadGetImage(int indexcam = 1)//取图函数
{
	if (indexcam != -1)
	{
		if (m_iSelfIndex != indexcam)
			return -1;
	}
	int pi = m_iSelfIndex;//1个相机时应该是0

	int i = g_vectorCamera[pi]->TriggerBy;//触发方式
	switch (i)
	{
	case -1:
	{
		daily_logger->info("ThreadGetImage No.{} SetTriggerOff", pi);
		SetTriggerOff(pi);//软件触发
		break;
	}
	case 0:
	{
		daily_logger->info("ThreadGetImage No.{} SetTriggerOff", pi);
		SetTriggerLine(pi, 0);//0，1，2——line0，1，2触发
		break;
	}
	case 1:
	{
		daily_logger->info("ThreadGetImage No.{} SetTriggerLine1", pi);
		SetTriggerLine(pi, 1);
		break;
	}
	case 2:
	{
		daily_logger->info("ThreadGetImage No.{} SetTriggerLine1", pi);
		SetTriggerLine(pi, 2);
		break;
	}
	default:
		break;
	}

	//注册采集回调函数
	g_vectorCamera[pi]->m_objStreamPtr->RegisterCaptureCallback(this, this);

	//开启流层采集
	g_vectorCamera[pi]->m_objStreamPtr->StartGrab();

	//获取属性控制器对象 
	CGXFeatureControlPointer m_objFeatureControlPtr = g_vectorCamera[pi]->m_objDevicePtr->GetRemoteFeatureControl();
	//发送开始采集命令
	m_objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();

	return -1;
}


void MultGetThread_Run::SetMultIndex(int ind)
{
	m_iSelfIndex = ind;
}

void MultGetThread_Run::SetDirectShowDlg(void *dlg)
{
	m_LabelShow = dlg;
}
#endif
#pragma endregion

#pragma region MultDecodeThread

MultDecodeThread_Run::MultDecodeThread_Run(QObject *parent)
	: QObject(parent)
{
	QueryPerformanceFrequency(&freq);
	_CheckClass = nullptr;
}

MultDecodeThread_Run::~MultDecodeThread_Run()
{
}

int MultDecodeThread_Run::ThreadDecodeImage(int indexcam = -1, bool b = false)
{
	_CheckClass->StartCheck(g_vectorCamera[m_iSelfIndex]->c_CameraSign, SYS_logger);
	return -1;
}

int MultDecodeThread_Run::ThreadDecodeImageMat(Mat img)
{
	QString str;
	QueryPerformanceCounter(&litmpst);
	bool results = _CheckClass->Check(img, nullptr, str);
	QueryPerformanceCounter(&litmpend);
	dfTim += (double)(litmpend.QuadPart - litmpst.QuadPart) / freq.QuadPart * 1000;
	SYS_logger->info("MultDecode {},Alg Time:{}ms", m_iSelfIndex, dfTim / (timecheck + 1));
	if (0 != i_SaveLoop)
	{
		QString pathsave;
		if (results)
		{
			if (m_iSaveOKorNG == 0 || m_iSaveOKorNG == 1)
			{
				int isname = i_SaveLoop == -1 ? timecheck : timecheck % i_SaveLoop;
				pathsave = okdir_str + QString::number(isname) + ".tif";
			}
		}
		else
		{
			if (m_iSaveOKorNG == 0 || m_iSaveOKorNG == -1)
			{
				int isname = i_SaveLoop == -1 ? timecheck : timecheck % i_SaveLoop;
				pathsave = ngdir_str + QString::number(isname) + ".tif";

			}
		}
		emit SAVESIGNAL(img, pathsave);
	}
	emit OUTRESULTSUMMARY(str, index_pos, timecheck);
	timecheck++;
	return 0;
}

void MultDecodeThread_Run::SetMultIndex(int ind)
{
	m_iSelfIndex = ind;
	_CheckClass = EnsureAlg(g_vectorCamera[m_iSelfIndex]->c_CameraName);
	SYS_logger->info("Thread No.{} EnsureAlg Finish", m_iSelfIndex);

	int pi = m_iSelfIndex;
	if (nullptr == _CheckClass)
	{
		return;
	}
	m_iSaveOKorNG = g_vectorCamera[pi]->i_SaveOKorNG;
	i_SaveLoop = g_vectorCamera[pi]->i_SaveLoop;
	QString x = g_vectorCamera[pi]->c_CameraSign;
	index_pos = x.mid(3).toInt();
	bool b_doing = false;
	i_captotal = 0;
	timecheck = 0;
	QDateTime current_time = QDateTime::currentDateTime();

	QString saveImageYear = QString::number(current_time.date().year());
	saveImageYear = saveImageYear.length() < 4 ? ("0" + saveImageYear) : saveImageYear;
	QString saveImageMonth = QString::number(current_time.date().month());
	saveImageMonth = saveImageMonth.length() < 2 ? ("0" + saveImageMonth) : saveImageMonth;
	QString saveImageDay = QString::number(current_time.date().day());
	saveImageDay = saveImageDay.length() < 2 ? ("0" + saveImageDay) : saveImageDay;
	QString saveImageHour = QString::number(current_time.time().hour());
	saveImageHour = saveImageHour.length() < 2 ? ("0" + saveImageHour) : saveImageHour;
	QString saveImageMinute = QString::number(current_time.time().minute());
	saveImageMinute = saveImageMinute.length() < 2 ? ("0" + saveImageMinute) : saveImageMinute;
	QString saveImageSecond = QString::number(current_time.time().second());
	saveImageSecond = saveImageSecond.length() < 2 ? ("0" + saveImageSecond) : saveImageSecond;
	saveImagePath = AppPath + "/SaveImage/"
		+ saveImageYear + "_" //z=a>b?x:y
		+ saveImageMonth + "_"
		+ saveImageDay + "_"
		+ saveImageHour + "_"
		+ saveImageMinute + "_"
		+ saveImageSecond;
	ngdir_str = saveImagePath + "/" + g_vectorCamera[pi]->c_CameraName + "/NG/";
	okdir_str = saveImagePath + "/" + g_vectorCamera[pi]->c_CameraName + "/OK/";

	QDir dir(ngdir_str);
	if (!dir.exists())
	{
		bool res = dir.mkpath(ngdir_str);
	}
	QStringList filter;
	filter << "*.tif";
	dir.setNameFilters(filter);
	QDir dirs(okdir_str);
	if (!dirs.exists())
	{
		bool res = dirs.mkpath(okdir_str);
	}
	dirs.setNameFilters(filter);
	return;
}


#pragma endregion

#pragma region MultInit_Run
MultInit_Run::MultInit_Run(QWidget * parent)
	:QWidget(parent)
{
	_parent = parent;
}

MultInit_Run::~MultInit_Run()
{
}

int MultInit_Run::CloseTh()
{
	return 0;//关闭 0
}

bool sortcamera(CAMERASTRUCT *a, CAMERASTRUCT *b)//排序方法
{
	QString x = a->c_CameraSign;
	QString y = b->c_CameraSign;
	return x.mid(3).toInt() < y.mid(3).toInt();
}
int MultInit_Run::ThreadInit()
{
	QDateTime current_time = QDateTime::currentDateTime();
	QString StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//emit InitSingle(StrCurrentTime + QString::fromLocal8Bit("-开始初始化！"));//发射唯一信号，没用

	InitFunction Camera_Func;//初始化相机
	Camera_Func.StartModel = true;
	Camera_Func.GetAllCamera();//包含ReadConfig(); read333

	//daily_logger->info("GetAllCamera Finished");
	qSort(g_vectorCamera.begin(), g_vectorCamera.end(), sortcamera);//相机快速排序
	g_iCameraTotal = g_vectorCamera.size();
	if (g_iCameraTotal > 0)//此处加上，cameraset的部分就不用了
	{
		for (int i = 0; i < g_vectorCamera.size(); i++)
		{
			if (!g_vectorCamera[i]->cb_Camera.IsOpen())
			{
				g_vectorCamera[i]->cb_Camera.Open();
#ifdef BASLERUSB
				g_vectorCamera[i]->cb_Camera.UserSetDefault.SetValue(UserSetDefault_UserSet3);
				g_vectorCamera[i]->cb_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet3);
				g_vectorCamera[i]->cb_Camera.UserSetLoad.Execute();
				g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(false);
#endif
			}
		}
		for (int i = 0; i < g_vectorCamera.size(); i++)
		{
			if (g_vectorCamera[i]->cb_Camera.IsOpen())
			{
				g_vectorCamera[i]->cb_Camera.Close();
			}
		}
	}

	((NewUI_Demo*)_parent)->initPictureListWidget(); //图片列表
	//daily_logger->info("InitCamera Finished");
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化Camera类完成。"));
	QThread::msleep(200);
	((NewUI_Demo*)_parent)->LoadAlgorithmDLL();//加载算法
	SYS_logger->info("LoadAlgorithmDLL Finished");
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化加载算法完成。"));
	QThread::msleep(200);

	((NewUI_Demo*)_parent)->InitCheckClass();//每个相机检测初始化
	SYS_logger->info("InitCheckClass Finished");
	g_iCameraTotal = g_CheckClass.size();
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化算法类完成。"));
	//QThread::msleep(200);
	((NewUI_Demo*)_parent)->OpenConnect();
	SYS_logger->info("SetTrigger Finished");
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化Trigger完成。"));
	//QThread::msleep(200);
	// 	((NewUI_Demo*)_parent)->SetTrigger();
	// 	daily_logger->info("SetTrigger Finished");
	// 	current_time = QDateTime::currentDateTime();
	// 	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	// 	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化Trigger完成。"));
	// 	QThread::msleep(200);
	((NewUI_Demo*)_parent)->InitPicList();//初始化图像列表
	SYS_logger->info("InitPicList Finished");
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化队列完成。"));
	//QThread::msleep(200);
	/*
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化Socket。"));
	daily_logger->info("FinishInitOther Finished");
	*/
	((NewUI_Demo*)_parent)->FinishInitOther();//其他初始化
	return -1;//发射-1
}


#pragma endregion

MultSaveThread_Run::MultSaveThread_Run(QObject * parent, int _camcount)
{
	m_icamcount = _camcount;
}

MultSaveThread_Run::~MultSaveThread_Run()
{
}


void MultSaveThread_Run::ThreadSave(Mat img, QString str)
{
	imwrite(std::string(str.toLocal8Bit()), img);
	if (g_saveLoopFlag&&!g_Deleting)
	{
		emit DELETETIF("tif$");
	}
}


MultDeleteTifThread_Run::MultDeleteTifThread_Run(QObject * parent)
{
	//evethingSearchResults("tif$");//在机器上可以用 找到并删除1个
	
}

MultDeleteTifThread_Run::~MultDeleteTifThread_Run()
{
}

//everything
void MultDeleteTifThread_Run::evethingSearchResults(QString str)
{
	g_Deleting = 1;
	Everything_SetRegex(TRUE);//打开正则表达式，界面设置无效，需要在此设置。
	Everything_SetSearchW(str.toStdWString().c_str());
	Everything_SetSort(EVERYTHING_SORT_DATE_MODIFIED_ASCENDING);//按修改日期正向排序，放到搜索后边
	Everything_QueryW(TRUE);

	DWORD i;
	QString str1, str2, str1PlusStr2;
	//for (i = 0; i < Everything_GetNumResults(); i++)
	//{
	//	str1 = QString::fromStdWString(Everything_GetResultFileNameW(i));
	//	str2 = QString::fromStdWString(Everything_GetResultPath(i));
	//	str1PlusStr2 = str2 + "\\" + str1;
	//	QMessageBox::about(nullptr, str1, str1PlusStr2);
	//}
	int count;
	if (Everything_GetNumResults() <= 0)
	{
		return;
	}
	if (Everything_GetNumResults() <= 100)
	{
		count = Everything_GetNumResults();
	}
	else
	{
		count = 100;
	}
	for (i = 0; i < count; i++)
	{
		str1 = QString::fromStdWString(Everything_GetResultFileNameW(0));
		str2 = QString::fromStdWString(Everything_GetResultPath(0));
		str1PlusStr2 = str2 + "\\" + str1;
		QFile file(str1PlusStr2);
		file.remove();//删除该文件
	}
	g_Deleting = 0;
	return;
}

MultSummaryThread_Run::MultSummaryThread_Run(QObject * parent, int _camcount)
{
	m_icamcount = _camcount;
	m_iResultAllList = 0;
	for (int i = 0; i < g_vectorCamera.size(); i++)
	{
		m_iResultAllList = g_vectorCamera[i]->i_RealLocatPos > m_iResultAllList ? g_vectorCamera[i]->i_RealLocatPos : m_iResultAllList;
	}
	m_iResultAllList++;
	m_qslResultEachCamera = new QStringList[m_iResultAllList];
	b_eachalreadyfinish = new bool[m_iResultAllList];
	for (int i = 0; i < m_icamcount; i++)
	{
		b_eachalreadyfinish[i] = TRUE;
	}
	for (int i = 0; i < m_iResultAllList; i++)
	{
		for (int z = 0; z < g_vectorCamera.size(); z++)
		{
			if (i == g_vectorCamera[z]->i_RealLocatPos)
			{
				b_eachalreadyfinish[i] = FALSE;
				break;
			}
		}
	}
}

MultSummaryThread_Run::~MultSummaryThread_Run()
{

	if (nullptr != m_qslResultEachCamera)
	{
		delete[] m_qslResultEachCamera;
		m_qslResultEachCamera = nullptr;
	}
	if (b_eachalreadyfinish != nullptr)
	{
		delete b_eachalreadyfinish;
		b_eachalreadyfinish = nullptr;
	}
}

void MultSummaryThread_Run::ThreadSummary(QString str, int pos, int timeincircle)//str 传的数据，pos 0-4
{

	SYS_logger->info("ThreadSummary log pos No.{} ,timeincircle No.{}", pos, timeincircle);
	// 	if (pos >= m_icamcount|| str =="")
	// 	{
	// 		return;
	// 	}
		//第一个为赋值
		//只有一个相机 pos->0,1
		//g_PhotoTimes = 3
	if (0 == timeincircle % g_PhotoTimes)									//timeincircle 0...999999999999 某通道的第几张图
	{
		//m_qslResultEachCamera 汇总前timeincircle 的结果				
		//timeincircle =0;	str "Good,Error1,Error2,Good,Good,Good"
		//m_qslResultEachCamera[0] = "Good“,”Error1”,“Error2”,“Good”,“Good”,“Good"
		m_qslResultEachCamera[pos] = str.split(",");
	}
	else
	{
		//timeincircle = 1;	str "ErrorA,Error1,Error2,Good,Good,Good"
		//timeincircle = 2;	str "ErrorB,Good,Error2,Good,Good,Good"
		//若算法检测为error，则覆盖m_qslResultEachCamera对应值
		QStringList tem = str.split(",");
		//tem = "ErrorA“,”Error1”,“Error2”,“Good”,“Good”,“Good"
		for (int i = 0; i < tem.size(); i++)
			//6
		{
			//	true					6
			if (tem[i] != "	"&&i < m_qslResultEachCamera[pos].size())
			{//m_qslResultEachCamera[0] = "Good“,”Error1”,“Error2”,“Good”,“Good”,“Good"
				m_qslResultEachCamera[pos][i] = tem[i];
				//m_qslResultEachCamera[0] = "ErrorA“,”Error1”,“Error2”,“Good”,“Good”,“Good"
			}
		}
		if (0 == (timeincircle + 1) % g_PhotoTimes)
		{
			b_eachalreadyfinish[pos] = TRUE;
		}
	}
	//timeincircle = 2
	if (0 == (timeincircle + 1) % g_PhotoTimes)
	{
		//m_qslResultEachCamera[0] = "ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good"
		bool _fini = TRUE;
		for (int i = 0; i < m_icamcount; i++)
		{
			if (b_eachalreadyfinish[i] == FALSE)
			{
				_fini = FALSE;
			}
		}
		if (!_fini)
		{
			return;
		}
		unsigned int result[4];
		result[0] = 0; result[1] = 0;
		result[2] = 0; result[3] = 0;

		//timeincircle = 2;m_qslResultEachCamera[0] = "Good“,”Good”,“Error2”,“Good”,“Good”,“Good"
		//timeincircle = 2;m_qslResultEachCamera[0] = "ErrorA“,”Error1”,“Error2”,“Good”,“Good”,“Good"



		//timeincircle = 2;m_qslResultEachCamera[1] = "Good“,”Good”,“Error2”,“Good”,“Good”,“Good"
		//timeincircle = 2;m_qslResultEachCamera[1] = "ErrorA“,”Error1”,“Error2”,“Good”,“Good”,“Good"


		//timeincircle = 2;m_qslResultEachCamera[2] = "Good“,”Good”,“Error2”,“Good”,“Good”,“Good"
		//timeincircle = 2;m_qslResultEachCamera[2] = "ErrorA“,”Error1”,“Error2”,“Good”,“Good”,“Good"






		//timeincircle = 2;m_qslResultEachCamera[0] = NULL;
		//timeincircle = 2;m_qslResultEachCamera[1] = NULL;
		//timeincircle = 2;m_qslResultEachCamera[2] = "ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good"
		//					一共检测胶囊通道数	18
		for (int i = 0; i < m_iResultAllList; i++)
		{
			int z = 0;
			//			6	m_qslResultEachCamera[1].size()=0;
			for (; z < m_qslResultEachCamera[i].size(); z++)
			{
				SYS_logger->info("xxThreadSummary Z= ++ i = {}, z={},{}", i, z, m_qslResultEachCamera[i][z].toLocal8Bit());
				if (m_qslResultEachCamera[i][z] != "Good" && m_qslResultEachCamera[i][z] != "DNull" && m_qslResultEachCamera[i][z] != "NULL")
				{
					int po = i * 6 + z;
					if (po < 8)
					{
						result[0] |= 1 << po;
						SYS_logger->info("00ThreadSummary Z= ++ i = {}, z={},{}", i, z, m_qslResultEachCamera[i][z].toLocal8Bit());
					}
					else if (po < 16)
					{
						result[1] |= 1 << (po - 8);
						SYS_logger->info("11ThreadSummary Z= ++ i = {}, z={},{}", i, z, m_qslResultEachCamera[i][z].toLocal8Bit());
					}
					else if (po < 24)
					{
						result[2] |= 1 << (po - 16);
						SYS_logger->info("22ThreadSummary Z= ++ i = {}, z={},{}", i, z, m_qslResultEachCamera[i][z].toLocal8Bit());
					}
					else if (po < 32)
					{
						result[3] |= 1 << (po - 24);
						SYS_logger->info("33ThreadSummary Z= ++ i = {}, z={},{}", i, z, m_qslResultEachCamera[i][z].toLocal8Bit());
					}
				}
				else
				{
					SYS_logger->info("44ThreadSummary Z= ++ i = {}, z={},{}", i, z, m_qslResultEachCamera[i][z].toLocal8Bit());
				}
				m_qslResultTOTALCamera.push_back(m_qslResultEachCamera[i][z]);
			}
			if (z == 0)
			{				//6
				for (; z < MAX_CAPSULECOUNT; z++)
				{
					int xy = i * 6 + z;
					if (xy < 8)
						result[0] |= 1 << xy;
					else if (xy < 16)
						result[1] |= 1 << (xy - 8);
					else if (xy < 24)
						result[2] |= 1 << (xy - 16);
					else if (xy < 32)
						result[3] |= 1 << (xy - 24);
					m_qslResultTOTALCamera.push_back("NULL");
				}
			}
			m_qslResultEachCamera[i].clear();
		}
		//m_qslResultTOTALCamera = "ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good","ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good","ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good"
		//m_qslResultTOTALCamera = "ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good","NULL","NULL","NULL","NULL","NULL","NULL","ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good","ErrorB“,”Error1”,“Error2”,“Good”,“Good”,“Good"


		/////++++++++++++++++++++++++++++++
		//g_iContinueErrorLimite = 10;

// 		int *continueLimite = new int[m_iResultAllList];
// 		for (int x = 0; x < m_qslResultTOTALCamera.size(); x++)
// 		{
// 			if (m_qslResultTOTALCamera[x] != "Good")
// 			{
// 				continueLimite[x]++;
// 			}
// 			else
// 				continueLimite[x] = 0;
// 		}

		SYS_logger->info("ThreadSummary log pos No.{} ,timeincircle No.{},{}|{}|{}|{}|{}", pos, timeincircle, m_qslResultTOTALCamera.join(",").toLocal8Bit(), result[0], result[1], result[2], result[3]);
		emit SUMMARYRESULTINCIRCLE(m_qslResultTOTALCamera);
#ifdef PLCCONNECT
		g_SocketPLC->SetResult(0, result);
#endif
		m_qslResultTOTALCamera.clear();
		b_eachalreadyfinish[pos] = false;
	}
}
