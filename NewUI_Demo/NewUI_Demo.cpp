#include "NewUI_Demo.h"
#include "ResultData.h"
#include <QMovie>
#include <QBitmap>//圆角
#include <QPainter>//圆角
#include <QPixmap>
#include "textticker.h"

//日志工具
std::shared_ptr<spd::logger> SYS_logger;//系统
std::shared_ptr<spd::logger> ALM_logger;//报警
std::shared_ptr<spd::logger> OPS_logger;//操作

//操作员名称
QString g_QSUserName = "hanlinzhineng_123_nengzhilinhan_321";
int g_IUserLevel;
//exe运行path
QString AppPath;
QString style1;
QString style2;
//程序终止符
HANDLE g_bShutDown = CreateEventW(nullptr, true, true, NULL);//默认，false自动复位|true手动复位，初始值，名称
//注册相机队列
QVector<CAMERASTRUCT*> g_vectorCamera;				//相机参数列表
//图像缓冲列表
QVector<CCycleBuffer*>	g_ImgBuffer;
//获取图像信号灯
QVector<HANDLE> WriteSign;
//USBKey句柄
HANDLE g_auhandle = (HANDLE)-1;
//算法方法库，共有多少种方法。
QVector<LOADDLLANDRELEASE*> dllVector;
//全局变量，串口句柄
HANDLE g_hCom = (HANDLE)-1;
//检测算法类实例化
QVector<CBaseCheckAlg*> g_CheckClass;						//检测类
//默认模板名称
QString g_qModelName;						//检测类
//检测类个数，通过InitCamera修改
int g_iCameraTotal = 0;
//每个线程控制开关
QVector <CONTROLPARAM> g_controlparam;
//采图多线程模块
QVector<MultGetThread_Run*> m_MultGetThread;
QVector<QThread*> QTH_MultGetThread;
//确定点击开始时间
QDateTime g_current_time;
#ifdef PLCCONNECT
Socket_CPP* g_SocketPLC = nullptr;
#endif
int g_PhotoTimes = 0;
int g_232Port;//调亮度
int MAX_CAPSULECOUNT;
QString g_ipAddress;
int g_port;

QString key_turnOff;

QString logpath1;//系统
QString logpath2;//报警
QString logpath3;//操作
QString saveImagePath;//图片存储路径
bool g_saveLoopFlag;//Everything delete flag 1删除 0不删
bool g_Deleting;

QString g_productionNumber="";//订单号默认为空
// void ResultCallBack(UI_MONITOR ui, char* i_result)
// {
// 	((NewUI_Demo*)ui)->emit EveryResult2Summary(i_result);
// }
void ShowCallBack(UI_MONITOR ui, int pos, Mat img, int checktimes)
{
	((NewUI_Demo*)ui)->emit SignShowImage(pos, img, checktimes);
}

void CloseCallBack()
{
}

void _handlers(void* context, DataToPC_typ data)
{
	((NewUI_Demo*)context)->emit SHOWEVERYPLCCONNECT(data);
}


NewUI_Demo::NewUI_Demo(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowIcon(QIcon("./ico/dr.ico"));//文件copy到了exe所在目录
	setWindowFlags(Qt::FramelessWindowHint);//无边框  

	m_data = new DataToPC_typ();//跑马灯需要
	memset(m_data, 0, sizeof(DataToPC_typ));
	//m_myTimer = new QTimer(this);
	//connect(m_myTimer, &QTimer::timeout, [=]() {
	//	showPLCValue(*m_data);
	//	});
	//m_myTimer->start(1000);//通讯用

	ui.lb_Alarm->m_showText = QString::fromLocal8Bit("设备未就绪！");
	ui.lb_Alarm->setStyleSheet("color: rgb(255,0,0);font-size:20pt");

	qRegisterMetaType<Mat>("Mat");
	qRegisterMetaType<Mat>("Mat&");//注册opencv函数，在槽函数中避免出错
	qRegisterMetaType<DataToPC_typ>("DataToPC_typ");
	qRegisterMetaType<DataToPC_typ>("DataToPC_typ&");//注册DataToPC_typ函数，在槽函数中避免出错
	AppPath = qApp->applicationDirPath();//exe所在目录

	QtLoginDlg* dlg = new QtLoginDlg;
	dlg->exec();

	QSettings readPara(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
	key_turnOff = readPara.value("ProgramSetting/AutoClose", "").toString();

	if (g_QSUserName == "hanlinzhineng_123_nengzhilinhan_321")
	{
		char *pathvar = getenv("CODERCOMPUTER");
		QString envStr=QString::fromLocal8Bit(pathvar);
		if (key_turnOff == "1"&&envStr!="coder")
		{
			QProcess pro;    //通过QProcess类来执行第三方程序
			QString cmd = QString("shutdown -s -t 0"); //shutdown -s -t 0 是window下的关机命令，

			pro.start(cmd);    //执行命令cmd
			pro.waitForStarted();
			pro.waitForFinished();
			//close();    //关闭上位机
		}
		exit(-1);
	}

	b_changingSpeed = 0;
	ui.cB_RunSpeed->setVisible(false);
	ui.cB_RunSpeed->move(372, 7);
	connect(ui.lE_PN, SIGNAL(POPUPKEYBOARD()), this, SLOT(SWITCHOSK()));

	connect(ui.lE_PN, SIGNAL(returnPressed()), this, SLOT(PNChanged()));

	QRegExp regx("[a-zA-Z0-9_]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui.lE_PN->setValidator(new QRegExpValidator(regx, this));

	connect(ui.lE_RunSpeed, SIGNAL(POPUPKEYBOARD()), this, SLOT(changeRunSpeed()));
	connect(ui.cB_RunSpeed, QOverload<const QString&>::of(&QComboBox::activated),
		[=](const QString& text) {
		QString str_Before = ui.lE_RunSpeed->text();
		b_changingSpeed = 1;
		if (text != QString::fromLocal8Bit("取消"))
		{
			if (text == str_Before)
			{
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("运行速度未作修改"), 2000);
				levelOut->show();
			}
			else if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "修改确认", "<img src = './ico/question.png'/>\t确认修改运行速度吗？", "确认", "取消"))
			{
				ui.lE_RunSpeed->setText(text);
				DataFromPC_typ typ;
				typ = getPCData();
				typ.Dummy = 0;//占空
				typ.Telegram_typ = 4;//运行
				typ.Run_Para.RunSpeed = ui.lE_RunSpeed->text().toInt();
				g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收
				SYS_logger->info("运行速度修改为{}",text.toInt());
			}
		}
		else
		{
			ui.lE_RunSpeed->setText(str_Before);
		}
		b_changingSpeed = 0;
	});

	current_time = QDateTime::currentDateTime();
	QString logYear = QString::number(current_time.date().year());
	logYear = logYear.length() < 4 ? ("0" + logYear) : logYear;
	QString logMonth = QString::number(current_time.date().month());
	logMonth = logMonth.length() < 2 ? ("0" + logMonth) : logMonth;
	QString logDay = QString::number(current_time.date().day());
	logDay = logDay.length() < 2 ? ("0" + logDay) : logDay;
	QString logHour = QString::number(current_time.time().hour());
	logHour = logHour.length() < 2 ? ("0" + logHour) : logHour;
	QString logMinute = QString::number(current_time.time().minute());
	logMinute = logMinute.length() < 2 ? ("0" + logMinute) : logMinute;
	logpath1 = AppPath + "/logs/" + g_QSUserName.mid(5) + "/System/System_daily_"
		+ logYear + "_" //z=a>b?x:y
		+ logMonth + "_"
		+ logDay + "_"
		+ logHour + "_"
		+ logMinute + ".txt";//eg:MainDLG_daily_2020_02_29_23_47.txt
	SYS_logger = spd::basic_logger_mt("SYS_logger", logpath1.toStdString());//标准logger格式，每行信息头名称为daily_logger
	SYS_logger->flush_on(spd::level::trace);//初始化一次

	logpath2 = AppPath + "/logs/" + g_QSUserName.mid(5) + "/Alarm/Alarm_daily_"
		+ logYear + "_" //z=a>b?x:y
		+ logMonth + "_"
		+ logDay + "_"
		+ logHour + "_"
		+ logMinute + ".txt";//eg:MainDLG_daily_2020_02_29_23_47.txt
	ALM_logger = spd::basic_logger_mt("ALM_logger", logpath2.toStdString());//标准logger格式，每行信息头名称为daily_logger
	ALM_logger->flush_on(spd::level::trace);//初始化一次	

	logpath3 = AppPath + "/logs/" + g_QSUserName.mid(5) + "/Operation/Operation_daily_"
		+ logYear + "_" //z=a>b?x:y
		+ logMonth + "_"
		+ logDay + "_"
		+ logHour + "_"
		+ logMinute + ".txt";//eg:MainDLG_daily_2020_02_29_23_47.txt
	OPS_logger = spd::basic_logger_mt("OPS_logger", logpath3.toStdString());//标-2
	OPS_logger->flush_on(spd::level::trace);//初始化一次

	char dest[100], dest2[100];//太小会溢出

	strcpy(dest2, "您的用户权限为： ");
	switch (g_IUserLevel)
	{
	case 0:strcpy(dest, "管理员"); break;
	case 1:strcpy(dest, "工程师"); break;
	case 2:strcpy(dest, "操作员"); break;
	case 3:strcpy(dest, "质检员"); break;
	default:break;

	}

	strcat(dest2, dest);

	ui.Button_DataOut->setEnabled(false);
	if (g_IUserLevel == 0)
	{
		ui.Button_DataOut->setEnabled(true);
	}
	else if (g_IUserLevel == 2)//权限按钮
	{
		ui.Button_CameraSet->setEnabled(false);
	}
	else if (g_IUserLevel == 3)
	{
		ui.Button_CameraSet->setEnabled(false);
		ui.Button_Start->setEnabled(false);
		ui.Button_Home->setEnabled(false);
		ui.Button_DataOut->setEnabled(true);
		ui.Button_AlarmReset->setEnabled(false);
		ui.Button_CountReset->setEnabled(false);
	}
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
	QString text = configIniRead.value("ProgramSetting/Style", "").toString();

	if (text == "Default Style")
	{
		style1 = "";
		style2 = "";
	}
	else if (text == "Iron Man")
	{
		style1 = "background-image:url(./ico/rev.png);";
		style2 = "background-image:url(./ico/rev2.png);";
	}
	else if (text == "zxc")
	{
		style1 = "background-image:url(./ico/zxc.png);";
		style2 = "background-image:url(./ico/zxc2.png);";
	}
	else if (text == "qdh")
	{
		style1 = "background-image:url(./ico/qdh.png);";
		style2 = "background-image:url(./ico/qdh2.png);";
	}

	ui.lb_style->raise();
	ui.lb_style->setStyleSheet(style1);
	ui.lb_style->setAttribute(Qt::WA_TransparentForMouseEvents);


	//👆👆👆👆👆👆
	levelOut = new WindowOut;
	levelOut->getString(QString::fromLocal8Bit(dest2), 2000);
	levelOut->show();

	SYS_logger->info("{}登录成功", dest);

	movie = new QMovie("./ico/capsule.gif");
	movie->setScaledSize(QSize(740, 660));
	ui.label->setMovie(movie);
	ui.label->setScaledContents(true);
	movie->start();

	//everyThing d盘20开头.tif结尾的文件 everything需要开机自启动，后台运行
	//evethingSearchResults("^[2]0...*.tif$");//找文件
	//evethingSearchResults("tif$");//在机器上可以用
	g_saveLoopFlag = 0;//Everything delete flag
	g_Deleting=0;//1删除中 0不在删除中
	//向上↑
	m_bAllInited = false;//初始化完成：ini 相机
	Camera_Func = nullptr;//包含bool GetAllCamera();int ReadConfig();
	m_bStarting = false;//是否在检测标识符
	m_SLabelStatue = "";//标记是否在检测


	connect(this, SIGNAL(INITCHECKCLASSSIGNAL()), this, SLOT(InitCheckClassSLOT()));
	timer = new QTimer(this);
	timer->setSingleShot(true);//单次触发
	m_MultInit = new MultInit_Run(this);//对多线程初始化的类
	bool flag = QObject::connect(timer, SIGNAL(timeout()), m_MultInit, SLOT(ThreadInit()));//window启动时执行一线程的初始化   read111

	QTH_Init = new QThread();//新线程
	m_MultInit->moveToThread(QTH_Init);//将槽函数放到新线程中去执行
	QTH_Init->start();//开始执行

	connect(ui.Button_AlarmReset, SIGNAL(clicked()), this, SLOT(AlarmResetCommand()));
	ui.Button_Start->setEnabled(false);//起初开始按钮为不可用状态
	connect(ui.Button_Start, SIGNAL(toggled(bool)), this, SLOT(onStartCheck(bool)));//checked button
	connect(ui.Button_Home, SIGNAL(clicked()), this, SLOT(onHome()));//not checked button
	connect(ui.Button_CameraSet, SIGNAL(clicked()), this, SLOT(onCameraSet()));//not checked button
	connect(ui.Button_DataOut, &QPushButton::clicked, [=]() {
		OPS_logger->info("数据导出按下");
		ResultData* rd = new ResultData(this);
		rd->exec();
	});

	connect(ui.Button_Log, &QPushButton::clicked, [=]() {
		OPS_logger->info("日志查询按下");
		m_log = new DailyLog(this);
		m_log->show();
	});

	connect(ui.Button_Exit, &QPushButton::clicked, [=]() {

		OPS_logger->info("退出按下");
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "退出确认", "<img src = './ico/question.png'/>\t确认退出系统吗？", "确认", "取消"))
		{
			SYS_logger->info("系统退出");
			close();
		}
		SYS_logger->info("系统退出取消");
	});

	flag = connect(this, SIGNAL(SignShowImage(int, Mat, int)), this, SLOT(SLOTShowImage(int, Mat, int)));//openCV有关

	//QFont fo = ui.tableWidget_Result->font();
	QFont fo(QString::fromLocal8Bit("幼圆"), 20);
	//fo.setPointSize(16);
	ui.tableWidget_Result->setFont(fo);//设置tableWidget字体
	QStringList title;
	title << QString::fromLocal8Bit("ErrorType") << QString::fromLocal8Bit("类型") << QString::fromLocal8Bit("数量");
	ui.tableWidget_Result->setColumnCount(3);//3列
	ui.tableWidget_Result->setHorizontalHeaderLabels(title);//加表头
	ui.tableWidget_Result->setColumnHidden(0, true);//隐藏ErrorType列
	ui.tableWidget_Result->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分填充表头
	ui.tableWidget_Result->verticalHeader()->setDefaultSectionSize(35);//默认行高20
	ui.tableWidget_Result->verticalHeader()->setVisible(false);//不显示行头
	int z = 0;
	ui.tableWidget_Result->insertRow(z);//加一行
	ui.tableWidget_Result->setItem(z, 0, new QTableWidgetItem(QString::fromLocal8Bit("Total")));//第0列，已隐藏
	ui.tableWidget_Result->setItem(z, 1, new QTableWidgetItem(QString::fromLocal8Bit("总数")));//第1列
	ui.tableWidget_Result->setItem(z, 2, new QTableWidgetItem(QString::number(0)));//第2列

	z++;
	ui.tableWidget_Result->insertRow(z);//再加一行
	ui.tableWidget_Result->setItem(z, 0, new QTableWidgetItem(QString::fromLocal8Bit("Good")));//第0列，已隐藏
	ui.tableWidget_Result->setItem(z, 1, new QTableWidgetItem(QString::fromLocal8Bit("合格数")));//第1列
	ui.tableWidget_Result->setItem(z, 2, new QTableWidgetItem(QString::number(0)));//第2列

	LoadImportantValue();
	/*******************eg：
	[ProgramSetting]
	PhotoTimes = 4				//一个胶囊拍几次
	MAX_CAPSULECOUNT = 6		//每个相机拍胶囊数量
	DefaultModel = testA		//默认模板
	IpAddress = 10.86.50.210	//PLC IP
	port = 5000					//PLC端口
	********************/


	ui.lb_Picture->setVisible(false);//图片显示
	ui.lb_Picture->setScaledContents(true);//内容尺寸可变
	m_pictureLevel = 1;//列表初始在哪页
	m_pictureHidden = 1;//初始取消不显示
	connect(ui.lW_Picture, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(changeListWidgetItem(QListWidgetItem*)));

}
bool sortcameras(CAMERASTRUCT *a, CAMERASTRUCT *b)//排序方法
{
	QString x = a->c_CameraSign;
	QString y = b->c_CameraSign;
	return x.mid(3).toInt() < y.mid(3).toInt();
}
int NewUI_Demo::ThreadInit()
{
	QDateTime current_time = QDateTime::currentDateTime();
	QString StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//emit InitSingle(StrCurrentTime + QString::fromLocal8Bit("-开始初始化！"));//发射唯一信号，没用

	InitFunction Camera_Func;//初始化相机
	Camera_Func.StartModel = true;
	Camera_Func.GetAllCamera();//包含ReadConfig(); read333

	//daily_logger->info("GetAllCamera Finished");
	qSort(g_vectorCamera.begin(), g_vectorCamera.end(), sortcameras);//相机快速排序
	g_iCameraTotal = g_vectorCamera.size();
	if (g_iCameraTotal > 0)//此处加上，cameraset的部分就不用了
	{
		for (int i = 0; i < g_vectorCamera.size(); i++)
		{
			if (!g_vectorCamera[i]->cb_Camera.IsOpen())
			{
				g_vectorCamera[i]->cb_Camera.Open();
				g_vectorCamera[i]->cb_Camera.UserSetDefault.SetValue(UserSetDefault_UserSet3);
				g_vectorCamera[i]->cb_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet3);
				g_vectorCamera[i]->cb_Camera.UserSetLoad.Execute();
				g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(false);
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
	//daily_logger->info("InitCamera Finished");
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化Camera类完成。"));
	QThread::msleep(200);
	LoadAlgorithmDLL();//加载算法
	SYS_logger->info("LoadAlgorithmDLL Finished");
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化加载算法完成。"));
	QThread::msleep(200);

	InitCheckClass();//每个相机检测初始化
	SYS_logger->info("InitCheckClass Finished");
	g_iCameraTotal = g_CheckClass.size();
	current_time = QDateTime::currentDateTime();
	StrCurrentTime = current_time.toString("hh:mm:ss:zzz");
	//((NewUI_Demo*)_parent)->ShowState(StrCurrentTime + QString::fromLocal8Bit("-初始化算法类完成。"));
	//QThread::msleep(200);
	OpenConnect();
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
	InitPicList();//初始化图像列表
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
	FinishInitOther();//其他初始化
	return -1;//发射-1
}

bool deleteDir(const QString& path)//删除路径
{
	if (path.isEmpty()) {//字符串类型路径为“”，返回0
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {//QDir类型路径不存在，返回1
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤，AllEntries 其值为Dirs | Files | Drives，列出目录、文件、驱动器及软链接等所有文件；不列出.和..（只有第一个过滤条件会有这两个东西存在）
	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
	foreach(QFileInfo file, fileList) { //遍历文件信息
		if (file.isFile()) { // 是文件，删除
			file.dir().remove(file.fileName());
		}
		else { // 递归删除
			deleteDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // 删除文件夹
}

NewUI_Demo::~NewUI_Demo()
{
#ifdef PLCCONNECT
	if (g_SocketPLC != nullptr)
	{
		g_SocketPLC->disconnect();//断开PLC
		delete g_SocketPLC;
		g_SocketPLC = nullptr;
	}
#endif
	deleteDir(AppPath + "/DefaultModel");
	disconnect(this, SIGNAL(SignShowImage(int, Mat, int)), this, SLOT(SLOTShowImage(int, Mat, int)));//openCV有关避免界面关掉还得接收
	disconnect(this, SIGNAL(SHOWEVERYPLCCONNECT(DataToPC_typ)), this, SLOT(showPLCValue(DataToPC_typ)));
	if (nullptr != m_MultSummaryThread)
	{
		delete m_MultSummaryThread;
		m_MultSummaryThread = nullptr;
	}
	if (nullptr != m_MultInit)
	{
		delete m_MultInit;
		m_MultInit = nullptr;
	}
	if (nullptr != m_MultSaveThread)
	{
		delete m_MultSaveThread;
		m_MultSaveThread = nullptr;
	}
	if (nullptr != QTH_MultSaveThread)
	{
		QTH_MultSaveThread->quit();
		QTH_MultSaveThread->wait();
		delete QTH_MultSaveThread;
		QTH_MultSaveThread = nullptr;
	}
	if (nullptr != QTH_MultSummaryThread)
	{
		QTH_MultSummaryThread->quit();
		QTH_MultSummaryThread->wait();
		delete QTH_MultSummaryThread;
		QTH_MultSummaryThread = nullptr;
	}

	if (QTH_Init != nullptr)
	{
		QTH_Init->quit();
		QTH_Init->wait();//释放对象锁
		delete QTH_Init;
		QTH_Init = nullptr;
	}
	for (int i = 0; i < g_vectorCamera.size(); i++)
	{
		g_vectorCamera[i]->cb_Camera.Close();
		g_vectorCamera[i]->cb_Camera.DestroyDevice();
		delete g_vectorCamera[i];
		g_vectorCamera[i] = nullptr;
	}
	for (int i = 0; i < dllVector.size(); i++)
	{
		delete dllVector[i];
		dllVector[i] = nullptr;
	}
	// Releases all pylon resources. 
	PylonTerminate();
	delete movie;
	delete m_data;

	char *pathvar = getenv("CODERCOMPUTER");
	QString envStr = QString::fromLocal8Bit(pathvar);
	if (key_turnOff == "1"&&envStr != "coder")
	{
		QProcess pro;    //通过QProcess类来执行第三方程序
		QString cmd = QString("shutdown -s -t 0"); //shutdown -s -t 0 是window下的关机命令，

		pro.start(cmd);    //执行命令cmd
		pro.waitForStarted();
		pro.waitForFinished();
	}
	//m_myTimer->stop();
	//delete m_myTimer; 

}


void NewUI_Demo::mousePressEvent(QMouseEvent* p)
{
}

void NewUI_Demo::mouseDoubleClickEvent(QMouseEvent* q)
{
}

void NewUI_Demo::showEvent(QShowEvent* p)
{

	if (!m_bAllInited)
	{
		timer->start(100);
	}
	// 	QPoint po = this->pos();
	// 	logDlg->move(po);
	// 	emit StartInitSingle();
}

bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist)
{
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if (!targetDir.exists()) {    /**< 如果目标目录不存在,则进行创建 */
		if (!targetDir.mkpath(targetDir.absolutePath()))//mkdir to mkpath 后者可以直接深入跨级创建，不必一级一级创建
		{
			return false;
		}
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    /**< 当为目录时,递归的进行copy */
			if (!copyDirectoryFiles(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else {            /**< 当允许覆盖操作时,将旧文件进行删除操作 */
			if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
				targetDir.remove(fileInfo.fileName());
			}

			/// 进行文件copy
			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}
bool NewUI_Demo::LoadImportantValue()
{
	/*******************eg：
	[ProgramSetting]
	PhotoTimes = 4				//一个胶囊拍几次
	MAX_CAPSULECOUNT = 6		//每个相机拍胶囊数量
	DefaultModel = testA		//默认模板
	IpAddress = 10.86.50.210	//PLC IP
	port = 5000					//PLC端口
	********************/
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
	g_PhotoTimes = configIniRead.value("ProgramSetting/PhotoTimes", 2).toInt();//第二个数是默认数值，如果不存在就用默认数值，下同
	g_232Port = configIniRead.value("ProgramSetting/232Port", 2).toInt();//第二个数是默认数值，如果不存在就用默认数值，下同
	
	adjustBrightness();

	MAX_CAPSULECOUNT = configIniRead.value("ProgramSetting/MAX_CAPSULECOUNT", 0).toInt();
	if (MAX_CAPSULECOUNT <= 0)
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("关键错误！未设置胶囊个数！"), 5000);
		levelOut->show();
	}
	g_qModelName = configIniRead.value("ProgramSetting/DefaultModel", "testA").toString();//当DefaultModel不存在时，返回testA
	QString modelfile = AppPath + "/ModelFile/" + g_qModelName;
	if (QDir(modelfile).isEmpty())//路径不存在
	{
		QMessageBox::critical(nullptr, QString::fromLocal8Bit("关键错误"), QString::fromLocal8Bit("默认模板不存在"));
		return false;
	}

	QDir finder(AppPath + "/DefaultModel");//finder为AppPath位置，找DefaultModel文件夹
	if (!finder.exists())//如果没有上述文件夹
	{
		copyDirectoryFiles(modelfile, AppPath + "/DefaultModel", true);//默认模板的东西拷入DefaultModel中
		g_qModelName = configIniRead.value("ProgramSetting/LastModel", g_qModelName).toString();//写当前模板全局变量 此时不是读
	}
	else//如果有这个文件夹
	{
		if (QMessageBox::No == showMsgBox(QMessageBox::Critical, "警告", "<img src = './ico/question.png'/>\t上次程序未能正常关闭,是否使用上次模板继续？", "是", "否"))
		{
			copyDirectoryFiles(modelfile, AppPath + "/DefaultModel", true);
			g_qModelName = configIniRead.value("ProgramSetting/DefaultModel", g_qModelName).toString();//写当前模板全局变量 此时不是读
				//此时不用再设置g_qModelName
		}
		else
		{
			//g_qModelName = txtFilenameInDir(AppPath + "/DefaultModel");//此时不一定是默认模板，而是应用的模板
			g_qModelName = configIniRead.value("ProgramSetting/LastModel", "testA").toString();
		}
	}
	return true;
}

void NewUI_Demo::createFile(QString filePath, QString fileName)//为创建txt文件，名字是模板名
{
	QDir tempDir;
	//临时保存程序当前路径
	QString currentDir = tempDir.currentPath();
	//如果filePath路径不存在，创建它
	if (!tempDir.exists(filePath))
	{
		qDebug() << "不存在该路径" << endl;
		tempDir.mkpath(filePath);
	}
	QFile* tempFile = new QFile;
	//将程序的执行路径设置到filePath下
	tempDir.setCurrent(filePath);
	qDebug() << tempDir.currentPath();
	//检查filePath路径下是否存在文件fileName,如果停止操作。
	if (tempFile->exists(fileName))
	{
		qDebug() << "文件存在";
		return;
	}
	//此时，路径下没有fileName文件，使用下面代码在当前路径下创建文件
	tempFile->setFileName(fileName);
	if (!tempFile->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "打开失败";
	}
	tempFile->close();
	//将程序当前路径设置为原来的路径
	tempDir.setCurrent(currentDir);
	qDebug() << tempDir.currentPath();
}

QString NewUI_Demo::txtFilenameInDir(QString path)//截取唯一的.txt文件名
{
	QDir dir(path);
	//查看路径中后缀为.cfg格式的文件
	QStringList filters;
	filters << QString("* .txt");
	dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
	dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式
	QString file_name = dir[0].left(dir[0].indexOf("."));//（.的位置）左边的东西截取下来
	return file_name;
}

bool NewUI_Demo::FinishInitOther()
{
	m_bAllInited = true;
	if (timerResize == nullptr)
	{
		timerResize = new QTimer();
		timerResize->setSingleShot(true);
		connect(timerResize, SIGNAL(timeout()), this, SLOT(SetEvertDlg()));
		timerResize->start(500);
	}

	return false;
}

int NewUI_Demo::LoadAlgorithmDLL()
{
	QDir finder(AppPath);//finder为AppPath位置
	if (!finder.exists())
	{
		return -1;
	}
	finder.setFilter(QDir::Files | QDir::NoSymLinks);//设置过滤器，setFilter（）设置搜索条件，QDir::Files - 只列出文件，QDir::NoSymLinks - 不列出符号连接（不支持符号连接的操作系统会忽略它）
	QFileInfoList list = finder.entryInfoList(); //通过entryInfoList则可以获取过滤后所得到的文件夹下的文件信息列表，遍历文件通过操作QFileInfo可得到所需的文件详细信息（大小、类型、后缀等）
	int file_count = list.count();
	if (file_count <= 0)
	{
		return -1;
	}
	QString Liststring_list;
	for (int i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString suffix = file_info.suffix();//文件后缀
		if (QString::compare(suffix, QString("dll"), Qt::CaseInsensitive) == 0)//CaseInsensitive不区分大小写比较
		{
			QString absolute_file_path = file_info.baseName();//绝对路径，包含文件但不包含后缀
			QString sss = absolute_file_path.right(10);//右侧10个字符
			if (absolute_file_path.right(10) == "CheckClass")
			{
				QLibrary dynamicDLL(file_info.filePath());
				if (dynamicDLL.load())
				{
					LOADDLLANDRELEASE* libiary = new LOADDLLANDRELEASE();
					libiary->LoadDLL = (pExport)dynamicDLL.resolve("_CreateExportObj@4");
					libiary->UnLoadDLL = (pDeleteExport)dynamicDLL.resolve("_DestroyExportObj@4");

					//
					strcpy(libiary->dllName, (absolute_file_path.mid(3, absolute_file_path.size() - 8)).toStdString().c_str());
					dllVector.push_back(libiary);//放入dllVector
					//delete ss;
				}
				else
				{
					QMessageBox::about(NULL, "", dynamicDLL.errorString());
				}

			}
		}
	}
	return 0;
}

bool NewUI_Demo::InitCheckClass()
{
	emit INITCHECKCLASSSIGNAL();
	return false;
}
void NewUI_Demo::InitCheckClassSLOT()
{
	QPixmap pix(AppPath + "/ico/dr-pharm.png");//

	int cou = g_vectorCamera.size();//配对的相机参数
	splitpixmap(pix, g_PhotoTimes, cou);//切图
	for (int i = 0; i < cou; i++)
	{
		for (int z = 0; z < g_PhotoTimes; z++)
		{
			ui.label->setVisible(false);//动画隐藏
			MyMainLab* label = new MyMainLab(this);

			// 			switch (z)//改颜色
			// 			{
			// 			case 0:
			// 				label->setStyleSheet("background-color:rgba(255,85,0,30);");
			// 				break;
			// 			case 1:
			// 				label->setStyleSheet("background-color:rgba(217,217,0,30);");
			// 				break;
			// 			case 2:
			// 				label->setStyleSheet("background-color:rgba(85,255,0,30);");
			// 				break;
			// 			case 3:
			// 				label->setStyleSheet("background-color:rgba(0,0,127,30);");
			// 				break;
			// 			case 4:
			// 				label->setStyleSheet("background-color:rgba(255,85,255,30);");
			// 				break;
			// 			case 5:
			// 				label->setStyleSheet("background-color:rgba(0,170,0,30);");
			// 				break;
			// 			default:
			//				break;
			// 			}
			label->setObjectName(QString::fromUtf8("LabelShow") + QString::number(i) + "_" + QString::number(z));
			label->setFrameShape(QFrame::Box);
			label->setLineWidth(1);

			label->setPixmap(m_pixlist.at(i + z * cou));//改切割图片

			label->setScaledContents(true);
			ui.gridLayout->addWidget(label, i, z, 1, 1);
		}
	}

	for (int i = 0; i < g_iCameraTotal; i++)//此时g_iCameraTotal== g_vectorCamera.size()
	{
		SYS_logger->info("InitCheckClass winId{}", i);
		SYS_logger->flush();

		pExport CExportDLL = NULL;//导出类
		for (int j = 0; j < dllVector.size(); j++)
		{
			if (strcmp(g_vectorCamera[i]->c_AlgName, dllVector[j]->dllName) == 0)
			{
				CExportDLL = dllVector[j]->LoadDLL;
				m_UnLoadDLL = dllVector[j]->UnLoadDLL;
				break;
			}
		}
		if (CExportDLL != NULL)
		{
			//QString sss = "LabelShow" + QString::number(i,10);
			CBaseCheckAlg* _CheckClass = CExportDLL(false);//是否是测试模式，false是正常，true是测试模式
			if (0 == i)
			{
				if (g_auhandle == (HANDLE)-1)
				{
					//g_auhandle = _CheckClass->GetEncryptHandle();
				}
			}
			if (NULL != _CheckClass)
			{

				_CheckClass->InitWindow(i, i == 0 ? g_auhandle : NULL, this);
				// 				for (int ii = 0;ii < g_PhotoTimes;ii++)
				// 				{
				// 					QLabel *lb = this->findChild<QLabel *>(QString::fromUtf8("LabelShow") + QString::number(i) + "_" + QString::number(ii));
				// 					if (lb != nullptr)
				// 					{
				// 						_CheckClass->TESTSETSHOW(lb);
				// 					}
				// 				}
								//设置算法库参数

				_CheckClass->SetParam(g_vectorCamera[i]->TypeOFCamera, g_vectorCamera[i]->c_CameraName);

				_CheckClass->SetShowCallBack(this, ShowCallBack);
				//_CheckClass->SetResultCallBack(this,ResultCallBack);
				_CheckClass->SetCloseCallBack(CloseCallBack);

				g_CheckClass.push_back(_CheckClass);
			}
		}
	}
	return;
}

bool NewUI_Demo::InitPicList()
{
#ifndef CALLBACKFUNC
	while (m_MultGetThread.size() != g_iCameraTotal)
	{
		m_MultGetThread.push_back(new MultGetThread_Run(nullptr));
	}
	while (QTH_MultGetThread.size() != g_iCameraTotal)
	{
		QTH_MultGetThread.push_back(new QThread);
	}
#endif
	while (m_MultDecodeThread.size() != g_iCameraTotal)
	{
		m_MultDecodeThread.push_back(new MultDecodeThread_Run(nullptr));
	}
	while (QTH_MultDecodeThread.size() != g_iCameraTotal)
	{
		QTH_MultDecodeThread.push_back(new QThread);
	}
	m_MultSaveThread = new MultSaveThread_Run(nullptr, g_vectorCamera.size());
	QTH_MultSaveThread = new QThread();
	m_MultSaveThread->moveToThread(QTH_MultSaveThread);
	QTH_MultSaveThread->start();

	m_MultDeleteTifThread = new MultDeleteTifThread_Run(nullptr);
	QTH_MultDeleteTifThread = new QThread();
	m_MultDeleteTifThread->moveToThread(QTH_MultDeleteTifThread);
	QTH_MultDeleteTifThread->start();

	m_MultSummaryThread = new MultSummaryThread_Run(nullptr, g_vectorCamera.size());
	QTH_MultSummaryThread = new QThread();
	m_MultSummaryThread->moveToThread(QTH_MultSummaryThread);
	QTH_MultSummaryThread->start();
	while (WriteSign.size() < g_iCameraTotal)
	{
		WriteSign.push_back(CreateEventW(NULL, true, true, NULL));
	}
	while (g_controlparam.size() < g_iCameraTotal)
	{
		CONTROLPARAM tempcontrol;
		tempcontrol.hOutTrigger = CreateEventW(NULL, false, false, NULL);
		tempcontrol.hPausle = CreateEventW(0, 1, 1, 0);
		tempcontrol.h_ReadFirstImg = CreateEventW(NULL, true, false, NULL);
		tempcontrol.hResultList = CreateEventW(NULL, true, false, NULL);
		tempcontrol.hResultTrigger = CreateEventW(NULL, false, false, NULL);
		tempcontrol.hFinishGetImage = CreateEventW(NULL, true, true, NULL);
		tempcontrol.hFinishDecodeLine = CreateEventW(NULL, true, true, NULL);
		tempcontrol.hNewImageArrive = CreateEventW(NULL, true, false, NULL);
		SetEvent(tempcontrol.hPausle);
		g_controlparam.push_back(tempcontrol);
	}
	bool flag = false;
	for (int i = 0; i < g_iCameraTotal; i++)
	{
		flag = QObject::connect(this, SIGNAL(STARTCHECK(int, bool)), m_MultGetThread[i], SLOT(ThreadGetImage(int, bool)), Qt::QueuedConnection);
		m_MultGetThread[i]->SetMultIndex(i);
		m_MultGetThread[i]->moveToThread(QTH_MultGetThread[i]);
		QTH_MultGetThread[i]->start();
		flag = QObject::connect(this, SIGNAL(STARTCHECK(int, bool)), m_MultDecodeThread[i], SLOT(ThreadDecodeImage(int, bool)), Qt::QueuedConnection);
		flag = QObject::connect(m_MultGetThread[i], SIGNAL(GETONEIMAGEMAT(Mat)), m_MultDecodeThread[i], SLOT(ThreadDecodeImageMat(Mat)), Qt::QueuedConnection);

		flag = QObject::connect(m_MultDecodeThread[i], SIGNAL(SAVESIGNAL(Mat, QString)), m_MultSaveThread, SLOT(ThreadSave(Mat, QString)), Qt::QueuedConnection);
		flag = QObject::connect(m_MultDecodeThread[i], SIGNAL(OUTRESULTSUMMARY(QString, int, int)), m_MultSummaryThread, SLOT(ThreadSummary(QString, int, int)), Qt::QueuedConnection);

		m_MultDecodeThread[i]->SetMultIndex(i);
		m_MultDecodeThread[i]->moveToThread(QTH_MultDecodeThread[i]);
		QTH_MultDecodeThread[i]->start();
	}
	flag = QObject::connect(m_MultSummaryThread, SIGNAL(SUMMARYRESULTINCIRCLE(QStringList)), this, SLOT(SlotShowResult(QStringList)), Qt::QueuedConnection);
	flag = QObject::connect(m_MultSaveThread, SIGNAL(DELETETIF(QString)), m_MultDeleteTifThread, SLOT(evethingSearchResults(QString)), Qt::QueuedConnection);

	//设置检测队列
	while (g_iCameraTotal != g_ImgBuffer.size())
	{
		CCycleBuffer* v_uchar/* = new CCycleBuffer(2448 * 2048 * 3 * 5)*/;
		g_ImgBuffer.push_back(v_uchar);
	}
	return false;
}

void NewUI_Demo::SLOTShowImage(int pos, Mat img, int checktimes)
{
	QLabel* label = this->findChild<QLabel*>("LabelShow" + QString::number(pos) + "_" + QString::number(checktimes % g_PhotoTimes));
	int zz = label->frameWidth();
	QSize ss = label->size();
	ss.setWidth(ss.width() - zz * 2);
	ss.setHeight(ss.height() - zz * 2);
	Mat imgsend;
	if (img.channels() == 1)
	{
		cv::cvtColor(img, imgsend, COLOR_GRAY2BGR);
	}
	else if (img.channels() == 3)
	{
		cv::cvtColor(img, imgsend, COLOR_BGR2RGB);
	}
	cv::resize(imgsend, imgsend, Size(ss.width(), ss.height()));
	QImage disImage = QImage((const unsigned char*)(imgsend.data), imgsend.cols, imgsend.rows, imgsend.step, QImage::Format_RGB888);
	label->setPixmap(QPixmap::fromImage(disImage));
}

bool NewUI_Demo::OpenConnect()
{
#ifdef PLCCONNECT
	g_SocketPLC = new Socket_CPP();

	if (g_SocketPLC->initialization())
	{
		QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
		QString ipAddress = configIniRead.value("ProgramSetting/IpAddress", "192.168.1.1").toString();
		int port = configIniRead.value("ProgramSetting/Port", 0).toInt();
		bool b = connect(this, SIGNAL(SHOWEVERYPLCCONNECT(DataToPC_typ)), this, SLOT(showPLCValue(DataToPC_typ)));
		g_SocketPLC->set_message_handler(&_handlers, this);

		if (!g_SocketPLC->connectServer(ipAddress.toStdString().c_str(), port))
		{
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("PLC通信错误！多次尝试，无法与PLC建立连接!"), 2000);
			levelOut->show();
		}

	}
#endif // PLCCONNECT

	return false;
}



bool NewUI_Demo::SlotShowResult(QStringList strlist)
{
	//"Good,Error1,Error2,Error1,Error0,..."
	QSettings Dir(m_IniResultPath, QSettings::IniFormat);//找到按下开始时创建的路径下的文件 没有就自动创建
	QSettings FinalDir(AppPath + "/result/AllResult.ini", QSettings::IniFormat);//所有结果文件

	QString total = QString::number(ui.tableWidget_Result->item(0, 2)->text().toInt() + strlist.size());
	ui.tableWidget_Result->item(0, 2)->setText(total);
	Dir.setValue("Result/Total", total);//写总数，没有key值也会创建一个
	FinalDir.setValue(timeForAllResult + "/Total", total);

	for (int i = 0; i < strlist.size(); i++)
	{
		int z = 1;
		for (; z < ui.tableWidget_Result->rowCount(); z++)//最开始count==2，总数和合格数
		{
			if (ui.tableWidget_Result->item(z, 0)->text() == strlist[i])
			{
				int xx = ui.tableWidget_Result->item(z, 2)->text().toInt();
				ui.tableWidget_Result->item(z, 2)->setText(QString::number(ui.tableWidget_Result->item(z, 2)->text().toInt() + 1));
				Dir.setValue("Result/" + strlist[i], QString::number(ui.tableWidget_Result->item(z, 2)->text().toInt()));
				FinalDir.setValue(QString(timeForAllResult + "/" + strlist[i]), QString::number(ui.tableWidget_Result->item(z, 2)->text().toInt()));
				break;
			}
		}
		int xx = ui.tableWidget_Result->rowCount();
		if (z == ui.tableWidget_Result->rowCount())//遍历没有，增加新行赋值1
		{
			ui.tableWidget_Result->insertRow(z);
			ui.tableWidget_Result->setItem(z, 0, new QTableWidgetItem(strlist[i]));
			ui.tableWidget_Result->setItem(z, 1, new QTableWidgetItem(strlist[i]));
			ui.tableWidget_Result->setItem(z, 2, new QTableWidgetItem(QString::number(1)));
			Dir.setValue("Result/" + strlist[i], QString::number(ui.tableWidget_Result->item(z, 2)->text().toInt()));
			FinalDir.setValue(timeForAllResult + "/" + strlist[i], QString::number(ui.tableWidget_Result->item(z, 2)->text().toInt()));

		}
	}
	return false;
}

void NewUI_Demo::showPLCValue(DataToPC_typ data)
{
	memcpy(m_data, &data, sizeof(DataToPC_typ));//主界面用

	//daily_logger->error("{},{},{}",data.Telegram_typ, data.Status.HomeOK, data.Status.AlarmStatus);

	//if (m_showSwitch <10|| g_productionNumber=="")
	//{
	quint64 freeSpace = getDiskFreeSpace(QString("D:/"));
	QString strFreeSpace = "(" + g_QSUserName.mid(5) + ")" + QString::fromLocal8Bit(" 存图空间剩余") + QString::number(freeSpace) + "GB";//输出磁盘剩余空间大小	
	if (freeSpace <= 1)
	{
		ui.lE_freeSpace->setStyleSheet("color: rgb(255,0,0);font-size:14pt");
		ui.lE_freeSpace->setText("(" + g_QSUserName.mid(5) + ")" + QString::fromLocal8Bit(" 存图空间不足,开始循环存图"));
		g_saveLoopFlag = 1;//Everything delete flag
	}
	else
	{
		ui.lE_freeSpace->setStyleSheet("font-size:14pt");
		ui.lE_freeSpace->setText(strFreeSpace);
	}

	//}
	//else 
	//{
	//	ui.lE_freeSpace->setStyleSheet("font-size:14pt");
	//	ui.lE_freeSpace->setText(QString::fromLocal8Bit("当前生产批号 ")+g_productionNumber);
	//}

	//m_showSwitch++;
	//if (m_showSwitch == 20){m_showSwitch = 0; }

	QString lb_Alarm_str = "";
	int lb_AlarmFlag = 0;

	if (data.Telegram_typ == 103)
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("严重错误！未连接PLC，请重试！"), 2000);
		levelOut->show();
		delete g_SocketPLC;
		g_SocketPLC = nullptr;
		return;
	}
	if (data.Telegram_typ == 102)
	{
		g_SocketPLC->InitWork();
		return;
	}

	if (data.Telegram_typ == 0)
	{
		return;
	}

	static bool homeOkLogFlag = 1;
	static bool initOKequal1 = 0;//开机有一次homeok出现
	if (data.Status.HomeOK == 1)//int
	{

		if (initOKequal1 == 0)//开机有一次homeok出现
		{
			return;
		}
		if (g_IUserLevel != 3&& !m_bWaitForPLCStop)
		{
			ui.Button_Start->setEnabled(true);
		}
		if (homeOkLogFlag)
		{
			SYS_logger->info("设备寻参成功");
		}
		homeOkLogFlag = 0;
	}
	else if (data.Status.HomeOK == 0)
	{
		initOKequal1 = 1;//开机有一次homeok出现
		homeOkLogFlag = 1;
		ui.Button_Start->setEnabled(false);
	}

	static int alarmLogFlag = 0;
	if (data.Status.AlarmStatus == 0)//int
	{
		alarmLogFlag = 1;
	}
	else if (data.Status.AlarmStatus == 1)
	{
		if (alarmLogFlag == 1 || alarmLogFlag == 10)
		{
			ALM_logger->warn("设备警告");
			lb_Alarm_str = QString::fromLocal8Bit("设备警告！");
			lb_AlarmFlag = 1;
		}
		alarmLogFlag = 9;
	}
	else if (data.Status.AlarmStatus == 2)
	{
		if (alarmLogFlag == 1 || alarmLogFlag == 9)
		{
			ALM_logger->error("设备故障！");
			lb_Alarm_str = QString::fromLocal8Bit("设备故障！");
			lb_AlarmFlag = 1;
		}
		alarmLogFlag = 10;
	}



	static bool EstopLogFlag = 1;
	if (!data.Inputs.EStop)//bool
	{
		lb_Alarm_str = QString::fromLocal8Bit("急停按下！");
		lb_AlarmFlag = 1;
		if (EstopLogFlag)
		{
			OPS_logger->info("急停按下！");
			ALM_logger->error("急停按下！");
		}
		EstopLogFlag = 0;
	}
	else
	{
		EstopLogFlag = 1;
	}

	if (lb_AlarmFlag == 1)
	{
		ui.lb_Alarm->m_showText = lb_Alarm_str;
		ui.lb_Alarm->setStyleSheet("color: rgb(255,0,0);font-size:20pt");
	}
	else
	{
		ui.lb_Alarm->m_showText = QString::fromLocal8Bit("设备已就绪");
		ui.lb_Alarm->setStyleSheet("color: rgb(0,255,0);font-size:20pt");
	}

	//运行数据
	if (b_changingSpeed == 0)
	{
		ui.lE_RunSpeed->setText(QString::number(data.ActData.RunSpeed));
	}
	ui.lE_CheckCount->setText(QString::number(data.ActData.CheckCount));
	ui.lE_RejectCount->setText(QString::number(data.ActData.RejectCount));
	if (data.ActData.ForceRejectCount-ui.lE_ForceRejectCount->text().toInt()>0)
	{
		ALM_logger->critical("强制踢出增加");
	}
	ui.lE_ForceRejectCount->setText(QString::number(data.ActData.ForceRejectCount));

	if (m_data->ActData.SysPhase == 0 && m_bWaitForPLCStop)
	{
		for (int i = 0; i < g_vectorCamera.size(); i++)
		{
#ifdef BASLER
			g_vectorCamera[i]->cb_Camera.StopGrabbing();
#endif
#ifdef DAHENG

			//发送停采命令
			CGXFeatureControlPointer m_objFeatureControlPtr = g_vectorCamera[i]->m_objDevicePtr->GetRemoteFeatureControl();
			m_objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();

			//关闭流层采集
			g_vectorCamera[i]->m_objStreamPtr->StopGrab();

			//注销采集回调函数
			g_vectorCamera[i]->m_objStreamPtr->UnregisterCaptureCallback();
#endif
		}

		ui.Button_Start->setText(QString::fromLocal8Bit("开始"));
		ui.Button_Start->setStyleSheet("font-size:20pt");

		if (g_IUserLevel == 0)
		{
			ui.Button_Home->setEnabled(true);
			ui.Button_AlarmReset->setEnabled(true);
			ui.Button_CameraSet->setEnabled(true);
			ui.Button_Log->setEnabled(true);
			ui.Button_DataOut->setEnabled(true);
			ui.Button_CountReset->setEnabled(true);
			ui.Button_Exit->setEnabled(true);
		}
		else if (g_IUserLevel == 1)
		{
			ui.Button_Home->setEnabled(true);
			ui.Button_AlarmReset->setEnabled(true);
			ui.Button_CameraSet->setEnabled(true);
			ui.Button_Log->setEnabled(true);
			ui.Button_CountReset->setEnabled(true);
			ui.Button_Exit->setEnabled(true);
		}
		else if (g_IUserLevel == 2)//权限按钮
		{
			ui.Button_Home->setEnabled(true);
			ui.Button_AlarmReset->setEnabled(true);
			ui.Button_Log->setEnabled(true);
			ui.Button_CountReset->setEnabled(true);
			ui.Button_Exit->setEnabled(true);
		}
		m_bWaitForPLCStop = false;
		SYS_logger->info("系统停止运行！");
	}
}

void NewUI_Demo::onTest()
{
}


void NewUI_Demo::onHome()
{
	OPS_logger->info("寻零按下");
	DataFromPC_typ typ;
	typ = getPCData();
	typ.Dummy = 0;//占空
	typ.Telegram_typ = 1;//命令报文
	typ.Machine_Cmd.cmdHome = 1;					//报警复位, 1:复位
	g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收

}

void NewUI_Demo::onStartCheck(bool b)
{
	if (b)
	{
		g_productionNumber=ui.lE_PN->text();
		ui.lE_PN->setEnabled(false);

		if (g_productionNumber!="")
		{
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("开始运行—批号：")+g_productionNumber, 2000);
			levelOut->show();
		}


		OPS_logger->info("开始按下");
		SYS_logger->info("系统开始运行！");

		ui.Button_Home->setEnabled(false);
		ui.Button_AlarmReset->setEnabled(false);
		ui.Button_CameraSet->setEnabled(false);
		ui.Button_Log->setEnabled(false);
		ui.Button_DataOut->setEnabled(false);
		ui.Button_CountReset->setEnabled(false);
		ui.Button_Exit->setEnabled(false);
		if (m_MultInit != nullptr)
		{
			delete m_MultInit;
			m_MultInit = nullptr;
		}

		if (QTH_Init != nullptr)
		{
			QTH_Init->quit();
			QTH_Init->wait();//释放对象锁
			delete QTH_Init;
			QTH_Init = nullptr;
		}

		QThread::msleep(200);
		SYS_logger->info("Start Checking! Connect Camera {}", g_iCameraTotal);
		//判断有无相机加载
		if (g_iCameraTotal == 0)
		{
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("警告！未初始化本地文件！"), 2000);
			levelOut->show();
			return;
		}
		ui.Button_Start->setText(QString::fromLocal8Bit("停止"));
		ui.Button_Start->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		g_current_time = QDateTime::currentDateTime();
		memset(m_Result, 0, 30 * sizeof(char));
		emit STARTCHECK(-1, 0);
		QThread::msleep(500);

		current_time = QDateTime::currentDateTime();
		QString resultYear = QString::number(current_time.date().year());
		resultYear = resultYear.length() < 4 ? ("0" + resultYear) : resultYear;
		QString resultMonth = QString::number(current_time.date().month());
		resultMonth = resultMonth.length() < 2 ? ("0" + resultMonth) : resultMonth;
		QString resultDay = QString::number(current_time.date().day());
		resultDay = resultDay.length() < 2 ? ("0" + resultDay) : resultDay;
		QString resultHour = QString::number(current_time.time().hour());
		resultHour = resultHour.length() < 2 ? ("0" + resultHour) : resultHour;
		QString resultMinute = QString::number(current_time.time().minute());
		resultMinute = resultMinute.length() < 2 ? ("0" + resultMinute) : resultMinute;

		if (g_productionNumber != "")
		{
			timeForAllResult = "Result_"
				+ resultYear + "_" //z=a>b?x:y
				+ resultMonth + "_"
				+ resultDay + "_"
				+ resultHour + "_"
				+ resultMinute + "_" + g_productionNumber;//eg:MainDLG_daily_2020_02_29_23_47.txt
		}
		else
		{
			timeForAllResult = "Result_"
				+ resultYear + "_" //z=a>b?x:y
				+ resultMonth + "_"
				+ resultDay + "_"
				+ resultHour + "_"
				+ resultMinute;//eg:MainDLG_daily_2020_02_29_23_47.txt
		}
		m_IniResultPath = AppPath + "/result/"//结果文件路径,每次开始的时候创建一个
			+ timeForAllResult + ".ini";//eg:Result_2020_02_29_23_47_1234567890abc.ini
		
	


#ifdef PLCCONNECT
		if (g_SocketPLC != nullptr)
		{
			g_SocketPLC->StartWork();
		}
#endif
	}
	else
	{

		ui.lE_PN->setEnabled(true);
		OPS_logger->info("停止按下！");
		onStopCheck();

	}
	return;
}

void NewUI_Demo::onStopCheck()
{
#ifdef PLCCONNECT
	if (g_SocketPLC != nullptr)
	{
		g_SocketPLC->StopWork();
	}
#endif

	ui.Button_Start->setEnabled(false);
	//3s以后timer执行下面一段
	//if Connect Basler camera just stop grabbing 
	m_bWaitForPLCStop = true;
	
}

void NewUI_Demo::closeEvent(QCloseEvent* eve)
{
}

void NewUI_Demo::onTestSignal()
{
}

void NewUI_Demo::onCameraSet()
{
	OPS_logger->info("参数设置按下");
	//相机参数设置
	QtCameraSet *m_DlgCameraSet = new QtCameraSet;
	m_DlgCameraSet->exec();
	for (int i = 0; i < g_CheckClass.size(); i++)
	{
		g_CheckClass[i]->SetShowCallBack(this, ShowCallBack);	//与QtGUISetting的line44 对应，需要将回调函数注册回当前窗体 
	}
	g_SocketPLC->set_message_handler(&_handlers, this);
}

int NewUI_Demo::showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str)//全是中文
{
	if (QString::fromLocal8Bit(button2Str) == "")
	{
		QMessageBox msg(QMessageBox::NoIcon, QString::fromLocal8Bit(titleStr), QString::fromLocal8Bit(contentStr), QMessageBox::Ok);
		msg.setWindowFlags(Qt::FramelessWindowHint);
		msg.setStyleSheet(
			"QPushButton {"
			"background-color:#f0f0f0;"
			"color:#00aa7f;"
			//" border-style: inherit;"
			//" border-width: 2px;"
			//" border-radius: 10px;"
			//" border-color: beige;"
			" font: bold 24px;"
			" min-width: 6em;"
			" min-height: 3em;"
			"}"
			"QLabel { min-width: 20em;min-height:3em;font:24px;background-color:#f0f0f0;}"
		);
		msg.setGeometry((768 - 523) / 2, 320, msg.width(), msg.height());
		//圆角👇
		QBitmap bmp(523, 185);
		bmp.fill();
		QPainter p(&bmp);
		p.setPen(Qt::NoPen);
		p.setBrush(Qt::black);
		p.drawRoundedRect(bmp.rect(), 5, 5);
		msg.setMask(bmp);

		msg.setButtonText(QMessageBox::Ok, QString::fromLocal8Bit(button1Str));
		msg.setWindowIcon(QIcon("./ico/dr.ico"));
		return msg.exec();
	}
	else
	{
		QMessageBox msg(QMessageBox::NoIcon, QString::fromLocal8Bit(titleStr), QString::fromLocal8Bit(contentStr), QMessageBox::Yes | QMessageBox::No);
		msg.setWindowFlags(Qt::FramelessWindowHint);
		msg.setStyleSheet(
			"QPushButton {"
			"background-color:#f0f0f0;"
			"color:#00aa7f;"
			//" border-style: inherit;"
			//" border-width: 2px;"
			//" border-radius: 10px;"
			//" border-color: beige;"
			" font: bold 24px;"
			" min-width: 6em;"
			" min-height: 3em;"

			"}"
			"QLabel { min-width: 20em;min-height:3em;font:24px;background-color:#f0f0f0;}"
		);
		msg.setGeometry((768 - 523) / 2, 320, msg.width(), msg.height());
		//圆角👇
		QBitmap bmp(523, 185);
		bmp.fill();
		QPainter p(&bmp);
		p.setPen(Qt::NoPen);
		p.setBrush(Qt::black);
		p.drawRoundedRect(bmp.rect(), 5, 5);
		msg.setMask(bmp);

		msg.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit(button1Str));
		msg.setButtonText(QMessageBox::No, QString::fromLocal8Bit(button2Str));
		msg.setWindowIcon(QIcon("./ico/dr.ico"));
		return msg.exec();
	}

	//  QMessageBox::NoIcon
	//	QMessageBox::Question
	//	QMessageBox::Information
	//	QMessageBox::Warning
	//	QMessageBox::Critical
}

void NewUI_Demo::splitpixmap(QPixmap& pixmap, int xnum, int ynum)//切图
{

	for (int x = 0; x < xnum; ++x) {
		for (int y = 0; y < ynum; ++y) {
			m_pixlist << pixmap.copy(x * (pixmap.width() / xnum), y * (pixmap.height() / ynum),
				pixmap.width() / xnum - 9, pixmap.height() / ynum - 9);
		}
	}
}

void NewUI_Demo::AlarmResetCommand()
{
	OPS_logger->info("故障复位按下");
	DataFromPC_typ typ;
	typ = getPCData();
	typ.Dummy = 0;//占空
	typ.Telegram_typ = 1;//命令报文
	typ.Machine_Cmd.cmdErrorAck = 1;					//报警复位, 1:复位
	g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收
}

void NewUI_Demo::on_Button_CountReset_clicked()
{
	OPS_logger->info("数据清零按下");
	DataFromPC_typ typ;
	typ = getPCData();
	typ.Dummy = 0;//占空
	typ.Telegram_typ = 1;//命令报文
	typ.Machine_Cmd.cmdResetCounter = 1;
	g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收

	for (int i = 0; i < ui.tableWidget_Result->rowCount(); i++)//遍历没有，增加新行赋值1
	{
		ui.tableWidget_Result->setItem(i, 2, new QTableWidgetItem(QString::number(0)));
	}
	SYS_logger->info("数据已清零");
}

void NewUI_Demo::changeRunSpeed()
{
	ui.lE_RunSpeed->clearFocus();
	ui.cB_RunSpeed->showPopup();
}

void NewUI_Demo::SetEvertDlg()
{
	for (int i = 0; i < g_iCameraTotal; i++)//此时g_iCameraTotal== g_vectorCamera.size()
	{
		for (int ii = 0; ii < g_PhotoTimes; ii++)
		{
			QLabel *lb = this->findChild<QLabel *>(QString::fromUtf8("LabelShow") + QString::number(i) + "_" + QString::number(ii));
			if (lb != nullptr)
			{
				g_CheckClass[i]->TESTSETSHOW(lb);
			}
		}

		// 		//g_CheckClass[i]->StartCheck("", daily_logger);
		// 	
		// 			m_MultDecodeThread[i]->ThreadDecodeImage(i, false);
	}
	if (timerResize != nullptr)
	{
		timerResize->stop();
		delete timerResize;
		timerResize = nullptr;
	}
}

//获取pc数据
DataFromPC_typ NewUI_Demo::getPCData()
{
	DataFromPC_typ tmp;
	memset(&tmp, 0, sizeof(DataFromPC_typ));//将新char所指向的前size字节的内存单元用一个0替换，初始化内存。下同

	tmp.Dummy = 0;
	//tmp.Telegram_typ = m_data->Telegram_typ;

	//系统参数
	tmp.Machine_Para.FeedAxisHomeOffset = m_data->Machine_Para.FeedAxisHomeOffset;//qstring-float-qstring-int
	tmp.Machine_Para.ClipPhase1 = m_data->Machine_Para.ClipPhase1;
	tmp.Machine_Para.ClipPhase2 = m_data->Machine_Para.ClipPhase2;
	tmp.Machine_Para.UpPhase1 = m_data->Machine_Para.UpPhase1;
	tmp.Machine_Para.UpPhase2 = m_data->Machine_Para.UpPhase2;
	tmp.Machine_Para.DropPhase1 = m_data->Machine_Para.DropPhase1;
	tmp.Machine_Para.DropPhase2 = m_data->Machine_Para.DropPhase2;
	tmp.Machine_Para.tClip1 = m_data->Machine_Para.tClip1;
	tmp.Machine_Para.tClip2 = m_data->Machine_Para.tClip2;
	tmp.Machine_Para.tUp1 = m_data->Machine_Para.tUp1;
	tmp.Machine_Para.tUp2 = m_data->Machine_Para.tUp2;
	tmp.Machine_Para.tDrop1 = m_data->Machine_Para.tDrop1;
	tmp.Machine_Para.tDrop2 = m_data->Machine_Para.tDrop2;
	tmp.Machine_Para.FeedLength = m_data->Machine_Para.FeedLength;
	tmp.Machine_Para.FlashTime = m_data->Machine_Para.FlashTime;
	tmp.Machine_Para.PhotoTime = m_data->Machine_Para.PhotoTime;
	tmp.Machine_Para.RejectTime = m_data->Machine_Para.RejectTime;
	tmp.Machine_Para.PhotoDelay = m_data->Machine_Para.PhotoDelay;
	tmp.Machine_Para.PhotoPhase = m_data->Machine_Para.PhotoPhase;
	tmp.Machine_Para.RejectPhase = m_data->Machine_Para.RejectPhase;
	tmp.Machine_Para.PhotoTimes = m_data->Machine_Para.PhotoTimes;
	tmp.Machine_Para.RotateSpeed = m_data->Machine_Para.RotateSpeed;
	tmp.Machine_Para.DisableForceReject = m_data->Machine_Para.DisableForceReject;		//关闭强制剔废,1:关闭
	tmp.Machine_Para.CapCheckAlarmTime = m_data->Machine_Para.CapCheckAlarmTime;		//胶囊检测报警时间，单位ms
	tmp.Machine_Para.RejectFallingTime = m_data->Machine_Para.RejectFallingTime;		//剔废胶囊下落时间，单位ms

																						//运行数据
	tmp.Run_Para.RunSpeed = m_data->ActData.RunSpeed;
	tmp.Run_Para.SysPhase = m_data->ActData.SysPhase;
	tmp.Run_Para.enPhoto = m_data->ActData.enPhoto;
	tmp.Run_Para.enReject = m_data->ActData.enReject;
	tmp.Run_Para.enFeed = m_data->ActData.enFeed;
	tmp.Run_Para.enRotate = m_data->ActData.enRotate;
	tmp.Run_Para.CheckCount = m_data->ActData.CheckCount;
	tmp.Run_Para.RejectCount = m_data->ActData.RejectCount;
	tmp.Run_Para.ForceRejectCount = m_data->ActData.ForceRejectCount;


	//照相处理结果和按钮在上面⬆⬆⬆⬆⬆⬆

	return tmp;
}

void NewUI_Demo::initPictureListWidget()//第一次列表显示项
{
	ui.lW_Picture->clear();
	if (g_iCameraTotal > 0)//此处加上，cameraset的部分就不用了
	{

		ui.lW_Picture->addItem(new QListWidgetItem(QString::fromLocal8Bit("取消")));
		if (m_pictureHidden == 0)
		{
			ui.lW_Picture->item(0)->setHidden(0);
		}
		else
		{
			ui.lW_Picture->item(0)->setHidden(1);
		}
		for (int i = 0; i < g_vectorCamera.size(); i++)
		{
			QString str_Name = g_vectorCamera[i]->c_CameraName;
			QString str_FullName = QString::number(i) + "_" + str_Name;
			ui.lW_Picture->addItem(new QListWidgetItem(str_FullName + "_OK"));
			ui.lW_Picture->addItem(new QListWidgetItem(str_FullName + "_NG"));
		}


	}
}


void NewUI_Demo::changeListWidgetItem(QListWidgetItem* item)//双击事件
{
	QString strPath = item->text();
	if (strPath.mid(1, 1) == "_")//弹出图片列表
	{
		QString path = saveImagePath + "/" + strPath.mid(2, 8) + "/" + strPath.mid(11);
		m_selectedPath = path;
		InitShowImgList(m_selectedPath);
	}
	else if (strPath == QString::fromLocal8Bit("刷新"))
	{
		InitShowImgList(m_selectedPath);
	}
	else if (strPath == QString::fromLocal8Bit("取消"))
	{
		ui.lb_Picture->setVisible(false);
		item->setHidden(true);
		m_pictureHidden = 1;
	}

	else if (strPath == QString::fromLocal8Bit("上一级"))
	{
		initPictureListWidget();
	}
	else
	{
		QImage img;
		strPath = m_selectedPath + "/" + strPath;
		if (!img.load(strPath))
		{
			ui.lb_Picture->setVisible(false);
			QMessageBox::information(this,
				QStringLiteral("打开图像失败"),
				QStringLiteral("打开图像失败!"));
			return;
		}

		ui.lb_Picture->setVisible(true);//图片显示
		QSize ss = ui.lb_Picture->size();
		img.scaled(ss, Qt::IgnoreAspectRatio);
		ui.lb_Picture->setPixmap(QPixmap::fromImage(img));

		ui.lW_Picture->item(0)->setHidden(false);//上面有图片的时候显示取消
		m_pictureHidden = 0;
	}

}

bool NewUI_Demo::InitShowImgList(QString _dir)//显示图片页
{
	m_pictureLevel = 2;//暂时没什么用
	ui.lW_Picture->clear();

	ui.lW_Picture->addItem(new QListWidgetItem(QString::fromLocal8Bit("取消")));
	if (m_pictureHidden == 0)
	{
		ui.lW_Picture->item(0)->setHidden(false);
	}
	else
	{
		ui.lW_Picture->item(0)->setHidden(true);
	}
	ui.lW_Picture->addItem(new QListWidgetItem(QString::fromLocal8Bit("上一级")));
	ui.lW_Picture->addItem(new QListWidgetItem(QString::fromLocal8Bit("刷新")));
	//判断路径是否存在
	QDir dir(_dir);

	if (!dir.exists())
	{
		return false;
	}

	//获取所选文件类型过滤器

	QStringList filters;
	filters << QString("*.jpeg") << QString("*.jpg") << QString("*.png") << QString("*.tif") << QString("*.tiff") << QString("*.gif") << QString("*.bmp");

	//定义迭代器并设置过滤器
	QDirIterator dir_iterator(_dir, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);


	QStringList pictureList;
	while (dir_iterator.hasNext())

	{

		dir_iterator.next();

		QFileInfo file_info = dir_iterator.fileInfo();

		QString absolute_file_path = file_info.fileName();
		pictureList.append(absolute_file_path);
		ui.lW_Picture->addItem(new QListWidgetItem(absolute_file_path));

	}

	return true;
}

quint64 NewUI_Demo::getDiskFreeSpace(QString driver)//D盘剩余空间
{
	LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();
	ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;
	if (!GetDiskFreeSpaceEx(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes))
	{
		return 0;
	}
	return (quint64)liTotalFreeBytes.QuadPart / 1024 / 1024 / 1024;
}


void NewUI_Demo::SWITCHOSK()//快捷键
{
	keybd_event(0x11, 0, 0, 0);
	keybd_event(0x5B, 0, 0, 0);
	keybd_event(0x4F, 0, 0, 0);
	keybd_event(0x11, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x5B, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x4F, 0, KEYEVENTF_KEYUP, 0);//ctrl+win+o切换
}

void NewUI_Demo::PNChanged()
{
	SWITCHOSK();
	ui.lE_PN->clearFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//下面是232串口部分相关
void NewUI_Demo::adjustBrightness()//亮度调节
{

	onConnectPort();

	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
	int light1=configIniRead.value("ProgramSetting/Light1", 50).toInt();
	int light2=configIniRead.value("ProgramSetting/Light2", 50).toInt();
	int light3=configIniRead.value("ProgramSetting/Light3", 50).toInt();

	if (m_serialPort->isOpen())
	{

		connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receiveInfo()));

		m_write++;
		QByteArray sendBuf;
		sendBuf.resize(7);

		sendBuf[0] = 0x53;
		sendBuf[1] = 0x4c;
		sendBuf[2] = 0xaa;
		sendBuf[3] = 0x03;
		sendBuf[4] = 0x03;
		if (m_write == 1)
		{
			sendBuf[5] = 1;
			sendBuf[6] = light1;
			m_serialPort->write(sendBuf);
			OPS_logger->info("232write LED1");
		}
	}

}


void NewUI_Demo::onConnectPort()
{
	if (m_serialPort == nullptr)
	{
		m_serialPort = new QSerialPort();//新串口
	}

	if (g_232Port > 0)
	{

		if (nullptr != m_serialPort)
		{
			if (m_serialPort->isOpen())//��������Ѿ����� �ȸ����ر���
			{
				m_serialPort->clear();
				m_serialPort->close();
			}

			//���ô������� �������������Ѿ��ɹ���ȡ���� ����ʹ�õ�һ��
			QString qport = "COM";
			qport += QString::number(g_232Port);
			m_serialPort->setPortName(qport);

			OPS_logger->warn("打开232串口：{}", qport.toStdString().c_str());

			if (!m_serialPort->open(QIODevice::ReadWrite))
			{
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("串口打开失败！"), 2000);
				levelOut->show();
				return;
			}
			m_serialPort->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
			m_serialPort->setDataBits(QSerialPort::Data8);
			m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
			m_serialPort->setParity(QSerialPort::NoParity);
			m_serialPort->setStopBits(QSerialPort::OneStop);

		}
	}
	return;
}


void NewUI_Demo::receiveInfo()//read 232
{
	QByteArray info = m_serialPort->readAll();
	//QByteArray hexData = info.toHex();
	char* s = info.data();

	OPS_logger->warn("232read {}", s);
	onCircleWrite();
}

void NewUI_Demo::onCircleWrite()
{

	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
	int light1 = configIniRead.value("ProgramSetting/Light1", 50).toInt();
	int light2 = configIniRead.value("ProgramSetting/Light2", 50).toInt();
	int light3 = configIniRead.value("ProgramSetting/Light3", 50).toInt();

	m_write++;
	QByteArray sendBuf;
	sendBuf.resize(7);

	sendBuf[0] = 0x53;
	sendBuf[1] = 0x4c;
	sendBuf[2] = 0xaa;
	sendBuf[3] = 0x03;
	sendBuf[4] = 0x03;

	if (m_write == 2)
	{
		sendBuf[5] = 2;
		sendBuf[6] = light2;
		m_serialPort->write(sendBuf);
		OPS_logger->info("232write LED2");
		return;
	}
	else if (m_write == 3)
	{
		sendBuf[5] = 3;
		sendBuf[6] = light3;
		m_serialPort->write(sendBuf);
		OPS_logger->info("232write LED3");
		return;
	}
	else if (m_write == 4)
	{

		disconnect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receiveInfo()));
		m_write = 0;

		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("初始亮度设置成功！"), 2000);
		levelOut->show();

		if (m_serialPort->isOpen())//��������Ѿ����� �ȸ����ر���
		{
			m_serialPort->clear();
			m_serialPort->close();
		}
	}

}
///////////////上面是串口的