#pragma once

#include "AllRelayHead.h"
#include "QtLoginDlg.h"
#include <QtWidgets/QMainWindow>
#include "ui_NewUI_Demo.h"
//�����ʼ����֮������ģ��
#include "InitFunction.h"
//�������ģ��
#include "MultThread_Run.h"
//�������ģ��
#include "QtCameraSet.h"
//��������
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
	//DLL��ʾ�����߳���ʾͼ��
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
	//������г�ʼ������
	bool FinishInitOther();
	//�����㷨��̬��
	int LoadAlgorithmDLL();
	//����ÿ��������㷨
	bool InitCheckClass();
	//��ʼ��������
	bool InitPicList();
	//�����������
	bool OpenConnect();
	char m_Result[30];
	int showMsgBox(QMessageBox::Icon icon,const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//ȫ������
	void NewUI_Demo::createFile(QString filePath, QString fileName);//Ϊ����txt�ļ���������ģ����
	QString NewUI_Demo::txtFilenameInDir(QString path);//��ȡΨһ��.txt�ļ���
	void splitpixmap(QPixmap& pixmap, int xnum, int ynum);//��ͼ
	void initLogLine();
	
	//��ȡpc����
	DataFromPC_typ getPCData(); 
	void initPictureListWidget(); 
	bool InitShowImgList(QString _dir);

	quint64 getDiskFreeSpace(QString driver);

	void onConnectPort();//232����
	void adjustBrightness();//���ȵ���
private:
	Ui::NewUI_DemoClass ui;
	//�������ģ��
	InitFunction *Camera_Func;
	//�㷨���߳�ģ��
	QVector<MultDecodeThread_Run*> m_MultDecodeThread;
	QVector<QThread*> QTH_MultDecodeThread;
	//����������߳�
	MultSaveThread_Run* m_MultSaveThread;
	QThread* QTH_MultSaveThread;
	//�����ͳ���߳�
	MultSummaryThread_Run* m_MultSummaryThread;
	QThread* QTH_MultSummaryThread;
	
	//ɾ��ͼƬ�߳�
	MultDeleteTifThread_Run* m_MultDeleteTifThread;
	QThread* QTH_MultDeleteTifThread;
	//�Ƿ����ڼ���ʶ��
	bool m_bStarting;
	//״̬����ʾ
	QLabel *normalshow;
	//ϵͳʱ��
	QDateTime current_time;
	//��ʼ���߳�
	MultInit_Run *m_MultInit;
	QThread *QTH_Init;
	//��ʼ����ɱ�ʶ��
	bool m_bAllInited;
	//����Ƿ��ڼ��
	QString m_SLabelStatue;

	QString m_IniResultPath;//ini result ·�������¿�ʼ��ʱ��
	QString timeForAllResult;//��������ϲ�Ϊһ��ini��Ҫ�������¼��ַ���
	WindowOut *levelOut;//showĬ��Ϊ��ģ̬modal������Ǿֲ�������������ʧ

	QList<QPixmap> m_pixlist;//ͼƬ�и�
	QMovie* movie;//
	//QTimer *m_myTimer;// ��ʱ������
	DailyLog* m_log;

	bool b_changingSpeed;//�޸��ٶ���
	QString m_selectedPath;//ѡ��ͼƬ����·��
	int m_pictureLevel;		//ͼƬĿ¼�ڵڼ���
	bool m_pictureHidden;	//ͼƬ�Ƿ���ʾ
	DataToPC_typ *m_data;	//��ȡ��PLC����
	bool m_bWaitForPLCStop = false; //PLC�ص����

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
	//DLL��ʾ�����߳���ʾͼ��
	void SLOTShowImage(int pos, Mat img, int checktimes);
	//��ʾͳ�ƽ��
	bool SlotShowResult(QStringList);
	void AlarmResetCommand();
	void SetEvertDlg();

	void on_Button_CountReset_clicked();

	void changeRunSpeed();
	void SWITCHOSK();//��ݼ�
	void PNChanged();

	void receiveInfo();//read 232
	void onCircleWrite();//write 232
#ifdef PLCCONNECT
	void showPLCValue(DataToPC_typ);
#endif

	//����ÿ��������㷨
	void InitCheckClassSLOT();

	void changeListWidgetItem(QListWidgetItem* item);//��ʾͼƬ
};
