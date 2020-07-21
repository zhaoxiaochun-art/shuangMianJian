#pragma once

//#define TESTINCOMPUTER
#include <QVector>
#include <QSettings>
#include <QApplication>
#include <QLabel>
#include <QThread>
#include <QMessageBox>
#include <QGraphicsScene>



typedef void*   UI_MONITOR;
#include "OpencvRelay.h"


#include "QT_FrontCheckClass_global.h"


#define WM_USER_CLOSE			WM_USER+10009	//关闭

#ifdef MATUREAPPROACH_EXPORTS
#define MATUREAPPROACH_API  __declspec(dllexport)
#else
#define MATUREAPPROACH_API __declspec(dllimport)
#endif

struct CHECKPARAM
{
	int i_TypeOfCamera;
	char c_CameraName[20];
	int i_CheckPosNo;
	char c_OperateCore[20];
	//////////////////////////////////////////////////////////////////////////
	int i_tempThread;
	int i_CapsuleCount;
	//////////////////////////////////////////////////////////////////////////
	CHECKPARAM()
	{
		i_tempThread = -1;
		i_CapsuleCount = -1;
	}
	~CHECKPARAM()
	{

	}
};

struct IMAGEPACKAGE
{
	int i_timeCheck;
	int i_imgWidth;
	int i_imgHeight;
	int i_imgChannel;
	int i_indexofpos;
	LARGE_INTEGER st;
	SYSTEMTIME sttime;
	bool b_OKORNG;
	uchar* p_bufferIMG;
	int rows;
	int columns;
	double d_dx;

	IMAGEPACKAGE(IMAGEPACKAGE* img)
	{
		st = img->st;
		sttime = img->sttime;
		i_timeCheck = img->i_timeCheck;
		i_imgWidth = img->i_imgWidth;
		i_imgHeight = img->i_imgHeight;
		i_imgChannel = img->i_imgChannel;
		p_bufferIMG = new uchar[i_imgChannel*i_imgHeight*i_imgWidth];
		memcpy(p_bufferIMG, img->p_bufferIMG, i_imgChannel*i_imgHeight*i_imgWidth);
		rows = 6709;
		columns = -1;
		d_dx = 0.0;
		b_OKORNG = img->b_OKORNG;
		i_indexofpos = -1;
	}
	IMAGEPACKAGE()
	{
		st = { 0 };
		sttime = { 0 };
		i_timeCheck = -1;
		i_imgWidth = -1;
		i_imgHeight = -1;
		i_imgChannel = -1;
		p_bufferIMG = NULL;
		rows = 6709;
		columns = -1;
		d_dx = 0.0;
		b_OKORNG = false;
	}
	~IMAGEPACKAGE()
	{
	}
};
struct pDataLength
{
	double* pData;
	double dmean;
	double ddeviation;
	int Length;
	int lcols;
	int rcols;
	pDataLength()
	{
		pData = NULL;
		Length = 0;
		dmean = 0.0;
		ddeviation = 0.0;
		lcols = 0;
		rcols = 0;
	}
	~pDataLength()
	{
	}
};

class CBaseCheckAlg:public QObject
{
	Q_OBJECT
signals:
	void SHOWIMGPOSTOTAL(int, Mat, int);
public:
	virtual void Release() = 0;
	virtual char* GetCameraName(void) = 0;
	virtual char* GetAlgName(void) = 0;
	virtual int SetParam(int _typeofcamera, char* _cameraName) = 0;
	virtual int ShowParamDlg(QWidget * parent, bool b_showornot) = 0;
	virtual int ReturnParam(int *_typeofcamera, char& _cameraName) = 0;
	//virtual int ParamDlg(RECT re) = 0;
	virtual int InitWindow(int pos, HANDLE _LEDhandle, void* _auhandle) = 0;
	//virtual int ReturnWindow(CWnd **hwnd, RECT &rect, CWnd **hwnd2, RECT &rect2) = 0;
	virtual int GetCheckPosNo() = 0;
	virtual void StartCheck(QString camerasign,  std::shared_ptr<spd::logger> daily_logger) = 0;
	virtual void StopCheck() = 0;
	virtual QString GetResult() = 0;
	virtual bool Check(Mat imgpackage, CHECKPARAM *checkparam, QString &str) = 0;
	virtual void ShowResult(QVector<double*> &result) = 0;
	virtual void BeatStart(void) = 0;
	virtual void BeatEnd(void) = 0;
	virtual void* GetEncryptHandle() = 0;
	virtual void EnableShow(bool) = 0;
	virtual void TESTSETSHOW(void*) =0;
	//设置信息回调函数
	typedef void (*CallbackText)(UI_MONITOR ui, char* i_result);
	virtual void SetResultCallBack(UI_MONITOR ui, CallbackText callbackfun) = 0;
	//设置图像回调函数
	typedef void(*CallbackImage)(UI_MONITOR ui, int pos, Mat img, int times);
	virtual void SetShowCallBack(UI_MONITOR ui, CallbackImage callbackfun) = 0;
	//设置关闭回调函数
	typedef void(*CallbackClose)();
	virtual void SetCloseCallBack(CallbackClose callbackfun) = 0;
};






extern "C" QT_FrontCheckClass_EXPORT CBaseCheckAlg* APIENTRY CreateExportObj(bool b_init);
extern "C" QT_FrontCheckClass_EXPORT void APIENTRY DestroyExportObj(CBaseCheckAlg* pExport);
