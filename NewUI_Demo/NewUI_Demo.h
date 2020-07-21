#pragma once

#include "AllRelayHead.h"
#include "QtLoginDlg.h"
#include <QtWidgets/QMainWindow>
#include "ui_NewUI_Demo.h"
//相机初始化和之后配置模块
#include "InitFunction.h"
//相机运行模块
#include "MultThread_Run.h"
//相机设置模块
#include "QtCameraSet.h"
//弹出窗口
#include "WindowOut.h"
#include "DailyLog.h"


typedef CBaseCheckAlg* (__stdcall *pExport)(bool b);
typedef void(__stdcall *pDeleteExport)(CBaseCheckAlg*);

struct LOADDLLANDRELEASE
{
	char *dllName;
	pExport LoadDLL;
	pDeleteExport UnLoadDLL;
	LOADDLLANDRELEASE()
	{
		dllName = new char[20];
	}
	~LOADDLLANDRELEASE()
	{
		delete dllName;
		dllName = NULL;
	}
 };

class MyMainLab:public QLabel
{
	Q_OBJECT
public:
	MyMainLab(QWidget*parent) 
		:QLabel(parent)
	{

	};
public slots:
	void showImage(int pos, Mat Img, int total)
	{
		imshow("", Img);
		return;
	}
};


class NewUI_Demo : public QMainWindow
{
	Q_OBJECT
signals:
	void STARTCHECK(int,bool);
 	void STOPCHECK();
 	void GetResult(QString letter, QString _CameraName);
 	void SendImage(Mat, QString);
 	void StartInitSingle();
 	void EveryResult2Summary(QString str);
 	void tempyellow();
	void QTCLOSEALLLED();
	//DLL显示在主线程显示图像
	void SignShowImage(int pos, Mat img, int checktimes);

	void INITCHECKCLASSSIGNAL();
	void SHOWEVERYPLCCONNECT(DataToPC_typ);

protected:
	QTimer *timer;
	QTimer *timerResize = nullptr;
	QTimer *timerreboot;
	QTimer *timerStart;
	QTimer *timerdoubleclick;
public:
	pDeleteExport m_UnLoadDLL;
	NewUI_Demo(QWidget *parent = Q_NULLPTR);
	int ThreadInit();
	~NewUI_Demo();

	void mousePressEvent(QMouseEvent *p);
	void mouseDoubleClickEvent(QMouseEvent *q);
	void showEvent(QShowEvent *p);
	bool LoadImportantValue();
	//完成所有初始化工作
	bool FinishInitOther();
	//加载算法动态库
	int LoadAlgorithmDLL();
	//分配每个相机的算法
	bool InitCheckClass();
	//初始化检测队列
	bool InitPicList();
	//连接弱电控制
	bool OpenConnect();
	char m_Result[30];
	int showMsgBox(QMessageBox::Icon icon,const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//全是中文
	void NewUI_Demo::createFile(QString filePath, QString fileName);//为创建txt文件，名字是模板名
	QString NewUI_Demo::txtFilenameInDir(QString path);//截取唯一的.txt文件名
	void splitpixmap(QPixmap& pixmap, int xnum, int ynum);//切图
	void initLogLine();
	
	//获取pc数据
	DataFromPC_typ getPCData(); 
	void initPictureListWidget(); 
	bool InitShowImgList(QString _dir);

	quint64 getDiskFreeSpace(QString driver);

	void onConnectPort();//232连接
	void adjustBrightness();//亮度调节
private:
	Ui::NewUI_DemoClass ui;
	//相机控制模块
	InitFunction *Camera_Func;
	//算法多线程模块
	QVector<MultDecodeThread_Run*> m_MultDecodeThread;
	QVector<QThread*> QTH_MultDecodeThread;
	//检测结果发送线程
	MultSaveThread_Run* m_MultSaveThread;
	QThread* QTH_MultSaveThread;
	//检测结果统计线程
	MultSummaryThread_Run* m_MultSummaryThread;
	QThread* QTH_MultSummaryThread;
	
	//删除图片线程
	MultDeleteTifThread_Run* m_MultDeleteTifThread;
	QThread* QTH_MultDeleteTifThread;
	//是否正在检测标识符
	bool m_bStarting;
	//状态栏显示
	QLabel *normalshow;
	//系统时间
	QDateTime current_time;
	//初始化线程
	MultInit_Run *m_MultInit;
	QThread *QTH_Init;
	//初始化完成标识符
	bool m_bAllInited;
	//标记是否在检测
	QString m_SLabelStatue;

	QString m_IniResultPath;//ini result 路径，按下开始的时候
	QString timeForAllResult;//单个结果合并为一个ini需要的日期事件字符串
	WindowOut *levelOut;//show默认为非模态modal，如果是局部变量会闪现消失

	QList<QPixmap> m_pixlist;//图片切割
	QMovie* movie;//
	//QTimer *m_myTimer;// 定时器对象
	DailyLog* m_log;

	bool b_changingSpeed;//修改速度中
	QString m_selectedPath;//选中图片所在路径
	int m_pictureLevel;		//图片目录在第几级
	bool m_pictureHidden;	//图片是否显示
	DataToPC_typ *m_data;	//获取的PLC数据
	bool m_bWaitForPLCStop = false; //PLC回到零点

	int m_showSwitch = 0;
	QSerialPort *m_serialPort = nullptr;
	int m_write = 0;
public slots:
	void onTest(); 
	void onHome();
	void onStartCheck(bool);
	void onStopCheck();
	void closeEvent(QCloseEvent *eve);
	void onTestSignal();
	void onCameraSet();
	//DLL显示在主线程显示图像
	void SLOTShowImage(int pos, Mat img, int checktimes);
	//显示统计结果
	bool SlotShowResult(QStringList);
	void AlarmResetCommand();
	void SetEvertDlg();

	void on_Button_CountReset_clicked();

	void changeRunSpeed();
	void SWITCHOSK();//快捷键
	void PNChanged();

	void receiveInfo();//read 232
	void onCircleWrite();//write 232
#ifdef PLCCONNECT
	void showPLCValue(DataToPC_typ);
#endif

	//分配每个相机的算法
	void InitCheckClassSLOT();

	void changeListWidgetItem(QListWidgetItem* item);//显示图片
};
