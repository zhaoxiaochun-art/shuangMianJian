#pragma once
#include "AllRelayHead.h"
#include <QObject>
class MultInit_Run : public QWidget
{
	Q_OBJECT
		signals:
	void InitSingle(QString str);//初始化信号，由下面的槽函数发送👇👇👇
	void StartsSingle();//开始信号
	void PlaySoundPath(QString str);//声音路径信号
private:
	QWidget* _parent;//一个对象
public:
	MultInit_Run(QWidget *parent);
	~MultInit_Run();
	public slots:
	int ThreadInit();//线程初始化，用于发送👆👆👆初始化信号给Dammy类对象，return -1；
	int CloseTh();//return 0；
};

#ifdef BASLER
class MultGetThread_Run : public QObject, public Pylon::CImageEventHandler
{
	Q_OBJECT
signals:
	void StartSingle();
	void RESTARTSIGNAL();
	void GETONEIMAGEMAT(Mat);
	void SHOWDIRECTIMG(Mat);
public:
	virtual void OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult);


private:
	int m_iSelfIndex;
	int totalcount;
	int sizetotal;
	bool m_bAlRegister;
	void * m_LabelShow;
	bool m_bSave;
	Mat m_MatGetOnece;
	QString m_strFile;
public:
	MultGetThread_Run(QObject *parent);
	~MultGetThread_Run();
public slots:
	int ThreadGetImage(int,bool);
public:
	void SetMultIndex(int);
	void SetDirectShowDlg(void*);
	void SetSaveImage(bool);

};
#endif
#ifdef DAHENG
class MultGetThread_Run : public QObject, public \
{
	Q_OBJECT
signals:
	void StartSingle();
	void RESTARTSIGNAL();
	void GETONEIMAGEMAT(Mat);
public:
	virtual void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam);

private:
	int m_iSelfIndex;
	int totalcount;
	int sizetotal;
	bool m_bAlRegister;
	void * m_LabelShow;
	Mat m_MatGetOnece;
public:
	MultGetThread_Run(QObject *parent);
	~MultGetThread_Run();
public slots:
	int ThreadGetImage(int);
public:
	void SetMultIndex(int);
	void SetDirectShowDlg(void*);

};
#endif


class MultDecodeThread_Run : public QObject
{
	Q_OBJECT
signals:
	void SAVESIGNAL(Mat, QString);
	void OUTRESULTSUMMARY(QString, int, int);
private:
	int m_iSelfIndex;
	uint m_iResult;	//单次检测结果，最大不超过八个
	int i_captotal = 0;
	int m_iSaveOKorNG;
	int i_SaveLoop;
	long timecheck;
	int index_pos;
	LARGE_INTEGER litmpst, litmpend, freq;
	double dfTim = 0.0;
	QString okdir_str;
	QString ngdir_str;
public:
	MultDecodeThread_Run(QObject *parent);
	~MultDecodeThread_Run();
	CBaseCheckAlg *_CheckClass;

public slots:
	int ThreadDecodeImage(int,bool);
	int ThreadDecodeImageMat(Mat);
public:
	void SetMultIndex(int);

};

class MultSaveThread_Run : public QObject
{
	Q_OBJECT
private:
	int m_iSelfIndex;
	int m_icamcount;
public:
	MultSaveThread_Run(QObject *parent, int camcount);
	~MultSaveThread_Run();
signals:
	void DELETETIF(QString str_tif);
public slots:
	void ThreadSave(Mat, QString);
};

class MultDeleteTifThread_Run : public QObject
{
	Q_OBJECT
private:
	int m_iSelfIndex;
	int m_icamcount;
public:
	MultDeleteTifThread_Run(QObject *parent);
	~MultDeleteTifThread_Run();
public slots:
	//everything
	void evethingSearchResults(QString str);
};

class MultSummaryThread_Run : public QObject
{
	Q_OBJECT
signals:
	void SUMMARYRESULTINCIRCLE(QStringList);
private:
	int m_iSelfIndex;
	int m_icamcount;
	int m_iResultAllList;
	double avg;
	bool *b_eachalreadyfinish;
	QStringList *m_qslResultEachCamera;
	QStringList m_qslResultTOTALCamera;
public:
	MultSummaryThread_Run(QObject *parent,int camcount);
	~MultSummaryThread_Run();
public slots:
	void ThreadSummary(QString,int,int);
};