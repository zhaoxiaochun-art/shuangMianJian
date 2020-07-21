#include "QtCameraSet.h"
#include <QPainter>
#include <QBitmap>
#include <QLabel>

extern QString AppPath;
extern QString style1;
extern QString style2;

extern std::shared_ptr<spd::logger> SYS_logger;
extern std::shared_ptr<spd::logger> OPS_logger;//操作
extern QVector<CAMERASTRUCT*> g_vectorCamera;
extern QVector<MultGetThread_Run*> m_MultGetThread;
extern QVector<CBaseCheckAlg*> g_CheckClass;
extern CBaseCheckAlg* EnsureAlg(QString str);
extern Socket_CPP* g_SocketPLC;
extern QString g_qModelName;//当前应用模板

extern QString g_QSUserName;
extern int g_IUserLevel;
extern int g_PhotoTimes;
extern int g_232Port;
extern QString g_productionNumber;
extern QString g_ipAddress;
extern int g_port;
//设置外触发Line1
extern void SetTriggerLine(int i, int line);
//设置软触发
extern void SetTriggerOff(int i);
#ifdef PLCCONNECT
void message_handler(void* context, DataToPC_typ data);
#endif

QtCameraSet::QtCameraSet(QWidget* parent)
	: QDialog(parent)

{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);//无边框   
	//setAttribute(Qt::WA_TranslucentBackground);//背景透明

	ui.lb_style2->raise();
	ui.lb_style2->setStyleSheet(style2);
	ui.lb_style2->setAttribute(Qt::WA_TransparentForMouseEvents);

	setWindowIcon(QIcon("./ico/dr.ico"));
	ui.lb_AppliedName->setText(g_qModelName);//更新lable 4-3

	if (g_IUserLevel == 1)//工程师移除一栏
	{
		ui.tabWidget->removeTab(3);
	}
	if (g_IUserLevel == 0)
	{
		initTableOfUserPermission();//用户处表格
	}

	initListWidgetofModel();//初始化model listWidget
	ui.listWidget->setGridSize(QSize(50, 40));//加大间距
	//should reconstruct the position
	//2020年2月24日10:30:39
	connect(ui.pB_StartGrab, SIGNAL(toggled(bool)), this, SLOT(StartGrab(bool)));//采集测试
	connect(ui.pB_StartGrab, SIGNAL(toggled(bool)), this, SLOT(SyncAnotherStartGrab(bool)));//采集测试
	connect(ui.pB_AlgSetting, SIGNAL(clicked()), this, SLOT(StopGrab()));//算法设置
	//connect(ui.pB_AlgSetting, SIGNAL(clicked()), this, SLOT(onShowAlgSet()));//同上

	ui.frame_Alg->setVisible(false);

	connect(ui.pB_StartContinueGrab, SIGNAL(toggled(bool)), this, SLOT(onContinueGrab(bool)));//相机调试
	connect(ui.cB_flash, SIGNAL(toggled(bool)), this, SLOT(slotCb_flash(bool)));

	//connect(ui.pB_StartStore, SIGNAL(toggled(bool)), this, SLOT(onStartStore(bool)));//开始存图
	connect(ui.cB_feed, SIGNAL(toggled(bool)), this, SLOT(slotCb_feed(bool)));
//	connect(ui.pb_StReadPLC, SIGNAL(toggled(bool)), this, SLOT(onReadPLCParam(bool)));//读取
	connect(ui.pb_StWritePLC, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));//写入
	for (int i = 0; i < 30; i++)
	{
		QString objname = "pb_cmdTestKick";
		objname = objname + QString::number(i);
		QPushButton* btn = findChild<QPushButton*>(objname); //返回this中一个名为objname的QPushButton孩子，即使按钮不是父亲的直接孩子：30个剔废按钮
		if (btn != nullptr)
		{
			connect(btn, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));//30个剔废按钮checked
		}

	}
	//connect(ui.pb_cmdHome, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));
	//connect(ui.pb_cmdStart, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));
	//connect(ui.pb_cmdStop, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));
	//connect(ui.pb_cmdEStop, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));
	connect(ui.pb_cmdJog, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));
//	connect(ui.pb_cmdErrorAck, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//报警复位, 1:复位
//	connect(ui.pb_cmdResetCounter, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//复位计数变量, 1:复位
	//connect(ui.pb_cmdParaSave, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//参数保存命令, 1:保存
	//connect(ui.pb_cmdParaLoad, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//参数读取命令, 1:读取
	connect(ui.pb_cmdTestFlash0, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));			//手动闪光, 1:闪光,自动复位
	connect(ui.pb_cmdTestFlash1, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));
	connect(ui.pb_cmdTestFlash2, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));

	connect(ui.pb_cmdTestValveUp, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//手动升降气缸, 1:Push, 2:Back
	connect(ui.pb_cmdTestValveClip, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动夹紧气缸, 1:Push, 2:Back
	connect(ui.pb_cmdTestValveDrop, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动落囊气缸, 1:Push, 2:Back
	connect(ui.pb_cmdTestInverter, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动胶囊料斗启动, 1:Start, 2:Stop
	connect(ui.pb_cmdTestLampRead, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动红灯输出, 1:输出 , 2: 复位
	connect(ui.pb_cmdTestLampYellow, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动黄灯输出, 1:输出 , 2: 复位
	connect(ui.pb_cmdTestLampGreen, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动绿灯输出, 1:输出 , 2: 复位
	connect(ui.pb_cmdTestBuzzer, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//手动蜂鸣器输出, 1:输出 , 2: 复位
	connect(ui.pb_cmdTestPhoto, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//手动拍照, 1:输出 , 2: 复位
	connect(ui.pb_cmdTestFlashPhoto, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动闪光加拍照, 1:启动
	connect(ui.pb_cmdTestCapPhoto, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//手动胶囊拍照
	connect(ui.pb_cmdRotateCtl, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//手动转囊启停  报文是1

	connect(ui.pB_enPhoto, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));					//拍照使能
	connect(ui.pB_enReject, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//剔废使能  报文是4
	connect(ui.pB_enFeed, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//料斗使能  报文是4
	connect(ui.pB_enRotate, SIGNAL(toggled(bool)), this, SLOT(onSendPLCCommand(bool)));						//转囊使能  报文是4

	initContinueKick();//连剔开关、数量初始化

	m_data = new DataToPC_typ;
	memset(m_data, 0, sizeof(DataToPC_typ));//主界面用
	//指示灯部分

	ui.lb_00->setPixmap(QPixmap("./ico/redLed.png"));
	ui.lb_01->setPixmap(QPixmap("./ico/redGreen.png"));
	ui.lb_10->setPixmap(QPixmap("./ico/redLed.png"));
	ui.lb_11->setPixmap(QPixmap("./ico/redGreen.png"));
	ui.lb_20->setPixmap(QPixmap("./ico/redLed.png"));
	ui.lb_21->setPixmap(QPixmap("./ico/redGreen.png"));
	ui.lb_30->setPixmap(QPixmap("./ico/redLed.png"));
	ui.lb_31->setPixmap(QPixmap("./ico/redGreen.png"));
	ui.lb_40->setPixmap(QPixmap("./ico/redLed.png"));
	ui.lb_41->setPixmap(QPixmap("./ico/redGreen.png"));
	//指示灯部分


	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
	g_ipAddress = configIniRead.value("ProgramSetting/IpAddress", "192.168.1.1").toString();
	g_port = configIniRead.value("ProgramSetting/Port", 0).toInt();
	ui.lE_IP->setText(g_ipAddress);
	ui.lE_Port->setText(QString::number(g_port));
	connect(ui.lE_IP, &QLineEdit::editingFinished, [=]() {
		if (g_ipAddress!= ui.lE_IP->text())
		{
			QSettings Dir(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//找到文件
			Dir.setValue("ProgramSetting/IpAddress", ui.lE_IP->text());//写当前模板
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，IP地址设置成功，重新登入系统后生效！"), 2000);
			levelOut->show();
			g_ipAddress = ui.lE_IP->text();
		}
		
		});
	connect(ui.lE_Port, &QLineEdit::editingFinished, [=]() {
		if (g_port != ui.lE_Port->text().toInt())
		{
			QSettings Dir(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//找到文件
			Dir.setValue("ProgramSetting/Port", ui.lE_Port->text());//写当前模板
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，通讯端口设置成功，重新登入系统后生效！"), 2000);
			levelOut->show();
			g_port = ui.lE_Port->text().toInt();
		}
		});

	//model部分的信号与槽↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

	connect(ui.pB_Model_Delete, SIGNAL(clicked()), this, SLOT(modelDelete()));
	connect(ui.pB_Model_ChangeName, &QPushButton::clicked, [=]() {
		modelID = 1;
		modelChangeName();
		});
	connect(ui.pB_Model_Add, &QPushButton::clicked, [=]() {
		modelID = 2;
		modelAdd();
		});
	connect(ui.pB_Model_Apply, SIGNAL(clicked()), this, SLOT(modelApply()));//clicked();
	connect(ui.pB_Model_Exit1, &QPushButton::clicked, [=]() {
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "退出确认", "<img src = './ico/question.png'/>\t确认退出系统设置页面吗？", "确认", "取消"))
		{
			close();
		}});
	connect(ui.pB_Model_Exit2, &QPushButton::clicked, [=]() {
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "退出确认", "<img src = './ico/question.png'/>\t确认退出系统设置页面吗？", "确认", "取消"))
		{
			close();
		}});
	connect(ui.pB_Model_Exit3, &QPushButton::clicked, [=]() {
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "退出确认", "<img src = './ico/question.png'/>\t确认退出系统设置页面吗？", "确认", "取消"))
		{
			close();
		}});	

	ui.cB_photoTimes->setCurrentIndex(g_PhotoTimes - 1);
	connect(ui.cB_photoTimes, QOverload<const QString&>::of(&QComboBox::activated),
		[=](const QString& text) {
			QSettings Dir(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//找到文件
			Dir.setValue("ProgramSetting/PhotoTimes", text);//写当前模板
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，拍照次数设置成功，重新登入系统后生效！"), 2000);
			levelOut->show(); 
			savePLCParaInPLCTxt();
		});

	//model部分的信号与槽↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
	//键盘部分信号与槽↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
	//两个键盘👇👇👇👇
	//1st
	QList<QAbstractButton*> buttonsList = ui.buttonGroup_Keyboard->buttons();
	if (buttonsList.isEmpty()) {
		qDebug() << "isEmpty";
		return;
	}
	foreach(QAbstractButton * button, buttonsList)
	{
		((QPushButton*)button)->move(670, 10);//前12个按钮
		//((QPushButton*)button)->FocusManager.IsFocusScope = "False";//不获取焦点
		((QPushButton*)button)->setFocusPolicy(Qt::NoFocus);
		//不接受焦点针对某些控件-用户鼠标和键盘不能对控件进行任何操作，对于pushbutton不存在
// 		ui->lineEdit->setFocusPolicy(Qt::NoFocus);
// 
// 		Qt::TabFocus             //可以通过通过Tab键接受焦点
// 			Qt::ClickFocus           //可以通过单击接受焦点
// 			Qt::StrongFocus          //TabFocus | ClickFocus | 0x8
// 			Qt::WheelFocus           //可以通过使用鼠标滚轮接受焦点
// 			Qt::NoFocus              //不接受焦点。

	}
	ui.pushButton_13->setText("");
	ui.pushButton_13->setFocusPolicy(Qt::NoFocus);
	ui.pushButton_12->setIcon(QIcon("./ico/dr_keyboard.ico"));//文件copy到了exe所在目录
	ui.pushButton_12->setText("");//文件copy到了exe所在目录


	connect(ui.pushButton_13,SIGNAL(clicked()),this, SLOT(MoveOut()));
	//connect(ui.lE_RunSpeed, SIGNAL(POPUPKEYBOARD()), this, SLOT(MoveOut()));
	//connect(ui.lE_IP, SIGNAL(POPUPKEYBOARD()), this, SLOT(MoveOut()));
	//connect(ui.lE_Port, SIGNAL(POPUPKEYBOARD()), this, SLOT(MoveOut()));
	QList<MyLineEdit*> MyLineEditList = ui.tabWidget_PLC->findChildren<MyLineEdit*>();
	for (int i = 0; i < MyLineEditList.size(); i++)
	{
		connect(MyLineEditList[i], SIGNAL(POPUPKEYBOARD()), this, SLOT(MoveOut()));
	}

	//2nd
	QList<QAbstractButton*> buttonsList_1 = ui.buttonGroup_Keyboard_1->buttons();
	if (buttonsList_1.isEmpty()) {
		qDebug() << "isEmpty";
		return;
	}
	foreach(QAbstractButton * button_1, buttonsList_1)
	{
		((QPushButton*)button_1)->move(660, 550);//前12个按钮
		//((QPushButton*)button)->FocusManager.IsFocusScope = "False";//不获取焦点
		((QPushButton*)button_1)->setFocusPolicy(Qt::NoFocus);
	}
	ui.pushButton_13_1->setText("");
	ui.pushButton_13_1->setFocusPolicy(Qt::NoFocus);
	ui.pushButton_13_2->setVisible(false);
	ui.pushButton_13_2->move(540, 550);

	ui.pushButton_12_1->setIcon(QIcon("./ico/dr_keyboard.ico"));//文件copy到了exe所在目录
	ui.pushButton_12_1->setText("");//文件copy到了exe所在目录

	connect(ui.pushButton_13_1, SIGNAL(clicked()), this, SLOT(MoveOut_1()));
	//all
	connectBtnGroup();


	//键盘部分信号与槽↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑👆👆👆👆


	//Users部分的信号与槽↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
	QRegExp regx("[a-zA-Z0-9_]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui.lE_SetUserName->setValidator(new QRegExpValidator(regx, this));
	connect(ui.lE_SetUserName, SIGNAL(POPUPKEYBOARD()), this, SLOT(SWITCHTABANDOSK()));

	ui.cB_SetUserPermission->setEnabled(false);
	QRegExp regx2("[0-9]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui.lE_SetUserSecretNum->setValidator(new QRegExpValidator(regx2, this));
	ui.lE_SetUserSecretNum->setEnabled(false);
	ui.pB_AddUser->setEnabled(false);

	connect(ui.pB_Users_Exit, &QPushButton::clicked, [=]() {
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "退出确认", "<img src = './ico/question.png'/>\t确认退出系统设置页面吗？", "确认", "取消"))
		{
			ui.pB_StartContinueGrab->setChecked(false);
			ui.pB_StartGrab->setChecked(false);
			close();
		}});
	connect(ui.pB_AddUser, SIGNAL(clicked()), this, SLOT(addUser()));
	connect(ui.treeWidget_2, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onTreeItemChanged(QTreeWidgetItem*)));
	
	checkPermission();//查看权限部分

	connect(ui.cB_Users, SIGNAL(currentTextChanged(const QString)), this, SLOT(updateCheckPermission(const QString)));
	//connect(ui.comboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, QOverload<const QString&>::of(&QtCameraSet::updateCheckPermission));

	connect(ui.tabWidget_Users, SIGNAL(currentChanged(int)), this, SLOT(btn_Enabled(int)));
	
	connect(ui.pB_Users_Apply, &QPushButton::clicked, [=]() {
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("请退出本检测系统后使用新用户重新登录即可！"), 2000);
		levelOut->show();
		});

	initTableWidget();//自定义部分表格
	//Users部分的信号与槽↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

	bool b = connect(this, SIGNAL(SHOWEVERYPLCVALUE(DataToPC_typ)), this, SLOT(getPLCData(DataToPC_typ)));
	g_SocketPLC->set_message_handler(&message_handler, this);//全局

	// insocket on_pb_cmdParaLoad_clicked code;//PLC数据部分存储
	initFrame_light();//灯光调整
	b_changeCamera = false;
	b_autoAutoChange = false;
	int cou = g_vectorCamera.size();//相机参数vector
	QFont font;
	font = ui.tabWidget_CameraVec->font();//左下角白框的字体——[SimSun,21]
	font.setPointSize(21);
	label_Offline = new QLabel(ui.gB_ShowArea);

	if (0 == cou)
	{
		//在最上面图片显示区域新建label
		label_Offline->setObjectName(QString::fromUtf8("label_Offline"));
		label_Offline->setPixmap(QPixmap("./ico/dr-pharm.png"));
		label_Offline->setScaledContents(true);
		label_Offline->setGeometry(QRect(0, 0, ui.gB_ShowArea->width(), ui.gB_ShowArea->height()));
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("严重错误！未连接相机，请连接相机后重试！"), 2000);
		levelOut->show();
		return;
	}
	label_Offline->setVisible(false);
	int h = ui.gB_ShowArea->height() / cou;//宽度
	int w = ui.gB_ShowArea->width() / cou;//高度
	int n = font.pointSize();//字体大小

	//ui.tabWidget_CameraVec->setEnabled(false);

	for (int i = 0; i < cou; i++)//新建和相机数一样的
	{
		// 		if (!g_vectorCamera[i]->cb_Camera.IsOpen())//在NewUI_Demo初始化时加上了，此处注释掉
		// 		{
		// 			g_vectorCamera[i]->cb_Camera.Open();
		// 			g_vectorCamera[i]->cb_Camera.UserSetDefault.SetValue(UserSetDefault_UserSet3);
		// 			g_vectorCamera[i]->cb_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet3);
		// 			g_vectorCamera[i]->cb_Camera.UserSetLoad.Execute();
		// 		}

		MyLabel* label;
		label = new MyLabel(ui.gB_ShowArea,i);//在最上面图片显示区域新建label
		label->setPixmap(QPixmap("./ico/dr-pharm.png"));
		label->setScaledContents(true);
		label->setStyleSheet("border:1px solid #a6a6a6;");
		label->setObjectName(QString::fromUtf8("label") + QString::number(i));//eg:一个相机建一个label0，两个建label0,label1，以此类推
		label->setGeometry(QRect(0, 0 + h * i, ui.gB_ShowArea->width(), h));//大小区域为：从屏幕图片显示区（0，h*i）位置开始，显示一个gB_ShowArea->width()*h的界面（宽50，高25）
		label->setFrameShape(QFrame::Box);//设置外边框
		connect(label, SIGNAL(SWITCHTABPAGE(int)), this, SLOT (switchTabPage(int)));
		if (i<m_MultGetThread.size())//sizeof是指针长度为4，.size是指针对应的内容长度，为5
		{
		bool b = connect(this, SIGNAL(STARTGRABBING(int, bool)), m_MultGetThread[i], SLOT(ThreadGetImage(int, bool)));//若相机调试发出信号，多线程开始取图
		m_MultGetThread[i]->SetDirectShowDlg(label);//m_LabelShow = label
		b = connect(m_MultGetThread[i], SIGNAL(SHOWDIRECTIMG(Mat)), label, SLOT(showimg(Mat)));
		m_MultGetThread[i]->SetSaveImage(false);//m_LabelShow = label
		}
		else
		{
			showMsgBox(QMessageBox::Warning, "警告", "<img src = './ico/warning.png'/>\t算法库加载错误，请检查！", "确认", "");
		}

		QWidget* tab = new QWidget();//新建tab
		tab->setFont(font);//设置tab字体
		tab->setObjectName(QString::fromUtf8("tab_") + g_vectorCamera[i]->c_CameraName);//tab_23170685
		ui.tabWidget_CameraVec->addTab(tab, QString::number(i));// g_vectorCamera[i]->c_CameraName);//将tab添加到左下角tabwidget object name:tab_23170685 title:23170685

		tableWidget = new QTableWidget(tab);//tab下面加tablewidget
		tableWidget->setObjectName(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);//tableWidget_23170685
		int tw = tab->width();
		int th = tab->height();
		tableWidget->setGeometry(QRect(9, 9, tab->height() + 15, tab->width() - 85));//设置widget尺寸 黑边是边界

		QStringList strlist;
		strlist << QString::fromLocal8Bit("参数") << QString::fromLocal8Bit("数值");
		tableWidget->setColumnCount(2);//两列
		tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分填充表头
		tableWidget->verticalHeader()->setDefaultSectionSize(35);//默认行高20
		tableWidget->setHorizontalHeaderLabels(strlist);//水平表头参数、数值
		tableWidget->horizontalHeader()->setVisible(true);//表头可见
		font = tableWidget->horizontalHeader()->font();//表头字体
		font.setPointSize(20);//字号
		tableWidget->horizontalHeader()->setFont(font);//设置表头字体
		tableWidget->setFont(font);//设置内容字体
		
		initCameraSetTableWidget(tableWidget);//初始化tabwidget

		connect(tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(onCameraCellChange(int, int)));//int-row,int-column 伴有键盘取消操作
		connect(tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(keyboardMoveOut(int,int)));
		//connect(tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCameraKeyboardOut(int, int)));//int-row,int-column
	}
	initTableForAllPara();
	font = ui.tabWidget_CameraVec->font();//左下角白框的字体——[SimSun,21]
	connect(ui.tabWidget_CameraVec, SIGNAL(currentChanged(int)), this, SLOT(showCurrentImage(int)));
	font.setPointSize(20);

}

QtCameraSet::~QtCameraSet()
{
	if (m_serialPort->isOpen())
	{
		m_serialPort->close();
	}
	delete m_serialPort;


}

void QtCameraSet::savePLCParaInPLCTxt()//加一个按钮保存
{
	DataFromPC_typ typ;
	typ = getPCData();
	typ.Machine_Para.PhotoTimes=ui.cB_photoTimes->currentIndex() + 1;
	char* plcParam = new char[sizeof(DataFromPC_typ) + 1];
	memset(plcParam, 0, sizeof(DataFromPC_typ) + 1); //1实验用
	memcpy(plcParam, &typ, sizeof(DataFromPC_typ));
	int a = strlen(plcParam);//字符串的长度

	QString plcParamPath = AppPath + "/plcParam/plc.txt";

	QFile f(plcParamPath);
	f.resize(0);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		showMsgBox(QMessageBox::Question, "错误提示", "<img src = './ico/critical.png'/>\t参数文件写入错误，请重试", "我知道了", "");
		return;
	}
	f.write(plcParam, sizeof(DataFromPC_typ));
	f.close();
}
void QtCameraSet::initCameraSetTableWidget(QTableWidget* table)//初始化左下角部分
{
	QString title = table->objectName();//title是table的对象名 tableWdiget_23170685
	int i = 0;
	for (; i < g_vectorCamera.size(); i++)
	{
		if (title.contains(g_vectorCamera[i]->c_CameraName))//title包含相机名 23170685
		{
			break;
		}
	}
	if (i < g_vectorCamera.size())
	{
		int currentcolumn = table->rowCount();//行数00000000000000000000
		table->insertRow(currentcolumn);//插入行
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("触发源")));//0列设置
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));//单元格不可编辑
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));//单元格不可选择
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->TriggerBy)));//1列设置

		currentcolumn = table->rowCount();//111111111111111111111111
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("曝光时长")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_ExpouseTime)));

		currentcolumn = table->rowCount();//2222222222222222
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("调用算法")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString(g_vectorCamera[i]->c_AlgName)));
		table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

		currentcolumn = table->rowCount();//333333333333333333
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("连续触发帧率")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_LineRateHZ)));

		currentcolumn = table->rowCount();//444444444444444444
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI宽度")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_imgWidth)));

		currentcolumn = table->rowCount();//555555555555555555
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI高度")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_imgHeight)));

		currentcolumn = table->rowCount();//666666666666666666
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI_X偏移量")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_OffsetX)));

		currentcolumn = table->rowCount();//77777777777777777
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI_Y偏移量")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_OffsetY)));

		QComboBox *cB_NgOrOk = new QComboBox;
		QStringList strlst;
		strlst << QString::fromLocal8Bit("仅存储不合格品") << QString::fromLocal8Bit("全部存储")<< QString::fromLocal8Bit("仅存储合格品");
		cB_NgOrOk->addItems(strlst);
		cB_NgOrOk->setCurrentIndex(g_vectorCamera[i]->i_SaveOKorNG+1);
		connect(cB_NgOrOk, QOverload<int>::of(&QComboBox::activated),
			[=](int index) {
				int i_before = g_vectorCamera[i]->i_SaveOKorNG;// 判断数据有没有变化
				g_vectorCamera[i]->i_SaveOKorNG = index-1;
				if (i_before != index - 1)
				{
					b_changeCamera = true;
				}
			});
		currentcolumn = table->rowCount();//88888888888888888
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("存图标识符")));//存图标识符，-1，NG存图；0，全部保存；1，OK存图
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setCellWidget(currentcolumn, 1, cB_NgOrOk);
		//table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_SaveOKorNG)));

		currentcolumn = table->rowCount();//9999999999999999999
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("存图循环标识符"))); //存图循环标识符，0，全部存图； > 0，张数内循环；OK, NG图像进行单独计数。
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_SaveLoop)));

		currentcolumn = table->rowCount();//10 10 10 10 10 10 10 10 10
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("相机序列号")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
		table->setItem(currentcolumn, 1, new QTableWidgetItem(QString(g_vectorCamera[i]->c_CameraName)));
		table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

		currentcolumn = table->rowCount();//11 11 11 11 11 11 11
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("白平衡")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));

		QPushButton *btn = new QPushButton;
		btn->setText(QString::fromLocal8Bit("执行"));
		table->setCellWidget(currentcolumn, 1, btn);
		connect(btn, &QPushButton::clicked, [=]() {
			int ret = showMsgBox(QMessageBox::Question, "执行确认", "<img src = './ico/question.png'/>\t是否确认执行一次白平衡操作?", "确认", "取消");
			if (ret == QMessageBox::Yes)
			{
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
					g_vectorCamera[i]->cb_Camera.BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Once);
					b_changeCamera = true;
				}
			}
			});//利用lambda表达式可用
		currentcolumn = table->rowCount();//12 12 12 12 12 12 12 12 12 12
		table->insertRow(currentcolumn);
		table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("曝光时长")));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags()& (~Qt::ItemIsEditable));
		table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags()& (~Qt::ItemIsSelectable));

		QSlider* hsd = new QSlider(Qt::Horizontal);
		hsd->setStyleSheet("  \
     QSlider::add-page:Horizontal\
     {     \
        background-color: rgb(87, 97, 106);\
        height:4px;\
     }\
     QSlider::sub-page:Horizontal \
    {\
        background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(7,208,255, 255), stop:1 rgba(7,208,255, 255));\
        height:4px;\
     }\
    QSlider::groove:Horizontal \
    {\
        background:transparent;\
        height:6px;\
    }\
    QSlider::handle:Horizontal \
    {\
        height: 25px;\
        width:35px;\
        border-image: url(./ico/btn.png);\
        margin: -15 0px; \
    }\
    ");
		hsd->setMinimum(59);
		hsd->setMaximum(10000);
		hsd->setValue(g_vectorCamera[i]->i_ExpouseTime);
		table->setCellWidget(currentcolumn, 1, hsd);
		connect(hsd, &QSlider::valueChanged, [=]() {
			table->item(1, 1)->setText(QString::number(hsd->value()));
			});//利用lambda表达式可用

	}
}

void QtCameraSet::tableForAllPara(QTableWidget* table)
{
	int currentcolumn = table->rowCount();//行数00000000000000000000
	table->insertRow(currentcolumn);//插入行
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("触发源")));//0列设置
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));//单元格不可编辑
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));//单元格不可选择
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));//1列设置

	currentcolumn = table->rowCount();//111111111111111111111111
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("曝光时长")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	currentcolumn = table->rowCount();//2222222222222222
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("调用算法")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("----"));
	table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

	currentcolumn = table->rowCount();//333333333333333333
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("连续触发帧率")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	currentcolumn = table->rowCount();//444444444444444444
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI宽度")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	currentcolumn = table->rowCount();//555555555555555555
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI高度")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	currentcolumn = table->rowCount();//666666666666666666
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI_X偏移量")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	currentcolumn = table->rowCount();//77777777777777777
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("ROI_Y偏移量")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	QComboBox *cB_NgOrOk = new QComboBox;
	QStringList strlst;
	strlst << QString::fromLocal8Bit("仅存储不合格品") << QString::fromLocal8Bit("全部存储") << QString::fromLocal8Bit("仅存储合格品");
	cB_NgOrOk->addItems(strlst);
	cB_NgOrOk->setCurrentIndex(3);
	connect(cB_NgOrOk, QOverload<int>::of(&QComboBox::activated),
		[=](int index) {
		QList<QComboBox*> MyCBList = ui.tabWidget_CameraVec->findChildren<QComboBox*>();
		for (int i = 0; i < MyCBList.size(); i++)
		{
			MyCBList.at(i)->setCurrentIndex(index);
		}

			b_changeCamera = true;

	});
	currentcolumn = table->rowCount();//88888888888888888
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("存图标识符")));//存图标识符，-1，NG存图；0，全部保存；1，OK存图
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setCellWidget(currentcolumn, 1, cB_NgOrOk);
	//table->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(g_vectorCamera[i]->i_SaveOKorNG)));

	currentcolumn = table->rowCount();//9999999999999999999
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("存图循环标识符"))); //存图循环标识符，0，全部存图； > 0，张数内循环；OK, NG图像进行单独计数。
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("*"));

	currentcolumn = table->rowCount();//10 10 10 10 10 10 10 10 10
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("相机序列号")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	table->setItem(currentcolumn, 1, new QTableWidgetItem("----"));
	table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 1)->setFlags(table->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

	currentcolumn = table->rowCount();//11 11 11 11 11 11 11
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("白平衡")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));

	QPushButton *btn = new QPushButton;
	btn->setText(QString::fromLocal8Bit("执行"));
	table->setCellWidget(currentcolumn, 1, btn);
	connect(btn, &QPushButton::clicked, [=]() {
		int ret = showMsgBox(QMessageBox::Question, "执行确认", "<img src = './ico/question.png'/>\t是否确认执行一次白平衡操作?", "确认", "取消");
		if (ret == QMessageBox::Yes)
		{
			for (int i = 0; i < g_vectorCamera.size(); i++)
			{
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
					g_vectorCamera[i]->cb_Camera.BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Once);
					b_changeCamera = true;
				}
			}
		}
	});//利用lambda表达式可用
	currentcolumn = table->rowCount();//12 12 12 12 12 12 12 12 12 12
	table->insertRow(currentcolumn);
	table->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("曝光时长")));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags()& (~Qt::ItemIsEditable));
	table->item(currentcolumn, 0)->setFlags(table->item(currentcolumn, 0)->flags()& (~Qt::ItemIsSelectable));

	QSlider* hsd = new QSlider(Qt::Horizontal);
	hsd->setStyleSheet("  \
     QSlider::add-page:Horizontal\
     {     \
        background-color: rgb(87, 97, 106);\
        height:4px;\
     }\
     QSlider::sub-page:Horizontal \
    {\
        background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(7,208,255, 255), stop:1 rgba(7,208,255, 255));\
        height:4px;\
     }\
    QSlider::groove:Horizontal \
    {\
        background:transparent;\
        height:6px;\
    }\
    QSlider::handle:Horizontal \
    {\
        height: 25px;\
        width:35px;\
        border-image: url(./ico/btn.png);\
        margin: -15 0px; \
    }\
    ");
	hsd->setMinimum(59);
	hsd->setMaximum(10000);
	hsd->setValue(0);
	table->setCellWidget(currentcolumn, 1, hsd);
	connect(hsd, &QSlider::valueChanged, [=]() {
		table->item(1, 1)->setText(QString::number(hsd->value()));
	});//利用lambda表达式可用
}

void QtCameraSet::initListWidgetofModel()//模型listwidget初始化显示，遍历文件夹，找到模板名称并显示
{


	QSettings readDefaultModel(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
	QString defaultModel = readDefaultModel.value("ProgramSetting/DefaultModel", "testA").toString();
	ui.lb_DefaultName->setText(defaultModel);//更新lable 4-4
	QDir dir(AppPath + "\\ModelFile");
	if (!dir.exists())//不存在，退出
	{
		return;
	}
	dir.setFilter(QDir::Dirs);//筛选目录
	QFileInfoList list = dir.entryInfoList();//文件信息list

	int file_count = list.count();
	if (file_count <= 0)
	{
		return;
	}
	QStringList string_list;
	int i, j = 1;//j用于标记是否存在默认模板
	for (i = 0; i < file_count; i++)
	{
		QFileInfo file_info = list.at(i);
		QString folderName = file_info.fileName();
		string_list.append(folderName);
	}
	string_list.removeAt(0);//去掉.
	string_list.removeAt(0);//去掉..
	ui.listWidget->addItems(string_list);
	ui.listWidget->sortItems();
	QFont font("Arial", 24);
	ui.listWidget->setStyleSheet("QListWidget::Item:hover{background:grey; }"
	);
	int rowcount = ui.listWidget->count();
	for (int cnt = 0; cnt < rowcount; cnt++)
	{
		ui.listWidget->item(cnt)->setFont(font);
	}

	//显示模板👇👇当前应用模板

	for (i = 0; i < string_list.count(); i++)
	{
		if (string_list.at(i) == g_qModelName)//找到的默认模板和ini设置的一致
		{
			ui.listWidget->setCurrentRow(i);
			j = 0;//如果存在置为0
		}
	}
	if (j) {
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("严重错误！不存在初始模板！"), 2000);
		levelOut->show();
	}

}

bool QtCameraSet::compareModels()//哈希码比较两个文件
{
	QString strA1 = AppPath + "/ModelFile/" + g_qModelName + "/CameraConfig.ini";//源目录文件1
	QString strB1 = AppPath + "/DefaultModel" + "/CameraConfig.ini";//当前应用目录文件1
	QString strA2 = AppPath + "/ModelFile/" + g_qModelName + "/CheckParam.ini";//源目录文件2
	QString strB2 = AppPath + "/DefaultModel" + "/CheckParam.ini";//当前应用目录文件2
	QFile theFile1(strA1);
	theFile1.open(QIODevice::ReadOnly);
	QByteArray ba = QCryptographicHash::hash(theFile1.readAll(), QCryptographicHash::Md5);
	theFile1.close();
	qDebug() << ba.toHex().constData();

	QFile theFile2(strB1);
	theFile2.open(QIODevice::ReadOnly);
	QByteArray bb = QCryptographicHash::hash(theFile2.readAll(), QCryptographicHash::Md5);
	theFile2.close();
	qDebug() << bb.toHex().constData();

	QFile theFile3(strA2);
	theFile3.open(QIODevice::ReadOnly);
	QByteArray bc = QCryptographicHash::hash(theFile3.readAll(), QCryptographicHash::Md5);
	theFile3.close();
	qDebug() << bc.toHex().constData();

	QFile theFile4(strB2);
	theFile4.open(QIODevice::ReadOnly);
	QByteArray bd = QCryptographicHash::hash(theFile4.readAll(), QCryptographicHash::Md5);
	theFile4.close();
	qDebug() << bd.toHex().constData();
	if (ba == bb && bc == bd)
	{
		return true;
	}

	else
	{
		return false;
	}


}

void QtCameraSet::modelApply()//应用按钮有关
{
	QString selectedModel = ui.listWidget->currentItem()->text();

	if (g_qModelName == selectedModel)//模板名相同
	{
		if (compareModels())//同时内容也相同
		{
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("当前模板数据已是最新！无需点击应用"), 2000);
			levelOut->show();
			return;
		}
		else//内容有变化
		{
			int ret = showMsgBox(QMessageBox::Question, "更新确认", "<img src = './ico/question.png'/>\t是否确认更新当前正在使用的模板?", "确认", "取消");
			if (ret == QMessageBox::Yes)
			{
				//将默认文件覆盖到之前应用的模板👇👇👇
				QString modelFilePath = AppPath + "/ModelFile/" + g_qModelName;
				QString defaultModelFilePath = AppPath + "/DefaultModel";
				copyDirectoryFiles(defaultModelFilePath, modelFilePath, true);//true代表覆盖已有的
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("当前模板已更新至最新！"), 2000);
				levelOut->show();
				return;
			}
		}

	}
	else//模板名不同
	{
		if (compareModels())//但是之前的两个相同的模板名里的内容是相同的，就不用去管它
		{
			int ret = showMsgBox(QMessageBox::Question, "确认提示", "<img src = './ico/question.png'/>\t是否应用为当前模板？", "确认", "取消");
			if (QMessageBox::Yes == ret)
			{
				QString modelFilePath = AppPath + "/ModelFile/" + g_qModelName;
				QString defaultModelFilePath = AppPath + "/DefaultModel";

				//将当前所选模板复制到默认文件夹，并将currentModel更新👇👇👇
				//deleteDir(defaultModelFilePath);
				modelFilePath = AppPath + "/ModelFile/" + selectedModel;
				copyDirectoryFiles(modelFilePath, defaultModelFilePath, true);
				g_qModelName = selectedModel;////将之后的应用放入当前model

				QSettings Dir(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//找到文件
				Dir.setValue("ProgramSetting/LastModel", selectedModel);//写当前模板

				ui.lb_AppliedName->setText(g_qModelName);//更新lable 4-2
				QString defaultModelValue = Dir.value("ProgramSetting/DefaultModel", "zxc").toString();

				if (selectedModel != defaultModelValue)//看当前的是不是默认模板
				{
					int msgres = showMsgBox(QMessageBox::Question, "确认提示", "<img src = './ico/question.png'/>\t是否设置为默认模板？", "是", "否");
					if (QMessageBox::Yes == msgres)
					{
						Dir.setValue("ProgramSetting/DefaultModel", selectedModel);//写默认模板
						ui.lb_DefaultName->setText(selectedModel);//更新lable 4-1
					}
				}
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("设置已完成！"), 2000);
				levelOut->show();
				return;
			}
		}
		else
		{
			int ret = showMsgBox(QMessageBox::Question, "确认提示", "<img src = './ico/question.png'/>\t上一个应用的模板数据未保存，需要保存还是丢弃？", "保存", "丢弃");
			if (ret == QMessageBox::Yes)
			{
				//将默认文件覆盖到之前应用的模板👇👇👇
				QString modelFilePath = AppPath + "/ModelFile/" + g_qModelName;
				QString defaultModelFilePath = AppPath + "/DefaultModel";
				copyDirectoryFiles(defaultModelFilePath, modelFilePath, true);//true代表覆盖已有的
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("上一个应用的模板数据已更新至最新！"), 2000);
				levelOut->show();

				return;
			}
			else
			{
				QString modelFilePath = AppPath + "/ModelFile/" + g_qModelName;
				QString defaultModelFilePath = AppPath + "/DefaultModel";
				copyDirectoryFiles(modelFilePath, defaultModelFilePath, true);//true代表覆盖已有的
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("上一个应用的模板数据已更新至最新！"), 2000);
				levelOut->show();
			}
			int res = showMsgBox(QMessageBox::Question, "确认提示", "<img src = './ico/question.png'/>\t是否应用选中模板为当前模板？", "确认", "取消");
			if (ret == QMessageBox::Yes)
			{
				QString modelFilePath = AppPath + "/ModelFile/" + g_qModelName;
				QString defaultModelFilePath = AppPath + "/DefaultModel";

				//将当前所选模板复制到默认文件夹，并将currentModel更新👇👇👇
				//deleteDir(defaultModelFilePath);
				modelFilePath = AppPath + "/ModelFile/" + selectedModel;
				copyDirectoryFiles(modelFilePath, defaultModelFilePath, true);
				g_qModelName = selectedModel;////将之后的应用放入当前model

				QSettings Dir(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//找到文件
				Dir.setValue("ProgramSetting/LastModel", selectedModel);//写当前模板

				ui.lb_AppliedName->setText(g_qModelName);//更新lable 4-5

				QString defaultModelValue = Dir.value("ProgramSetting/DefaultModel", "zxc").toString();

				if (selectedModel != defaultModelValue)//看当前的是不是默认模板
				{
					int msgres = showMsgBox(QMessageBox::Question, "确认提示", "<img src = './ico/question.png'/>\t是否设置为默认模板？", "是", "否");
					if (QMessageBox::Yes == msgres)
					{
						Dir.setValue("ProgramSetting/DefaultModel", selectedModel);//写默认模板
						ui.lb_DefaultName->setText(selectedModel);//更新lable 4-6
					}
				}
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("设置已完成！"), 2000);
				levelOut->show();
				return;

			}
		}
	}
}

bool QtCameraSet::SaveSetParam()
{
	QSettings configIniWrite(AppPath + "\\ModelFile\\" + g_qModelName + "\\CameraConfig.ini", QSettings::IniFormat);
	for (int i = 0; i < g_vectorCamera.size(); i++)
	{
 		QString cam = g_vectorCamera[i]->c_CameraName;
 		configIniWrite.setValue(cam + "/TriggerBy", g_vectorCamera[i]->TriggerBy);//将修改的参数存入ini，下同
 		configIniWrite.setValue(cam + "/ExpouseTime", g_vectorCamera[i]->i_ExpouseTime);
 		configIniWrite.setValue(cam + "/LineRateHZ", g_vectorCamera[i]->i_LineRateHZ);
 		configIniWrite.setValue(cam + "/imgWidth", g_vectorCamera[i]->i_imgWidth);
 		configIniWrite.setValue(cam + "/imgHeight", g_vectorCamera[i]->i_imgHeight);
 		configIniWrite.setValue(cam + "/OffsetX", g_vectorCamera[i]->i_OffsetX);
 		configIniWrite.setValue(cam + "/OffsetY", g_vectorCamera[i]->i_OffsetY);
 		configIniWrite.setValue(cam + "/SaveOKorNG", g_vectorCamera[i]->i_SaveOKorNG);
 		configIniWrite.setValue(cam + "/SaveLoop", g_vectorCamera[i]->i_SaveLoop);
 		configIniWrite.setValue(cam + "/CameraSign", g_vectorCamera[i]->c_CameraSign);
		if (!g_vectorCamera[i]->cb_Camera.IsOpen())
		{
			g_vectorCamera[i]->cb_Camera.Open();
		}
		g_vectorCamera[i]->cb_Camera.StopGrabbing();
		g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(false);
		g_vectorCamera[i]->cb_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet3);
		g_vectorCamera[i]->cb_Camera.UserSetSave.Execute();
		
	}
	return false;
}
bool QtCameraSet::DismissParamChange()
{
	QSettings configIniRead(AppPath + "\\ModelFile\\" + g_qModelName + "\\CameraConfig.ini", QSettings::IniFormat);
	for (int i = 0; i < g_vectorCamera.size(); i++)
	{
		QString cam = g_vectorCamera[i]->c_CameraName;
		g_vectorCamera[i]->TriggerBy = configIniRead.value(cam + "/TriggerBy", 0).toInt();//复原，下同
		g_vectorCamera[i]->i_ExpouseTime = configIniRead.value(cam + "/ExpouseTime", 0).toInt();
		g_vectorCamera[i]->i_LineRateHZ = configIniRead.value(cam + "/LineRateHZ", 0).toInt();
		g_vectorCamera[i]->i_imgWidth = configIniRead.value(cam + "/imgWidth", 0).toInt();
		g_vectorCamera[i]->i_imgHeight = configIniRead.value(cam + "/imgHeight", 0).toInt();
		g_vectorCamera[i]->i_OffsetX = configIniRead.value(cam + "/OffsetX", 0).toInt();
		g_vectorCamera[i]->i_OffsetY = configIniRead.value(cam + "/OffsetY", 0).toInt();
		g_vectorCamera[i]->i_SaveOKorNG = configIniRead.value(cam + "/SaveOKorNG", 0).toInt();
		g_vectorCamera[i]->i_SaveLoop = configIniRead.value(cam + "/SaveLoop", 0).toInt();
		strcpy(g_vectorCamera[i]->c_CameraSign, configIniRead.value(cam + "/CameraSign").toString().toStdString().c_str());
		if (!g_vectorCamera[i]->cb_Camera.IsOpen())
		{
			g_vectorCamera[i]->cb_Camera.Open();
			g_vectorCamera[i]->cb_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet3);
			g_vectorCamera[i]->cb_Camera.UserSetLoad.Execute();
			g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(false);
		}
	}
	return false;
}

void QtCameraSet::onShowAlgSet()//算法设置
{
	QString str = g_vectorCamera[ui.tabWidget_CameraVec->currentIndex()]->c_CameraName;//当前选中的相机

	CBaseCheckAlg* _tempalg = EnsureAlg(str);
	if (_tempalg != nullptr)//算法指针不为空
	{
		QRect s = this->geometry();//
		if (_tempalg->ShowParamDlg(this, true) == 1)//参数值改了时为1
		{
			//拷回原来的目录👇👇👇
			int ret = showMsgBox(QMessageBox::Information, "模板参数修改", "<img src = './ico/question.png'/>\t默认模板参数已修改\n是否保存？", "是", "否");
			if (ret == QMessageBox::No)//不保存
			{
				return;
			}
			else//保存
			{
				QSettings readDefaultModel(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
				QString defaultModel = readDefaultModel.value("ProgramSetting/DefaultModel", "testA").toString();
				QString modelFilePath = AppPath + "/ModelFile/" + defaultModel;//默认模板源目录
				QString defaultModelFilePath = AppPath + "/DefaultModel";//默认模板目录，开系统有，关系统删
				copyDirectoryFiles(defaultModelFilePath, modelFilePath, true);//算法改后拷回源目录
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("默认模板参数已保存！"), 2000);
				levelOut->show();
			}

		}
	}

}

void QtCameraSet::onContinueGrab(bool b)
{
	//QString str = ui.tabWidget_CameraVec->tabText(ui.tabWidget_CameraVec->currentIndex());//标签索引以0开始，该标签上的text赋值给str
	int i = ui.tabWidget_CameraVec->currentIndex();
	if (i < g_vectorCamera.size())
	
	//for (; i < g_vectorCamera.size(); i++)
// 	{
// 		if (str == g_vectorCamera[i]->c_CameraName)
// 		{
// 			break;//如果名字一样跳出for循环
// 		}
// 	}
// 	if (i == g_vectorCamera.size())
// 	{
// 		return;//未找到相同的相机return掉
// 	}
// 	else
	{
		if (b)//由信号传递而来，按下是1，再按下是0，以此循环，1执行这一段
		{
			ui.pB_StartGrab->setEnabled(false);
			ui.pB_StartGrab->setChecked(false);//将右侧开始测试按钮复位
			ui.pB_StartContinueGrab->setStyleSheet("background: rgb(0,255,0)");
			emit STARTGRABBING(i, 1);//发送取图信号，连接多线程槽函数
		}
		else//0执行这一段
		{
			ui.pB_StartGrab->setEnabled(true);
			ui.cB_flash->setChecked(false);
			ui.pB_StartContinueGrab->setStyleSheet("");
#ifdef BASLER
			g_vectorCamera[i]->cb_Camera.StopGrabbing();//停止取图
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
	}


}

//should change with initTableWidget function as the same time
void QtCameraSet::onCameraCellChange(int r, int c)//tableWidget_camera参数的变化，r-行 c-列//每次数值变化会执行一次该行
{
	((QTableWidget*)sender())->blockSignals(true);

	//SWITCHOSK();
	if (b_autoAutoChange)//这个不知道干啥用的，可能类似于blockSignals
	{
		b_autoAutoChange = false;
		((QTableWidget*)sender())->blockSignals(false);
		return;
	}
	QString sendername = ((QTableWidget*)sender())->objectName();//改变的对象object名称
	int i = 0;
	for (; i < g_vectorCamera.size(); i++)
	{
		if (sendername.contains(g_vectorCamera[i]->c_CameraName))
		{
			break;
		}
	}
	try
	{
#ifdef DAHENG
		//发送停采命令
		CGXFeatureControlPointer m_objFeatureControlPtr = g_vectorCamera[i]->m_objDevicePtr->GetRemoteFeatureControl();
		if (r == 4 || r == 5 || r == 6 || r == 7)
		{
			m_objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();

			//关闭流层采集
			g_vectorCamera[i]->m_objStreamPtr->StopGrab();

			//注销采集回调函数
			g_vectorCamera[i]->m_objStreamPtr->UnregisterCaptureCallback();
		}
#endif // DAHENG

		if (i <= g_vectorCamera.size())
		{
#ifdef BASLER
			if (!g_vectorCamera[i]->cb_Camera.IsOpen())
			{
				g_vectorCamera[i]->cb_Camera.Open();
			}
			if (r == 4 || r == 5)
			{
				g_vectorCamera[i]->cb_Camera.StopGrabbing();//停止抓取
				QThread::msleep(5);
				if (!g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.Open();
				}
			}
#endif
			switch (r)
			{
			case 0:
			{
#ifdef BASLER
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
#endif
				{
					int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();

					int valueBefore = g_vectorCamera[i]->TriggerBy;
					if (!((values == -1) || (values == 0 && IsNumber(((QTableWidget*)sender())->item(r, c)->text())) || (values == 1) || (values == 2)))
					{
						((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
						levelOut = new WindowOut;
						levelOut->getString(QString::fromLocal8Bit("请输入-1（包含）到2（包含）之间的整数"), 2000);
						levelOut->show();
						break;
					}
					g_vectorCamera[i]->TriggerBy = values;
					switch (values)
					{
					case -1:
					{
						SYS_logger->info("ThreadGetImage No.{} SetTriggerOff", values);
						SetTriggerOff(i);
						break;
					}
					case 0:
					case 1:
					case 2:
					{
						SYS_logger->info("ThreadGetImage No.{} SetTriggerLine1", values);
						SetTriggerLine(i, r);
						break;
					}
					default:
						break;
					}
					b_changeCamera = true;
				}
				break;
			}
			case 1:
			{
				//expouse time
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_ExpouseTime;
				if (!((values >= 59) && (values <= 10000)))//actually it canbe 1000000 at largest
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入59（包含）到10000（包含）之间的整数"), 2000);//(basler相机本身支持到29-1000，0000) 大恒20-100，0000了通用就这样设置
					levelOut->show();
					break;
				}
				QSlider* sld = ((QTableWidget*)sender())->findChild<QSlider*>();
				sld->setValue(values);
				g_vectorCamera[i]->i_ExpouseTime = values;
				b_changeCamera = true;
#ifdef BASLER
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.ExposureTime.SetValue(values);
				}
#endif
#ifdef DAHENG
				m_objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(values);
#endif
				break;
			}
			case 2:
			{
				break;
			}
			case 3:
			{//FPS
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();//basler支持0.00288到1000000,大恒是0.8开始到10000，为了通用就如下这样设置  大概其
				int valueBefore = g_vectorCamera[i]->i_LineRateHZ;
				if (!((values >= 1) && (values <= 10)))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入1（包含）到10（包含）之间的整数"), 2000);
					levelOut->show();
					break;
				}
				g_vectorCamera[i]->i_LineRateHZ = values;
				b_changeCamera = true;
#ifdef BASLER
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.AcquisitionFrameRateEnable.SetValue(true);
					g_vectorCamera[i]->cb_Camera.AcquisitionFrameRate.SetValue(values);
				}
#endif
#ifdef DAHENG
				m_objFeatureControlPtr->GetEnumFeature("AcquisitionFrameRateMode")->SetValue("On");
				m_objFeatureControlPtr->GetFloatFeature("AcquisitionFrameRate")->SetValue(values);
#endif
				break;
			}
			case 4:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_imgWidth;
#ifdef BASLER
				if ((values < 16) || (values > 1280))
				{

					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入16（包含）到1280（包含）之间的整数"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					break;
				}
				values = values / 16 * 16;
				int valueOffset = g_vectorCamera[i]->i_OffsetX;
				if ((values + valueOffset) > 1280)
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("由于ROI宽度与ROI_X偏移量之和大于1280，\n已将ROI_X偏移量设置为0，如有需要请重新设置"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(6, 1)->setText(QString::number(0));
					if (g_vectorCamera[i]->cb_Camera.IsOpen())
					{
						g_vectorCamera[i]->cb_Camera.OffsetX.SetValue(0);
					}
					g_vectorCamera[i]->i_OffsetX = 0;
				}

				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.Width.SetValue(values);
				}
				g_vectorCamera[i]->i_imgWidth = values;
				b_changeCamera = true;
#endif
#ifdef DAHENG
				if (!((values >= 64) && (values <= 1280)))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入64（包含）到1280（包含）之间的整数"), 2000);
					levelOut->show();
					break;
				}
				values = values / 16 * 16;
				int valueOffset = g_vectorCamera[i]->i_OffsetX;
				if (!((values + valueOffset) <= 1280))
				{
					((QTableWidget*)sender())->item(6, 1)->setText(QString::number(0));
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("由于ROI宽度与ROI_X偏移量之和大于1280，\n已将ROI_X偏移量设置为0，如有需要请重新设置"), 2000);
					levelOut->show();

				}
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				m_objFeatureControlPtr->GetIntFeature("Width")->SetValue(values);
				g_vectorCamera[i]->i_imgWidth = values;
				b_changeCamera = true;

#endif
				break;
			}
			case 5:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_imgHeight;
#ifdef BASLER
				if ((values < 2) || (values > 1024))
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入2（包含）到1024（包含）之间的整数"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					break;
				}
				values = values / 2 * 2;
				int valueOffset = g_vectorCamera[i]->i_OffsetY;
				if ((values + valueOffset) > 1024)
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("由于ROI高度与ROI_Y偏移量之和大于1024，\n已将ROI_Y偏移量设置为0，如有需要请重新设置"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(7, 1)->setText(QString::number(0));
					if (g_vectorCamera[i]->cb_Camera.IsOpen())
					{
						g_vectorCamera[i]->cb_Camera.OffsetY.SetValue(0);
					}
					g_vectorCamera[i]->i_OffsetY = 0;
				}
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.Height.SetValue(values);
				}
				g_vectorCamera[i]->i_imgHeight = values;
				b_changeCamera = true;
#endif
#ifdef DAHENG
				if (!((values >= 64) && (values <= 1024)))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入64（包含）到1024（包含）之间的整数"), 2000);
					levelOut->show();
					break;
				}
				values = values / 2 * 2;
				int valueOffset = g_vectorCamera[i]->i_OffsetY;
				if (!((values + valueOffset) <= 1024))
				{
					((QTableWidget*)sender())->item(7, 1)->setText(QString::number(0));
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("由于ROI高度与ROI_Y偏移量之和大于1024，\n已将ROI_Y偏移量设置为0，如有需要请重新设置"), 2000);
					levelOut->show();
				}
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				m_objFeatureControlPtr->GetIntFeature("Height")->SetValue(values);
				g_vectorCamera[i]->i_imgHeight = values;
				b_changeCamera = true;

#endif
				break;
			}
			case 6:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_OffsetX;
				int valueImgWidth = g_vectorCamera[i]->i_imgWidth;
#ifdef BASLER
				if ((values + valueImgWidth < 16) || (values + valueImgWidth > 1280) || values < 0 || (!IsNumber(((QTableWidget*)sender())->item(r, c)->text())))
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("ROI宽度与ROI_X偏移量之和须介于16（包含）\n和1280（包含）之间的非负数，请重新输入"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					break;
				}
				values = values / 16 * 16;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.OffsetX.SetValue(values);
				}
				g_vectorCamera[i]->i_OffsetX = values;
				b_changeCamera = true;
#endif
#ifdef DAHENG
				if (!((values + valueImgWidth >= 64) && (values + valueImgWidth <= 1280) && values >= 0 && (IsNumber(((QTableWidget*)sender())->item(r, c)->text()))))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("ROI宽度与ROI_X偏移量之和须介于64（包含）和\n1280（包含）之间，且为非负数，请重新输入"), 2000);
					levelOut->show();
					break;
				}
				values = values / 16 * 16;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				m_objFeatureControlPtr->GetIntFeature("OffsetX")->SetValue(values);
				g_vectorCamera[i]->i_OffsetX = values;
				b_changeCamera = true;

#endif
				break;
			}
			case 7:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_OffsetY;
				int valueImgHeight = g_vectorCamera[i]->i_imgHeight;
#ifdef BASLER
				if ((values + valueImgHeight < 2) || (values + valueImgHeight > 1024) || values < 0 || (!IsNumber(((QTableWidget*)sender())->item(r, c)->text())))
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("ROI高度与ROI_Y偏移量之和须介于2（包含）\n和1024（包含）之间的非负数，请重新输入"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化  baslor should change as below
					break;
				}
				values = values / 2 * 2;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				if (g_vectorCamera[i]->cb_Camera.IsOpen())
				{
					g_vectorCamera[i]->cb_Camera.OffsetY.SetValue(values);
				}
				g_vectorCamera[i]->i_OffsetY = values;
				b_changeCamera = true;
#endif
#ifdef DAHENG
				if (!((values + valueImgHeight >= 64) && (values + valueImgHeight <= 1024) && values >= 0 && (IsNumber(((QTableWidget*)sender())->item(r, c)->text()))))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("ROI高度与ROI_Y偏移量之和须介于64（包含）和\n1024（包含）之间的非负数，请重新输入"), 2000);
					levelOut->show();
					break;
				}
				values = values / 2 * 2;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				m_objFeatureControlPtr->GetIntFeature("OffsetY")->SetValue(values);
				g_vectorCamera[i]->i_OffsetY = values;
				b_changeCamera = true;

#endif
				break;
			}
			case 8:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_SaveOKorNG;
				if (!((values == -1) || (values == 0 && (IsNumber(((QTableWidget*)sender())->item(r, c)->text()))) || (values == 1)))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入-1（包含）到1（包含）之间的整数"), 2000);
					levelOut->show();
					break;
				}
				g_vectorCamera[i]->i_SaveOKorNG = values;
				b_changeCamera = true;
				break;
			}
			case 9:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = g_vectorCamera[i]->i_SaveLoop;
				if (!((values >= 0 && (IsNumber(((QTableWidget*)sender())->item(r, c)->text())))|| values == -1))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入大于等于-1的整数"), 2000);
					levelOut->show();
					break;
				}
				g_vectorCamera[i]->i_SaveLoop = values;
				b_changeCamera = true;
				break;
			}

			case 10:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = QString(g_vectorCamera[i]->c_CameraSign).mid(3).toInt();
				for (int j = 0; j < g_vectorCamera.size(); j++)
				{
					QString str = ((QTableWidget*)sender())->objectName().mid(12);
					if (str != g_vectorCamera[j]->c_CameraName
						&& values == QString(g_vectorCamera[j]->c_CameraSign).mid(3).toInt())
					{
						((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
						levelOut = new WindowOut;
						levelOut->getString(QString::fromLocal8Bit("序号冲突！\n请修改相机序号为") + QString(g_vectorCamera[j]->c_CameraName) + QString::fromLocal8Bit("的相机序号再修改该值"), 2000);
						levelOut->show();
						break;

					}
					if (!(IsNumber(((QTableWidget*)sender())->item(r, c)->text())))
					{
						((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
						levelOut = new WindowOut;
						levelOut->getString(QString::fromLocal8Bit("请输入数字！"), 2000);
						levelOut->show();
						break;

					}
				}

				char src[100], dest[100], dest2[100];//太小会溢出

				strcpy(dest2, "POS");
				strcpy(dest, ((QTableWidget*)sender())->item(r, c)->text().toStdString().c_str());

				strcat(dest2, dest);
				strcpy(g_vectorCamera[i]->c_CameraSign, dest2);
				b_changeCamera = true;
				break;
			}

			default:
				break;
			}
		}

		if (r == 4 || r == 5)
		{
			if (ui.pB_StartContinueGrab->isChecked())
			{
				onContinueGrab(true);
			}
			if (ui.pB_StartGrab->isChecked())
			{
				StartGrab(true);
			}
		}
	}
	catch (std::exception& e)
	{
		QString st = e.what();
		QMessageBox::warning(this, tr("Pylon Error004"),
			st, QMessageBox::Abort);
		((QTableWidget*)sender())->blockSignals(false);
		return;
	}
	catch (const GenericException& e)
	{
		// Error handling.
		QString st = e.what();
		QMessageBox::warning(this, tr("Pylon Error008"),
			st, QMessageBox::Abort);
		((QTableWidget*)sender())->blockSignals(false);
		return;
	}


	((QTableWidget*)sender())->blockSignals(false);
}

//should change with initTableWidget function as the same time
void QtCameraSet::onCameraCellChangeForAll(int r, int c)//tableWidget_camera参数的变化，r-行 c-列//每次数值变化会执行一次该行
{
	((QTableWidget*)sender())->blockSignals(true);


			switch (r)
			{
			case 0:
			{

					int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();

					int valueBefore = -401;
					if (!((values == -1) || (values == 0 && IsNumber(((QTableWidget*)sender())->item(r, c)->text())) || (values == 1) || (values == 2)))
					{
						((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
						levelOut = new WindowOut;
						levelOut->getString(QString::fromLocal8Bit("请输入-1（包含）到2（包含）之间的整数"), 2000);
						levelOut->show();
						break;
					}
					for (int i = 0; i < g_vectorCamera.size(); i++)
					{
						QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

						oneTable->item(r, c)->setText(QString::number(values));
					}
					break;
			}
			case 1:
			{
				//expouse time
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
				if (!((values >= 59) && (values <= 10000)))//actually it canbe 1000000 at largest
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入59（包含）到10000（包含）之间的整数"), 2000);//(basler相机本身支持到29-1000，0000) 大恒20-100，0000了通用就这样设置
					levelOut->show();
					break;
				}
				QSlider* sld = ((QTableWidget*)sender())->findChild<QSlider*>();
				sld->setValue(values);

				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
				break;
			}
			case 2:
			{
				break;
			}
			case 3:
			{//FPS
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();//basler支持0.00288到1000000,大恒是0.8开始到10000，为了通用就如下这样设置  大概其
				int valueBefore =-401;
				if (!((values >= 1) && (values <= 10)))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入1（包含）到10（包含）之间的整数"), 2000);
					levelOut->show();
					break;
				}

				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
				break;
			}
			case 4:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
#ifdef BASLER
				if ((values < 16) || (values > 1280))
				{

					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入16（包含）到1280（包含）之间的整数"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					break;
				}
				values = values / 16 * 16;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));
				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
#endif
				break;
			}
			case 5:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
#ifdef BASLER
				if ((values < 2) || (values > 1024))
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入2（包含）到1024（包含）之间的整数"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					break;
				}
				values = values / 2 * 2;
				
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));

				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
#endif
				break;
			}
			case 6:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
				int valueImgWidth =0;
#ifdef BASLER
				if ((values + valueImgWidth < 16) || (values + valueImgWidth > 1280) || values < 0 || (!IsNumber(((QTableWidget*)sender())->item(r, c)->text())))
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("ROI宽度与ROI_X偏移量之和须介于16（包含）\n和1280（包含）之间的非负数，请重新输入"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					break;
				}
				values = values / 16 * 16;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));

				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
#endif
				break;
			}
			case 7:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
				int valueImgHeight = 0;
#ifdef BASLER
				if ((values + valueImgHeight < 2) || (values + valueImgHeight > 1024) || values < 0 || (!IsNumber(((QTableWidget*)sender())->item(r, c)->text())))
				{
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("ROI高度与ROI_Y偏移量之和须介于2（包含）\n和1024（包含）之间的非负数，请重新输入"), 2000);
					levelOut->show();
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化  baslor should change as below
					break;
				}
				values = values / 2 * 2;
				((QTableWidget*)sender())->item(r, c)->setText(QString::number(values));

				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
#endif
				break;
			}
			case 8:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
				if (!((values == -1) || (values == 0 && (IsNumber(((QTableWidget*)sender())->item(r, c)->text()))) || (values == 1)))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入-1（包含）到1（包含）之间的整数"), 2000);
					levelOut->show();
					break;
				}

				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
			
				break;
			}
			case 9:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = -401;
				if (!((values >= 0 && (IsNumber(((QTableWidget*)sender())->item(r, c)->text()))) || values == -1))
				{
					((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("请输入大于等于-1的整数"), 2000);
					levelOut->show();
					break;
				}
				for (int i = 0; i < g_vectorCamera.size(); i++)
				{
					QTableWidget* oneTable = ui.tabWidget_CameraVec->findChild<QTableWidget*>(QString::fromUtf8("tableWidget_") + g_vectorCamera[i]->c_CameraName);

					oneTable->item(r, c)->setText(QString::number(values));
				}
				break;
			}

			case 10:
			{
				int values = ((QTableWidget*)sender())->item(r, c)->text().toInt();
				int valueBefore = QString(g_vectorCamera[0]->c_CameraSign).mid(3).toInt();
				for (int j = 0; j < g_vectorCamera.size(); j++)
				{
					QString str = ((QTableWidget*)sender())->objectName().mid(12);
					if (str != g_vectorCamera[j]->c_CameraName
						&& values == QString(g_vectorCamera[j]->c_CameraSign).mid(3).toInt())
					{
						((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
						levelOut = new WindowOut;
						levelOut->getString(QString::fromLocal8Bit("序号冲突！\n请修改相机序号为") + QString(g_vectorCamera[j]->c_CameraName) + QString::fromLocal8Bit("的相机序号再修改该值"), 2000);
						levelOut->show();
						break;

					}
					if (!(IsNumber(((QTableWidget*)sender())->item(r, c)->text())))
					{
						((QTableWidget*)sender())->item(r, c)->setText(QString::number(valueBefore));//取消变化
						levelOut = new WindowOut;
						levelOut->getString(QString::fromLocal8Bit("请输入数字！"), 2000);
						levelOut->show();
						break;

					}
				}

				char src[100], dest[100], dest2[100];//太小会溢出

				strcpy(dest2, "POS");
				strcpy(dest, ((QTableWidget*)sender())->item(r, c)->text().toStdString().c_str());

				strcat(dest2, dest);
				strcpy(g_vectorCamera[0]->c_CameraSign, dest2);
				b_changeCamera = true;
				break;
			}

			default:
				break;
			}

	((QTableWidget*)sender())->blockSignals(false);
}

#ifdef PLCCONNECT

void message_handler(void* context, DataToPC_typ data)
{
	((QtCameraSet*)context)->emit SHOWEVERYPLCVALUE(data);
}


void QtCameraSet::closeEvent(QCloseEvent*)
{
	StopGrab();
	if (g_SocketPLC!=nullptr)
	{
		g_SocketPLC->set_message_handler(nullptr, this);
	}
	//	ui.pb_StReadPLC->setChecked(false);//读取按钮弹起
	//QThread::msleep(100);

	if (b_changeCamera)//参数有更改的话，下面显示提示框
	{
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Warning, "修改确认", "<img src = './ico/question.png'/>\t相机参数已修改，是否保存？", "保存", "取消"))
		{
			SaveSetParam();//退出询问是否保存参数
		}
		else
		{
			DismissParamChange();
		}
	}

	for (int i = 0; i < g_vectorCamera.size(); i++)
	{
		if (g_vectorCamera[i]->cb_Camera.IsOpen())
		{
			g_vectorCamera[i]->cb_Camera.Close();
		}
		m_MultGetThread[i]->SetDirectShowDlg(nullptr);
	}

	disconnect(this, SIGNAL(SHOWEVERYPLCVALUE(DataToPC_typ)), this, SLOT(getPLCData(DataToPC_typ)));
}

void QtCameraSet::keyboardMoveOut(int row, int column)//点击相机参数弹出键盘
{
	if (column==1)
	{
		if (row==0|| row == 1 || row == 3 || row == 4 || row == 5 || row == 6 || row == 7 || row == 8 || row == 9 )
		{
			if (ui.pushButton_13_1->text() == "")
			{
				MoveOut_1();
			}
		}
	}
}


void QtCameraSet::onReadPLCParam(bool b)
{
	if (b)
	{
		//g_SocketPLC->set_message_handler(&message_handler, this);
//		ui.pb_StReadPLC->setStyleSheet("background: rgb(0,255,0)");
	}
	else
	{
		//g_SocketPLC->set_message_handler(nullptr, this);
// 		ui.pb_StReadPLC->setStyleSheet("");
// 		ui.pb_StReadPLC->setFont(font);
	}

}
void QtCameraSet::onStartStore(bool b)
{
	if (b)
	{
		//PLC转囊
		for (int i = 0; i < m_MultGetThread.size(); i++)
		{
			m_MultGetThread[i]->SetSaveImage(true);//m_LabelShow = label
		}
		emit STARTGRABBING(-1, 0);
		//ui.pB_StartStore->setText(QString::fromLocal8Bit("停止存图"));

		DataFromPC_typ typ;
		typ.Dummy = 0;//占空
		typ.Telegram_typ = 1;//命令报文
		typ.Machine_Cmd.cmdRotateCtl = 1;					//
		g_SocketPLC->Communicate_PLC(&typ, nullptr);//
	}
	else
	{
		for (int i = 0; i < m_MultGetThread.size(); i++)
		{
			m_MultGetThread[i]->SetSaveImage(false);//m_LabelShow = label
			g_vectorCamera[i]->cb_Camera.StopGrabbing();//停止取图
		}

		//ui.pB_StartStore->setText(QString::fromLocal8Bit("开始存图"));
		DataFromPC_typ typ;
		typ.Dummy = 0;//占空
		typ.Telegram_typ = 1;//命令报文
		typ.Machine_Cmd.cmdRotateCtl = 2;					//
		g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收
	}
}
void QtCameraSet::onSendPLCCommand(bool b)
{
	QPushButton* obj = qobject_cast<QPushButton*>(sender());//判断是哪个按钮触发了槽函数
	obj->setFocus();
	QString objname = obj->objectName();//获取触发者objectname
	DataFromPC_typ typ;
	typ = getPCData();
	typ.Dummy = 0;//占空
	typ.Telegram_typ = 1;//命令报文
	if (obj != nullptr)
	{

		if (b)
		{
			if (objname == "pb_StWritePLC")
			{
				typ.Telegram_typ = 2;
				ui.pb_StWritePLC->setStyleSheet("background: rgb(0,255,0)");
				g_SocketPLC->Communicate_PLC(&typ, nullptr);//系统

				typ.Telegram_typ = 4;
				g_SocketPLC->Communicate_PLC(&typ, nullptr);//运行
			}//参数报文
			else if (objname == "pB_enPhoto" || objname == "pB_enReject" || objname == "pB_enFeed" || objname == "pB_enRotate")
			{
				typ.Telegram_typ = 4;//运行报文
				if (objname == "pB_enPhoto")
				{
					typ.Run_Para.enPhoto = 1;
				}	
				if (objname == "pB_enReject")
				{
					typ.Run_Para.enReject = 1;
				}
				if (objname == "pB_enFeed")
				{
					typ.Run_Para.enFeed = 1;
				}
				if (objname == "pB_enRotate")
				{
					typ.Run_Para.enRotate = 1;
				}
				obj->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
				g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收
			}

			else {
				if (objname.contains("pb_cmdTestKick"))
				{
					int index_cmdkick = objname.mid(14).toInt();//从0开始，14位之后的字符转为int，0-29
					typ.Machine_Cmd.cmdTestKick[index_cmdkick] = 1;//结构体数据赋值
				}
				if (objname == "pb_cmdHome") typ.Machine_Cmd.cmdHome = 1;//结构体数据赋值
				if (objname == "pb_cmdStart") typ.Machine_Cmd.cmdStart = 1;
				if (objname == "pb_cmdStop") typ.Machine_Cmd.cmdStop = 1;
				if (objname == "pb_cmdEStop")
				{
					typ.Machine_Cmd.cmdEStop = 1;//非点动按钮,点动需要到最下面setChecked(false)
					obj->setText(QString::fromLocal8Bit("急停\n按下！"));
				}
				if (objname == "pb_cmdJog")
				{
					typ.Machine_Cmd.cmdJog = 1;
					obj->setText(QString::fromLocal8Bit("运行..."));
				}
				if (objname == "pb_cmdErrorAck") typ.Machine_Cmd.cmdErrorAck = 1;					//报警复位, 1:复位
				if (objname == "pb_cmdResetCounter") typ.Machine_Cmd.cmdResetCounter = 1;				//复位计数变量, 1:复位
				if (objname == "pb_cmdParaSave") typ.Machine_Cmd.cmdParaSave = 1;						//参数保存命令, 1:保存
				if (objname == "pb_cmdParaLoad") typ.Machine_Cmd.cmdParaLoad = 1;						//参数读取命令, 1:读取
				if (objname == "pb_cmdTestFlash0") typ.Machine_Cmd.cmdTestFlash[0] = 1;			//手动闪光, 1:闪光,自动复位
				if (objname == "pb_cmdTestFlash1") typ.Machine_Cmd.cmdTestFlash[1] = 1;
				if (objname == "pb_cmdTestFlash2") typ.Machine_Cmd.cmdTestFlash[2] = 1;

				if (objname == "pb_cmdTestValveUp") typ.Machine_Cmd.cmdTestValveUp = 1;						//手动升降气缸, 1:Push, 2:Back
				if (objname == "pb_cmdTestValveClip") typ.Machine_Cmd.cmdTestValveClip = 1;					//手动夹紧气缸, 1:Push, 2:Back
				if (objname == "pb_cmdTestValveDrop") typ.Machine_Cmd.cmdTestValveDrop = 1;					//手动落囊气缸, 1:Push, 2:Back
				if (objname == "pb_cmdTestInverter") typ.Machine_Cmd.cmdTestInverter = 1;				//手动胶囊料斗启动, 1:Start, 2:Stop
				if (objname == "pb_cmdTestLampRead") typ.Machine_Cmd.cmdTestLampRead = 1;				//手动红灯输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestLampYellow") typ.Machine_Cmd.cmdTestLampYellow = 1;					//手动黄灯输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestLampGreen") typ.Machine_Cmd.cmdTestLampGreen = 1;					//手动绿灯输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestBuzzer") typ.Machine_Cmd.cmdTestBuzzer = 1;						//手动蜂鸣器输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestPhoto") typ.Machine_Cmd.cmdTestPhoto = 1;						//手动拍照, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestFlashPhoto") typ.Machine_Cmd.cmdTestFlashPhoto = 1;					//手动闪光加拍照, 1:启动
				if (objname == "pb_cmdTestCapPhoto") typ.Machine_Cmd.cmdTestCapPhoto = 1;				//手动胶囊拍照
				if (objname == "pb_cmdRotateCtl") typ.Machine_Cmd.cmdRotateCtl = 1;						//手动转囊启停

				obj->setStyleSheet("background: rgb(0,255,0)");
			}
		}
		if (!b)
		{
			if (objname == "pb_StWritePLC")
			{
				ui.pb_StWritePLC->setStyleSheet("");
				ui.pb_StWritePLC->setFont(font);
			}
			else if (objname == "pB_enPhoto" || objname == "pB_enReject" || objname == "pB_enFeed" || objname == "pB_enRotate")
			{
				typ.Telegram_typ = 4;//运行报文
				if (objname == "pB_enPhoto")
				{
					typ.Run_Para.enPhoto = 0;
				}
				if (objname == "pB_enReject")
				{
					typ.Run_Para.enReject = 0;
				}
				if (objname == "pB_enFeed")
				{
					typ.Run_Para.enFeed = 0;
				}
				if (objname == "pB_enRotate")
				{
					typ.Run_Para.enRotate = 0;
				}
				obj->setStyleSheet("font-size:20pt");
				g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收
			}
			else {
				if (objname.contains("pb_cmdTestKick"))
				{
					int index_cmdkick = objname.mid(14).toInt();//从0开始，14位之后的字符转为int，0-29
					typ.Machine_Cmd.cmdTestKick[index_cmdkick] = 0;//结构体数据赋值
				}
				if (objname == "pb_cmdHome")typ.Machine_Cmd.cmdHome = 0;
				if (objname == "pb_cmdStart") typ.Machine_Cmd.cmdStart = 0;
				if (objname == "pb_cmdStop") typ.Machine_Cmd.cmdStop = 0;
				if (objname == "pb_cmdEStop")
				{
					typ.Machine_Cmd.cmdEStop = 0;//非点动按钮
					obj->setText(QString::fromLocal8Bit("急停"));
				}
				if (objname == "pb_cmdJog")
				{

					typ.Machine_Cmd.cmdJog = 0; //非点动按钮
					obj->setText(QString::fromLocal8Bit("点动"));
				}
				if (objname == "pb_cmdErrorAck") typ.Machine_Cmd.cmdErrorAck = 0;					//报警复位, 1:复位
				if (objname == "pb_cmdResetCounter") typ.Machine_Cmd.cmdResetCounter = 0;				//复位计数变量, 1:复位
				//if (objname == "pb_cmdParaSave") typ.Machine_Cmd.cmdParaSave = 0;						//参数保存命令, 1:保存
				//if (objname == "pb_cmdParaLoad") typ.Machine_Cmd.cmdParaLoad = 0;						//参数读取命令, 1:读取
				if (objname == "pb_cmdTestFlash0") typ.Machine_Cmd.cmdTestFlash[0] = 0;			//手动闪光, 1:闪光,自动复位
				if (objname == "pb_cmdTestFlash1") typ.Machine_Cmd.cmdTestFlash[1] = 0;
				if (objname == "pb_cmdTestFlash2") typ.Machine_Cmd.cmdTestFlash[2] = 0;

				if (objname == "pb_cmdTestValveUp") typ.Machine_Cmd.cmdTestValveUp = 2;						//手动升降气缸, 1:Push, 2:Back
				if (objname == "pb_cmdTestValveClip") typ.Machine_Cmd.cmdTestValveClip = 2;					//手动夹紧气缸, 1:Push, 2:Back
				if (objname == "pb_cmdTestValveDrop") typ.Machine_Cmd.cmdTestValveDrop = 2;					//手动落囊气缸, 1:Push, 2:Back
				if (objname == "pb_cmdTestInverter") typ.Machine_Cmd.cmdTestInverter = 2;				//手动胶囊料斗启动, 1:Start, 2:Stop
				if (objname == "pb_cmdTestLampRead") typ.Machine_Cmd.cmdTestLampRead = 2;				//手动红灯输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestLampYellow") typ.Machine_Cmd.cmdTestLampYellow = 2;					//手动黄灯输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestLampGreen") typ.Machine_Cmd.cmdTestLampGreen = 2;					//手动绿灯输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestBuzzer") typ.Machine_Cmd.cmdTestBuzzer = 2;						//手动蜂鸣器输出, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestPhoto") typ.Machine_Cmd.cmdTestPhoto = 2;						//手动拍照, 1:输出 , 2: 复位
				if (objname == "pb_cmdTestFlashPhoto") typ.Machine_Cmd.cmdTestFlashPhoto = 0;					//手动闪光加拍照, 1:启动
				if (objname == "pb_cmdTestCapPhoto") typ.Machine_Cmd.cmdTestCapPhoto = 0;				//手动胶囊拍照
				if (objname == "pb_cmdRotateCtl") typ.Machine_Cmd.cmdRotateCtl = 2;						//手动转囊启停
				obj->setStyleSheet("");
			}
		}
		if (objname != "pb_StWritePLC")//写入debug两次发送/旨在上面发送一次
		{
			g_SocketPLC->Communicate_PLC(&typ, nullptr);//此处只发送，不接收
		}//参数报文


		if (objname == "pb_StWritePLC" /*|| objname.contains("pb_cmdTestKick")*/ || objname == "pb_cmdHome" || objname == "pb_cmdStart"
			|| objname == "pb_cmdStop" || objname == "pb_cmdErrorAck"
			|| objname == "pb_cmdResetCounter" || objname == "pb_cmdParaSave" || objname == "pb_cmdParaLoad"
			//|| objname == "pb_cmdTestFlash0" || objname == "pb_cmdTestFlash1" || objname == "pb_cmdTestFlash2"
			//|| objname == "pb_cmdTestValveUp" || objname == "pb_cmdTestValveClip" || objname == "pb_cmdTestValveDrop"
			//|| objname == "pb_cmdTestInverter" || objname == "pb_cmdTestLampRead" || objname == "pb_cmdTestLampYellow"
			//|| objname == "pb_cmdTestLampGreen" || objname == "pb_cmdTestBuzzer"  || objname == "pb_cmdRotateCtl"
			|| objname == "pb_cmdTestPhoto" || objname == "pb_cmdTestFlashPhoto" || objname == "pb_cmdTestCapPhoto")
		{
			obj->setChecked(false);
		}

	}
	MoveOutWhenWrite();


}
//获取pc数据
DataFromPC_typ QtCameraSet::getPCData()
{

	DataFromPC_typ tmp;
	memset(&tmp, 0, sizeof(DataFromPC_typ));//将新char所指向的前size字节的内存单元用一个0替换，初始化内存。下同
	tmp.Dummy = 0;
	//tmp.Telegram_typ = ui.lE_Telegram_typ->text().toInt();

	//系统参数
	tmp.Machine_Para.FeedAxisHomeOffset = QString::number(ui.lE_FeedAxisHomeOffset->text().toFloat() * 100).toInt();//qstring-float-qstring-int
	tmp.Machine_Para.ClipPhase1 = QString::number(ui.lE_ClipPhase1->text().toFloat() * 100).toInt();
	tmp.Machine_Para.ClipPhase2 = QString::number(ui.lE_ClipPhase2->text().toFloat() * 100).toInt();
	tmp.Machine_Para.UpPhase1 = QString::number(ui.lE_UpPhase1->text().toFloat() * 100).toInt();
	tmp.Machine_Para.UpPhase2 = QString::number(ui.lE_UpPhase2->text().toFloat() * 100).toInt();
	tmp.Machine_Para.DropPhase1 = QString::number(ui.lE_DropPhase1->text().toFloat() * 100).toInt();
	tmp.Machine_Para.DropPhase2 = QString::number(ui.lE_DropPhase2->text().toFloat() * 100).toInt();
	tmp.Machine_Para.tClip1 = ui.lE_tClip1->text().toFloat();
	tmp.Machine_Para.tClip2 = ui.lE_tClip2->text().toFloat();
	tmp.Machine_Para.tUp1 = ui.lE_tUp1->text().toFloat();
	tmp.Machine_Para.tUp2 = ui.lE_tUp2->text().toFloat();
	tmp.Machine_Para.tDrop1 = ui.lE_tDrop1->text().toFloat();
	tmp.Machine_Para.tDrop2 = ui.lE_tDrop2->text().toFloat();
	tmp.Machine_Para.FeedLength = QString::number(ui.lE_FeedLength->text().toFloat() * 100).toInt();
	tmp.Machine_Para.FlashTime = ui.lE_FlashTime->text().toFloat();
	tmp.Machine_Para.PhotoTime = ui.lE_PhotoTime->text().toFloat();
	tmp.Machine_Para.RejectTime = ui.lE_RejectTime->text().toFloat();
	tmp.Machine_Para.PhotoDelay = ui.lE_PhotoDelay->text().toFloat();
	tmp.Machine_Para.PhotoPhase = QString::number(ui.lE_PhotoPhase->text().toFloat() * 100).toInt();
	tmp.Machine_Para.RejectPhase = QString::number(ui.lE_RejectPhase->text().toFloat() * 100).toInt();
	tmp.Machine_Para.PhotoTimes = ui.lE_PhotoTimes->text().toInt();
	tmp.Machine_Para.RotateSpeed = ui.lE_RotateSpeed->text().toFloat();
	tmp.Machine_Para.DisableForceReject= ui.lE_DisableForceReject->text().toInt();		//关闭强制剔废,1:关闭
	tmp.Machine_Para.CapCheckAlarmTime = ui.lE_CapCheckAlarmTime->text().toInt();		//胶囊检测报警时间，单位ms
	tmp.Machine_Para.RejectFallingTime = ui.lE_RejectFallingTime->text().toInt();		//剔废胶囊下落时间，单位ms
	
																						//运行数据
	tmp.Run_Para.RunSpeed = ui.lE_RunSpeed->text().toInt();
	tmp.Run_Para.SysPhase = ui.lE_SysPhase->text().toInt();
	tmp.Run_Para.enPhoto = ui.pB_enPhoto->isChecked() ? 1 : 0;
	tmp.Run_Para.enReject = ui.pB_enReject->isChecked() ? 1 : 0;
	tmp.Run_Para.enFeed = ui.pB_enFeed->isChecked() ? 1 : 0;
	tmp.Run_Para.enRotate = ui.pB_enRotate->isChecked() ? 1 : 0;
	tmp.Run_Para.CheckCount = m_data->ActData.CheckCount;
	tmp.Run_Para.RejectCount = m_data->ActData.RejectCount;
	tmp.Run_Para.ForceRejectCount = m_data->ActData.ForceRejectCount;

	//命令
	//tmp.Machine_Cmd.cmdHome = ui.pb_cmdHome->isChecked() ? 1 : 0;//寻参,1:寻参启动
	tmp.Machine_Cmd.cmdStart = 0;							//启动,1:启动运行
	tmp.Machine_Cmd.cmdStop = 0;							//停止,停在0相位,1:停止
 	tmp.Machine_Cmd.cmdEStop = 0;							//紧急停止(立即停止), 1:停止
	tmp.Machine_Cmd.cmdJog = ui.pb_cmdJog->isChecked() ? 1 : 0;							//点动运行, 1,启动,2,停止
	tmp.Machine_Cmd.cmdErrorAck = 0;						//报警复位, 1:复位
	tmp.Machine_Cmd.cmdResetCounter = 0;					//复位计数变量, 1:复位
	//tmp.Machine_Cmd.cmdParaSave = ui.pb_cmdParaSave->isChecked() ? 1 : 0;						//参数保存命令, 1:保存
	//tmp.Machine_Cmd.cmdParaLoad = ui.pb_cmdParaLoad->isChecked() ? 1 : 0;						//参数读取命令, 1:读取
	tmp.Machine_Cmd.cmdTestKick[0] = ui.pb_cmdTestKick0->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[1] = ui.pb_cmdTestKick1->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[2] = ui.pb_cmdTestKick2->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[3] = ui.pb_cmdTestKick3->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back	
	tmp.Machine_Cmd.cmdTestKick[4] = ui.pb_cmdTestKick4->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[5] = ui.pb_cmdTestKick5->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[6] = ui.pb_cmdTestKick6->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[7] = ui.pb_cmdTestKick7->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[8] = ui.pb_cmdTestKick8->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[9] = ui.pb_cmdTestKick9->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[10] = ui.pb_cmdTestKick10->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[11] = ui.pb_cmdTestKick11->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[12] = ui.pb_cmdTestKick12->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[13] = ui.pb_cmdTestKick13->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back	
	tmp.Machine_Cmd.cmdTestKick[14] = ui.pb_cmdTestKick14->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[15] = ui.pb_cmdTestKick15->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[16] = ui.pb_cmdTestKick16->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[17] = ui.pb_cmdTestKick17->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[18] = ui.pb_cmdTestKick18->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[19] = ui.pb_cmdTestKick19->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[20] = ui.pb_cmdTestKick20->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[21] = ui.pb_cmdTestKick21->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[22] = ui.pb_cmdTestKick22->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[23] = ui.pb_cmdTestKick23->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back	
	tmp.Machine_Cmd.cmdTestKick[24] = ui.pb_cmdTestKick24->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[25] = ui.pb_cmdTestKick25->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[26] = ui.pb_cmdTestKick26->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[27] = ui.pb_cmdTestKick27->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[28] = ui.pb_cmdTestKick28->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestKick[29] = ui.pb_cmdTestKick29->isChecked() ? 1 : 2;		//手动剔废, 1:Push, 2:Back
	
	tmp.Machine_Cmd.cmdTestFlash[0] = ui.pb_cmdTestFlash0->isChecked() ? 1 : 0;			//手动闪光, 1:闪光,自动复位
	tmp.Machine_Cmd.cmdTestFlash[1] = ui.pb_cmdTestFlash1->isChecked() ? 1 : 0;			//手动闪光, 1:闪光,自动复位
	tmp.Machine_Cmd.cmdTestFlash[2] = ui.pb_cmdTestFlash2->isChecked() ? 1 : 0;			//手动闪光, 1:闪光,自动复位

	tmp.Machine_Cmd.cmdTestValveUp = ui.pb_cmdTestValveUp->isChecked() ? 1 : 2;						//手动升降气缸, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestValveClip = ui.pb_cmdTestValveClip->isChecked() ? 1 : 2;					//手动夹紧气缸, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestValveDrop = ui.pb_cmdTestValveDrop->isChecked() ? 1 : 2;					//手动落囊气缸, 1:Push, 2:Back
	tmp.Machine_Cmd.cmdTestInverter = ui.pb_cmdTestInverter->isChecked() ? 1 : 0;					//手动胶囊料斗启动, 1:Start, 2:Stop
	tmp.Machine_Cmd.cmdTestLampRead = ui.pb_cmdTestLampRead->isChecked() ? 1 : 2;					//手动红灯输出, 1:输出 , 2: 复位
	tmp.Machine_Cmd.cmdTestLampYellow = ui.pb_cmdTestLampYellow->isChecked() ? 1 : 2;					//手动黄灯输出, 1:输出 , 2: 复位
	tmp.Machine_Cmd.cmdTestLampGreen = ui.pb_cmdTestLampGreen->isChecked() ? 1 : 2;					//手动绿灯输出, 1:输出 , 2: 复位
	tmp.Machine_Cmd.cmdTestBuzzer = ui.pb_cmdTestBuzzer->isChecked() ? 1 : 2;						//手动蜂鸣器输出, 1:输出 , 2: 复位
	tmp.Machine_Cmd.cmdTestPhoto = ui.pb_cmdTestPhoto->isChecked() ? 1 : 2;						//手动拍照, 1:输出 , 2: 复位
	tmp.Machine_Cmd.cmdTestFlashPhoto = ui.pb_cmdTestFlashPhoto->isChecked() ? 1 : 0;					//手动闪光加拍照, 1:启动
	tmp.Machine_Cmd.cmdTestCapPhoto = ui.pb_cmdTestCapPhoto->isChecked() ? 1 : 0;					//手动胶囊拍照
	tmp.Machine_Cmd.cmdRotateCtl = ui.pb_cmdRotateCtl->isChecked() ? 1 : 0;						//手动转囊启停
	tmp.Machine_Cmd.cmdSetAlarm = 0;								//手动触发报警号，不等于零触发。范围1-99

	//照相处理结果和按钮在上面⬆⬆⬆⬆⬆⬆

	return tmp;
}
//PC显示数据
void QtCameraSet::getPLCData(DataToPC_typ data)
{
	memcpy(m_data, &data, sizeof(DataToPC_typ));//主界面用

	//报文类型
	//ui.lE_Telegram_typ->setText(QString::number(data.Telegram_typ));

	//运行数据
	if (!ui.lE_RunSpeed->hasFocus())
	{
		ui.lE_RunSpeed->setText(QString::number(data.ActData.RunSpeed));
	}
	ui.lE_SysPhase->setText(QString::number(data.ActData.SysPhase / 100.0, 'f', 2));
	if (data.ActData.enPhoto == 1)
	{
		ui.pB_enPhoto->blockSignals(true);
		ui.pB_enPhoto->setChecked(true);
		ui.pB_enPhoto->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pB_enPhoto->blockSignals(false);

	}
	else
	{
		ui.pB_enPhoto->blockSignals(true);
		ui.pB_enPhoto->setChecked(false);
		ui.pB_enPhoto->setStyleSheet("font-size:20pt");

		ui.pB_enPhoto->blockSignals(false);
	}
	if (data.ActData.enReject == 1)

	{
		ui.pB_enReject->blockSignals(true);
		ui.pB_enReject->setChecked(true);
		ui.pB_enReject->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pB_enReject->blockSignals(false);
	}
	else
	{
		ui.pB_enReject->blockSignals(true);
		ui.pB_enReject->setChecked(false);
		ui.pB_enReject->setStyleSheet("font-size:20pt");
		ui.pB_enReject->blockSignals(false);
	}

	if (data.ActData.enFeed == 1)
	{
		ui.pB_enFeed->blockSignals(true);
		ui.pB_enFeed->setChecked(true);
		ui.pB_enFeed->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pB_enFeed->blockSignals(false);

	}
	else
	{
		ui.pB_enFeed->blockSignals(true);
		ui.pB_enFeed->setChecked(false);
		ui.pB_enFeed->setStyleSheet("font-size:20pt");

		ui.pB_enFeed->blockSignals(false);
	}
	if (data.ActData.enRotate == 1)

	{
		ui.pB_enRotate->blockSignals(true);
		ui.pB_enRotate->setChecked(true);
		ui.pB_enRotate->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pB_enRotate->blockSignals(false);
	}
	else
	{
		ui.pB_enRotate->blockSignals(true);
		ui.pB_enRotate->setChecked(false);
		ui.pB_enRotate->setStyleSheet("font-size:20pt");
		ui.pB_enRotate->blockSignals(false);
	}
	
	//ui.lE_CheckCount->setText(QString::number(data.ActData.CheckCount));
	//ui.lE_RejectCount->setText(QString::number(data.ActData.RejectCount));
	//ui.lE_ForceRejectCount->setText(QString::number(data.ActData.ForceRejectCount));

	//系统状态
	ui.lE_AlarmStatus->setText(QString::number(data.Status.AlarmStatus));
	ui.lE_Alarm16->setText((char*)(data.Status.Alarm));
	ui.lE_ServoErrorNum0->setText(QString::number(data.Status.ServoErrorNum[0]));
	ui.lE_ServoErrorNum1->setText(QString::number(data.Status.ServoErrorNum[1]));
	ui.lE_SysPhase_1->setText(QString::number(data.Status.SysPhase / 100.0, 'f', 2));
	ui.lE_HomeOK->setText(QString::number(data.Status.HomeOK));

	//系统参数
	if (!ui.lE_FeedAxisHomeOffset->hasFocus())
	{
		ui.lE_FeedAxisHomeOffset->setText(QString::number(data.Machine_Para.FeedAxisHomeOffset / 100.0, 'f', 2));//显示到小数点后两位
	}
	if (!ui.lE_ClipPhase1->hasFocus())
	{
		ui.lE_ClipPhase1->setText(QString::number(data.Machine_Para.ClipPhase1 / 100.0, 'f', 2));
	}
	if (!ui.lE_ClipPhase2->hasFocus())
	{
	ui.lE_ClipPhase2->setText(QString::number(data.Machine_Para.ClipPhase2 / 100.0, 'f', 2));
	}
	if (!ui.lE_UpPhase1->hasFocus())
	{
	ui.lE_UpPhase1->setText(QString::number(data.Machine_Para.UpPhase1 / 100.0, 'f', 2));
	}
	if (!ui.lE_UpPhase2->hasFocus())
	{
	ui.lE_UpPhase2->setText(QString::number(data.Machine_Para.UpPhase2 / 100.0, 'f', 2));
	}
	if (!ui.lE_DropPhase1->hasFocus())
	{
	ui.lE_DropPhase1->setText(QString::number(data.Machine_Para.DropPhase1 / 100.0, 'f', 2));
	}
	if (!ui.lE_DropPhase2->hasFocus())
	{
	ui.lE_DropPhase2->setText(QString::number(data.Machine_Para.DropPhase2 / 100.0, 'f', 2));
	}
	if (!ui.lE_tClip1->hasFocus())
	{
	ui.lE_tClip1->setText(QString::number(data.Machine_Para.tClip1));
	}
	if (!ui.lE_tClip2->hasFocus())
	{
	ui.lE_tClip2->setText(QString::number(data.Machine_Para.tClip2));
	}
	if (!ui.lE_tUp1->hasFocus())
	{
	ui.lE_tUp1->setText(QString::number(data.Machine_Para.tUp1));
	}
	if (!ui.lE_tUp2->hasFocus())
	{
	ui.lE_tUp2->setText(QString::number(data.Machine_Para.tUp2));
	}
	if (!ui.lE_tDrop1->hasFocus())
	{
	ui.lE_tDrop1->setText(QString::number(data.Machine_Para.tDrop1));
	}
	if (!ui.lE_tDrop2->hasFocus())
	{
	ui.lE_tDrop2->setText(QString::number(data.Machine_Para.tDrop2));
	}
	if (!ui.lE_FeedLength->hasFocus())
	{
	ui.lE_FeedLength->setText(QString::number(data.Machine_Para.FeedLength / 100.0, 'f', 2));
	}
	if (!ui.lE_FlashTime->hasFocus())
	{
	ui.lE_FlashTime->setText(QString::number(data.Machine_Para.FlashTime));
	}
	if (!ui.lE_PhotoTime->hasFocus())
	{
	ui.lE_PhotoTime->setText(QString::number(data.Machine_Para.PhotoTime));
	}
	if (!ui.lE_RejectTime->hasFocus())
	{
	ui.lE_RejectTime->setText(QString::number(data.Machine_Para.RejectTime));
	}
	if (!ui.lE_PhotoDelay->hasFocus())
	{
	ui.lE_PhotoDelay->setText(QString::number(data.Machine_Para.PhotoDelay));
	}
	if (!ui.lE_PhotoPhase->hasFocus())
	{
	ui.lE_PhotoPhase->setText(QString::number(data.Machine_Para.PhotoPhase / 100.0, 'f', 2));
	}
	if (!ui.lE_RejectPhase->hasFocus())
	{
	ui.lE_RejectPhase->setText(QString::number(data.Machine_Para.RejectPhase / 100.0, 'f', 2));
	}
	if (!ui.lE_PhotoTimes->hasFocus())
	{
	ui.lE_PhotoTimes->setText(QString::number(data.Machine_Para.PhotoTimes));
	}
	if (!ui.lE_RotateSpeed->hasFocus())
	{
	ui.lE_RotateSpeed->setText(QString::number(data.Machine_Para.RotateSpeed));
	}
	if (!ui.lE_DisableForceReject->hasFocus())
	{
		ui.lE_DisableForceReject->setText(QString::number(data.Machine_Para.DisableForceReject));
	}
	if (!ui.lE_CapCheckAlarmTime->hasFocus())
	{
		ui.lE_CapCheckAlarmTime->setText(QString::number(data.Machine_Para.CapCheckAlarmTime));
	}
	if (!ui.lE_RejectFallingTime->hasFocus())
	{
		ui.lE_RejectFallingTime->setText(QString::number(data.Machine_Para.RejectFallingTime));
	}


	//输入点
	//ui.lE_EStop->setText(data.Inputs.EStop ? "1" : "0");
	ui.lb_00->setVisible(data.Inputs.EStop ? false : true);
	//ui.lE_AxisFeedPosEnd->setText(data.Inputs.AxisFeedPosEnd ? "1" : "0");
	ui.lb_10->setVisible(data.Inputs.AxisFeedPosEnd ? false : true);
	//ui.lE_AxisFeedNegEnd->setText(data.Inputs.AxisFeedNegEnd ? "1" : "0");
	ui.lb_20->setVisible(data.Inputs.AxisFeedNegEnd ? false : true);
	//ui.lE_AxisFeedHome->setText(data.Inputs.AxisFeedHome ? "1" : "0");
	ui.lb_30->setVisible(data.Inputs.AxisFeedHome ? false : true);
	//ui.lE_AirPressChk->setText(data.Inputs.AirPressChk ? "1" : "0");
	ui.lb_40->setVisible(data.Inputs.AirPressChk ? false : true);
	
	//输出点
	//ui.lE_Inveter->setText(data.Outputs.Inveter ? "1" : "0");	
	if (data.Outputs.Inveter)
	{
		ui.pb_cmdTestInverter->blockSignals(true);
		ui.pb_cmdTestInverter->setChecked(true);
		ui.pb_cmdTestInverter->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestInverter->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestInverter->blockSignals(true);
		ui.pb_cmdTestInverter->setChecked(false);
		ui.pb_cmdTestInverter->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestInverter->blockSignals(false);
	}
	//ui.lE_ClipValve->setText(data.Outputs.ClipValve ? "1" : "0");
	if (data.Outputs.ClipValve)
	{
		ui.pb_cmdTestValveClip->blockSignals(true);
		ui.pb_cmdTestValveClip->setChecked(true);
		ui.pb_cmdTestValveClip->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestValveClip->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestValveClip->blockSignals(true);
		ui.pb_cmdTestValveClip->setChecked(false);
		ui.pb_cmdTestValveClip->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestValveClip->blockSignals(false);
	}
	//ui.lE_UpValve->setText(data.Outputs.UpValve ? "1" : "0");
	if (data.Outputs.UpValve)
	{
		ui.pb_cmdTestValveUp->blockSignals(true);
		ui.pb_cmdTestValveUp->setChecked(true);
		ui.pb_cmdTestValveUp->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestValveUp->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestValveUp->blockSignals(true);
		ui.pb_cmdTestValveUp->setChecked(false);
		ui.pb_cmdTestValveUp->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestValveUp->blockSignals(false);
	}
	//ui.lE_DropValve->setText(data.Outputs.DropValve ? "1" : "0");
	if (data.Outputs.DropValve)
	{
		ui.pb_cmdTestValveDrop->blockSignals(true);
		ui.pb_cmdTestValveDrop->setChecked(true);
		ui.pb_cmdTestValveDrop->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestValveDrop->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestValveDrop->blockSignals(true);
		ui.pb_cmdTestValveDrop->setChecked(false);
		ui.pb_cmdTestValveDrop->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestValveDrop->blockSignals(false);
	}
	//ui.lE_LampRed->setText(data.Outputs.LampRed ? "1" : "0");
	if (data.Outputs.LampRed)
	{
		ui.pb_cmdTestLampRead->blockSignals(true);
		ui.pb_cmdTestLampRead->setChecked(true);
		ui.pb_cmdTestLampRead->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestLampRead->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestLampRead->blockSignals(true);
		ui.pb_cmdTestLampRead->setChecked(false);
		ui.pb_cmdTestLampRead->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestLampRead->blockSignals(false);
	}
	//ui.lE_LampYellow->setText(data.Outputs.LampYellow ? "1" : "0");
	if (data.Outputs.LampYellow)
	{
		ui.pb_cmdTestLampYellow->blockSignals(true);
		ui.pb_cmdTestLampYellow->setChecked(true);
		ui.pb_cmdTestLampYellow->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestLampYellow->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestLampYellow->blockSignals(true);
		ui.pb_cmdTestLampYellow->setChecked(false);
		ui.pb_cmdTestLampYellow->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestLampYellow->blockSignals(false);
	}
		//ui.lE_LampGreen->setText(data.Outputs.LampGreen ? "1" : "0");
		if (data.Outputs.LampGreen)
		{
			ui.pb_cmdTestLampGreen->blockSignals(true);
			ui.pb_cmdTestLampGreen->setChecked(true);
			ui.pb_cmdTestLampGreen->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
			ui.pb_cmdTestLampGreen->blockSignals(false);
		}
		else
		{
			ui.pb_cmdTestLampGreen->blockSignals(true);
			ui.pb_cmdTestLampGreen->setChecked(false);
			ui.pb_cmdTestLampGreen->setStyleSheet("font-size:20pt");
			ui.pb_cmdTestLampGreen->blockSignals(false);
		}
	//ui.lE_Buzzer->setText(data.Outputs.Buzzer ? "1" : "0");
	if (data.Outputs.Buzzer)
	{
		ui.pb_cmdTestBuzzer->blockSignals(true);
		ui.pb_cmdTestBuzzer->setChecked(true);
		ui.pb_cmdTestBuzzer->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestBuzzer->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestBuzzer->blockSignals(true);
		ui.pb_cmdTestBuzzer->setChecked(false);
		ui.pb_cmdTestBuzzer->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestBuzzer->blockSignals(false);
	}
	//ui.lE_Photo->setText(data.Outputs.Photo ? "1" : "0");
	if (data.Outputs.Photo)
	{
		ui.pb_cmdTestPhoto->blockSignals(true);
		ui.pb_cmdTestPhoto->setChecked(true);
		ui.pb_cmdTestPhoto->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestPhoto->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestPhoto->blockSignals(true);
		ui.pb_cmdTestPhoto->setChecked(false);
		ui.pb_cmdTestPhoto->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestPhoto->blockSignals(false);
	}

	//ui.lE_Flash0->setText(data.Outputs.Flash[0] ? "1" : "0");
	if (data.Outputs.Flash[0])
	{
		ui.pb_cmdTestFlash0->blockSignals(true);
		ui.pb_cmdTestFlash0->setChecked(true);
		ui.pb_cmdTestFlash0->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestFlash0->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestFlash0->blockSignals(true);
		ui.pb_cmdTestFlash0->setChecked(false);
		ui.pb_cmdTestFlash0->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestFlash0->blockSignals(false);
	}
	//ui.lE_Flash1->setText(data.Outputs.Flash[1] ? "1" : "0");
	if (data.Outputs.Flash[1])
	{
		ui.pb_cmdTestFlash1->blockSignals(true);
		ui.pb_cmdTestFlash1->setChecked(true);
		ui.pb_cmdTestFlash1->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestFlash1->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestFlash1->blockSignals(true);
		ui.pb_cmdTestFlash1->setChecked(false);
		ui.pb_cmdTestFlash1->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestFlash1->blockSignals(false);
	}
	//ui.lE_Flash2->setText(data.Outputs.Flash[2] ? "1" : "0");	
	if (data.Outputs.Flash[2])
	{
		ui.pb_cmdTestFlash2->blockSignals(true);
		ui.pb_cmdTestFlash2->setChecked(true);
		ui.pb_cmdTestFlash2->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestFlash2->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestFlash2->blockSignals(true);
		ui.pb_cmdTestFlash2->setChecked(false);
		ui.pb_cmdTestFlash2->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestFlash2->blockSignals(false);
	}

	/*ui.lE_Reject0->setText(data.Outputs.Reject[0] ? "1" : "0");*/
	if (data.Outputs.Reject[0])
	{
		ui.pb_cmdTestKick0->blockSignals(true);
		ui.pb_cmdTestKick0->setChecked(true);
		ui.pb_cmdTestKick0->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick0->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick0->blockSignals(true);
		ui.pb_cmdTestKick0->setChecked(false);
		ui.pb_cmdTestKick0->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick0->blockSignals(false);
	}
	//ui.lE_Reject1->setText(data.Outputs.Reject[1] ? "1" : "0");	
	if (data.Outputs.Reject[1])
	{
		ui.pb_cmdTestKick1->blockSignals(true);
		ui.pb_cmdTestKick1->setChecked(true);
		ui.pb_cmdTestKick1->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick1->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick1->blockSignals(true);
		ui.pb_cmdTestKick1->setChecked(false);
		ui.pb_cmdTestKick1->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick1->blockSignals(false);
	}
	//ui.lE_Reject2->setText(data.Outputs.Reject[2] ? "1" : "0");
	if (data.Outputs.Reject[2])
	{
		ui.pb_cmdTestKick2->blockSignals(true);
		ui.pb_cmdTestKick2->setChecked(true);
		ui.pb_cmdTestKick2->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick2->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick2->blockSignals(true);
		ui.pb_cmdTestKick2->setChecked(false);
		ui.pb_cmdTestKick2->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick2->blockSignals(false);
	}
	//ui.lE_Reject3->setText(data.Outputs.Reject[3] ? "1" : "0");
	if (data.Outputs.Reject[3])
	{
		ui.pb_cmdTestKick3->blockSignals(true);
		ui.pb_cmdTestKick3->setChecked(true);
		ui.pb_cmdTestKick3->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick3->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick3->blockSignals(true);
		ui.pb_cmdTestKick3->setChecked(false);
		ui.pb_cmdTestKick3->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick3->blockSignals(false);
	}
	//ui.lE_Reject4->setText(data.Outputs.Reject[4] ? "1" : "0");
	if (data.Outputs.Reject[4])
	{
		ui.pb_cmdTestKick4->blockSignals(true);
		ui.pb_cmdTestKick4->setChecked(true);
		ui.pb_cmdTestKick4->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick4->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick4->blockSignals(true);
		ui.pb_cmdTestKick4->setChecked(false);
		ui.pb_cmdTestKick4->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick4->blockSignals(false);
	}
	//ui.lE_Reject5->setText(data.Outputs.Reject[5] ? "1" : "0");
	if (data.Outputs.Reject[5])
	{
		ui.pb_cmdTestKick5->blockSignals(true);
		ui.pb_cmdTestKick5->setChecked(true);
		ui.pb_cmdTestKick5->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick5->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick5->blockSignals(true);
		ui.pb_cmdTestKick5->setChecked(false);
		ui.pb_cmdTestKick5->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick5->blockSignals(false);
	}
	//ui.lE_Reject6->setText(data.Outputs.Reject[6] ? "1" : "0");
	if (data.Outputs.Reject[6])
	{
		ui.pb_cmdTestKick6->blockSignals(true);
		ui.pb_cmdTestKick6->setChecked(true);
		ui.pb_cmdTestKick6->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick6->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick6->blockSignals(true);
		ui.pb_cmdTestKick6->setChecked(false);
		ui.pb_cmdTestKick6->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick6->blockSignals(false);
	}
	//ui.lE_Reject7->setText(data.Outputs.Reject[7] ? "1" : "0");
	if (data.Outputs.Reject[7])
	{
		ui.pb_cmdTestKick7->blockSignals(true);
		ui.pb_cmdTestKick7->setChecked(true);
		ui.pb_cmdTestKick7->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick7->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick7->blockSignals(true);
		ui.pb_cmdTestKick7->setChecked(false);
		ui.pb_cmdTestKick7->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick7->blockSignals(false);
	}
	//ui.lE_Reject8->setText(data.Outputs.Reject[8] ? "1" : "0");
	if (data.Outputs.Reject[8])
	{
		ui.pb_cmdTestKick8->blockSignals(true);
		ui.pb_cmdTestKick8->setChecked(true);
		ui.pb_cmdTestKick8->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick8->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick8->blockSignals(true);
		ui.pb_cmdTestKick8->setChecked(false);
		ui.pb_cmdTestKick8->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick8->blockSignals(false);
	}
	//ui.lE_Reject9->setText(data.Outputs.Reject[9] ? "1" : "0");
	if (data.Outputs.Reject[9])
	{
		ui.pb_cmdTestKick9->blockSignals(true);
		ui.pb_cmdTestKick9->setChecked(true);
		ui.pb_cmdTestKick9->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick9->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick9->blockSignals(true);
		ui.pb_cmdTestKick9->setChecked(false);
		ui.pb_cmdTestKick9->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick9->blockSignals(false);
	}
	//ui.lE_Reject10->setText(data.Outputs.Reject[10] ? "1" : "0");
	if (data.Outputs.Reject[10])
	{
		ui.pb_cmdTestKick10->blockSignals(true);
		ui.pb_cmdTestKick10->setChecked(true);
		ui.pb_cmdTestKick10->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick10->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick10->blockSignals(true);
		ui.pb_cmdTestKick10->setChecked(false);
		ui.pb_cmdTestKick10->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick10->blockSignals(false);
	}
	//ui.lE_Reject11->setText(data.Outputs.Reject[11] ? "1" : "0");
	if (data.Outputs.Reject[11])
	{
		ui.pb_cmdTestKick11->blockSignals(true);
		ui.pb_cmdTestKick11->setChecked(true);
		ui.pb_cmdTestKick11->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick11->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick11->blockSignals(true);
		ui.pb_cmdTestKick11->setChecked(false);
		ui.pb_cmdTestKick11->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick11->blockSignals(false);
	}
	//ui.lE_Reject12->setText(data.Outputs.Reject[12] ? "1" : "0");
	if (data.Outputs.Reject[12])
	{
		ui.pb_cmdTestKick12->blockSignals(true);
		ui.pb_cmdTestKick12->setChecked(true);
		ui.pb_cmdTestKick12->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick12->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick12->blockSignals(true);
		ui.pb_cmdTestKick12->setChecked(false);
		ui.pb_cmdTestKick12->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick12->blockSignals(false);
	}
	//ui.lE_Reject13->setText(data.Outputs.Reject[13] ? "1" : "0");
	if (data.Outputs.Reject[13])
	{
		ui.pb_cmdTestKick13->blockSignals(true);
		ui.pb_cmdTestKick13->setChecked(true);
		ui.pb_cmdTestKick13->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick13->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick13->blockSignals(true);
		ui.pb_cmdTestKick13->setChecked(false);
		ui.pb_cmdTestKick13->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick13->blockSignals(false);
	}
	//ui.lE_Reject14->setText(data.Outputs.Reject[14] ? "1" : "0");
	if (data.Outputs.Reject[14])
	{
		ui.pb_cmdTestKick14->blockSignals(true);
		ui.pb_cmdTestKick14->setChecked(true);
		ui.pb_cmdTestKick14->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick14->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick14->blockSignals(true);
		ui.pb_cmdTestKick14->setChecked(false);
		ui.pb_cmdTestKick14->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick14->blockSignals(false);
	}
	//ui.lE_Reject15->setText(data.Outputs.Reject[15] ? "1" : "0");
	if (data.Outputs.Reject[15])
	{
		ui.pb_cmdTestKick15->blockSignals(true);
		ui.pb_cmdTestKick15->setChecked(true);
		ui.pb_cmdTestKick15->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick15->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick15->blockSignals(true);
		ui.pb_cmdTestKick15->setChecked(false);
		ui.pb_cmdTestKick15->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick15->blockSignals(false);
	}
	//ui.lE_Reject16->setText(data.Outputs.Reject[16] ? "1" : "0");
	if (data.Outputs.Reject[16])
	{
		ui.pb_cmdTestKick16->blockSignals(true);
		ui.pb_cmdTestKick16->setChecked(true);
		ui.pb_cmdTestKick16->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick16->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick16->blockSignals(true);
		ui.pb_cmdTestKick16->setChecked(false);
		ui.pb_cmdTestKick16->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick16->blockSignals(false);
	}
	//ui.lE_Reject17->setText(data.Outputs.Reject[17] ? "1" : "0");
	if (data.Outputs.Reject[17])
	{
		ui.pb_cmdTestKick17->blockSignals(true);
		ui.pb_cmdTestKick17->setChecked(true);
		ui.pb_cmdTestKick17->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick17->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick17->blockSignals(true);
		ui.pb_cmdTestKick17->setChecked(false);
		ui.pb_cmdTestKick17->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick17->blockSignals(false);
	}
	//ui.lE_Reject18->setText(data.Outputs.Reject[18] ? "1" : "0");
	if (data.Outputs.Reject[18])
	{
		ui.pb_cmdTestKick18->blockSignals(true);
		ui.pb_cmdTestKick18->setChecked(true);
		ui.pb_cmdTestKick18->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick18->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick18->blockSignals(true);
		ui.pb_cmdTestKick18->setChecked(false);
		ui.pb_cmdTestKick18->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick18->blockSignals(false);
	}
	//ui.lE_Reject19->setText(data.Outputs.Reject[19] ? "1" : "0");
	if (data.Outputs.Reject[19])
	{
		ui.pb_cmdTestKick19->blockSignals(true);
		ui.pb_cmdTestKick19->setChecked(true);
		ui.pb_cmdTestKick19->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick19->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick19->blockSignals(true);
		ui.pb_cmdTestKick19->setChecked(false);
		ui.pb_cmdTestKick19->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick19->blockSignals(false);
	}
	//ui.lE_Reject20->setText(data.Outputs.Reject[20] ? "1" : "0");
	if (data.Outputs.Reject[20])
	{
		ui.pb_cmdTestKick20->blockSignals(true);
		ui.pb_cmdTestKick20->setChecked(true);
		ui.pb_cmdTestKick20->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick20->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick20->blockSignals(true);
		ui.pb_cmdTestKick20->setChecked(false);
		ui.pb_cmdTestKick20->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick20->blockSignals(false);
	}
	//ui.lE_Reject21->setText(data.Outputs.Reject[21] ? "1" : "0");
	if (data.Outputs.Reject[21])
	{
		ui.pb_cmdTestKick21->blockSignals(true);
		ui.pb_cmdTestKick21->setChecked(true);
		ui.pb_cmdTestKick21->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick21->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick21->blockSignals(true);
		ui.pb_cmdTestKick21->setChecked(false);
		ui.pb_cmdTestKick21->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick21->blockSignals(false);
	}
	//ui.lE_Reject22->setText(data.Outputs.Reject[22] ? "1" : "0");
	if (data.Outputs.Reject[22])
	{
		ui.pb_cmdTestKick22->blockSignals(true);
		ui.pb_cmdTestKick22->setChecked(true);
		ui.pb_cmdTestKick22->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick22->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick22->blockSignals(true);
		ui.pb_cmdTestKick22->setChecked(false);
		ui.pb_cmdTestKick22->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick22->blockSignals(false);
	}
	//ui.lE_Reject23->setText(data.Outputs.Reject[23] ? "1" : "0");
	if (data.Outputs.Reject[23])
	{
		ui.pb_cmdTestKick23->blockSignals(true);
		ui.pb_cmdTestKick23->setChecked(true);
		ui.pb_cmdTestKick23->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick23->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick23->blockSignals(true);
		ui.pb_cmdTestKick23->setChecked(false);
		ui.pb_cmdTestKick23->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick23->blockSignals(false);
	}
	//ui.lE_Reject24->setText(data.Outputs.Reject[24] ? "1" : "0");
	if (data.Outputs.Reject[24])
	{
		ui.pb_cmdTestKick24->blockSignals(true);
		ui.pb_cmdTestKick24->setChecked(true);
		ui.pb_cmdTestKick24->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick24->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick24->blockSignals(true);
		ui.pb_cmdTestKick24->setChecked(false);
		ui.pb_cmdTestKick24->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick24->blockSignals(false);
	}
	//ui.lE_Reject25->setText(data.Outputs.Reject[25] ? "1" : "0");
	if (data.Outputs.Reject[25])
	{
		ui.pb_cmdTestKick25->blockSignals(true);
		ui.pb_cmdTestKick25->setChecked(true);
		ui.pb_cmdTestKick25->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick25->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick25->blockSignals(true);
		ui.pb_cmdTestKick25->setChecked(false);
		ui.pb_cmdTestKick25->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick25->blockSignals(false);
	}
	//ui.lE_Reject26->setText(data.Outputs.Reject[26] ? "1" : "0");
	if (data.Outputs.Reject[26])
	{
		ui.pb_cmdTestKick26->blockSignals(true);
		ui.pb_cmdTestKick26->setChecked(true);
		ui.pb_cmdTestKick26->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick26->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick26->blockSignals(true);
		ui.pb_cmdTestKick26->setChecked(false);
		ui.pb_cmdTestKick26->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick26->blockSignals(false);
	}
	//ui.lE_Reject27->setText(data.Outputs.Reject[27] ? "1" : "0");
	if (data.Outputs.Reject[27])
	{
		ui.pb_cmdTestKick27->blockSignals(true);
		ui.pb_cmdTestKick27->setChecked(true);
		ui.pb_cmdTestKick27->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick27->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick27->blockSignals(true);
		ui.pb_cmdTestKick27->setChecked(false);
		ui.pb_cmdTestKick27->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick27->blockSignals(false);
	}
	//ui.lE_Reject28->setText(data.Outputs.Reject[28] ? "1" : "0");
	if (data.Outputs.Reject[28])
	{
		ui.pb_cmdTestKick28->blockSignals(true);
		ui.pb_cmdTestKick28->setChecked(true);
		ui.pb_cmdTestKick28->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick28->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick28->blockSignals(true);
		ui.pb_cmdTestKick28->setChecked(false);
		ui.pb_cmdTestKick28->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick28->blockSignals(false);
	}
	//ui.lE_Reject29->setText(data.Outputs.Reject[29] ? "1" : "0");
	if (data.Outputs.Reject[29])
	{
		ui.pb_cmdTestKick29->blockSignals(true);
		ui.pb_cmdTestKick29->setChecked(true);
		ui.pb_cmdTestKick29->setStyleSheet("background: rgb(0,255,0);font-size:20pt");
		ui.pb_cmdTestKick29->blockSignals(false);
	}
	else
	{
		ui.pb_cmdTestKick29->blockSignals(true);
		ui.pb_cmdTestKick29->setChecked(false);
		ui.pb_cmdTestKick29->setStyleSheet("font-size:20pt");
		ui.pb_cmdTestKick29->blockSignals(false);
	}
}
#endif

MESSAGE_HANDLER QtCameraSet::ShowFunc(void* context, DataToPC_typ)
{
	return MESSAGE_HANDLER();
}

void QtCameraSet::connectBtnGroup()//键盘弹出
{
	connect(ui.buttonGroup_Keyboard, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
		[=](QAbstractButton* button) {
		if (button->objectName() == "pushButton_1") { keybd_event(0x31, 0, 0, 0); keybd_event(0x31, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_2") { keybd_event(0x32, 0, 0, 0); keybd_event(0x32, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_3") { keybd_event(0x33, 0, 0, 0); keybd_event(0x33, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_4") { keybd_event(0x34, 0, 0, 0); keybd_event(0x34, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_5") { keybd_event(0x35, 0, 0, 0); keybd_event(0x35, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_6") { keybd_event(0x36, 0, 0, 0); keybd_event(0x36, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_7") { keybd_event(0x37, 0, 0, 0); keybd_event(0x37, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_8") { keybd_event(0x38, 0, 0, 0); keybd_event(0x38, 0, KEYEVENTF_KEYUP, 0); }
		if (button->objectName() == "pushButton_9") { keybd_event(0x39, 0, 0, 0); keybd_event(0x39, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_10") { keybd_event(0x30, 0, 0, 0); keybd_event(0x30, 0, KEYEVENTF_KEYUP, 0); } 
			if (button->objectName() == "pushButton_11") { keybd_event(0xBE, 0, 0, 0); keybd_event(0xBE, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_14") { keybd_event(0x6D, 0, 0, 0); keybd_event(0x6D, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_12") { keybd_event(0x08, 0, 0, 0); keybd_event(0x08, 0, KEYEVENTF_KEYUP, 0); } 

		});
	connect(ui.buttonGroup_Keyboard_1, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
		[=](QAbstractButton* button) {
			if (button->objectName() == "pushButton_1_1") { keybd_event(0x31, 0, 0, 0); keybd_event(0x31, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_2_1") { keybd_event(0x32, 0, 0, 0); keybd_event(0x32, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_3_1") { keybd_event(0x33, 0, 0, 0); keybd_event(0x33, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_4_1") { keybd_event(0x34, 0, 0, 0); keybd_event(0x34, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_5_1") { keybd_event(0x35, 0, 0, 0); keybd_event(0x35, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_6_1") { keybd_event(0x36, 0, 0, 0); keybd_event(0x36, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_7_1") { keybd_event(0x37, 0, 0, 0); keybd_event(0x37, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_8_1") { keybd_event(0x38, 0, 0, 0); keybd_event(0x38, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_9_1") { keybd_event(0x39, 0, 0, 0); keybd_event(0x39, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_10_1") { keybd_event(0x30, 0, 0, 0); keybd_event(0x30, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_11_1") { keybd_event(0xBE, 0, 0, 0); keybd_event(0xBE, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_11_2") { keybd_event(0x6D, 0, 0, 0); keybd_event(0x6D, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_12_1") { keybd_event(0x08, 0, 0, 0); keybd_event(0x08, 0, KEYEVENTF_KEYUP, 0); }
			if (button->objectName() == "pushButton_13_2") { keybd_event(0x0D, 0, 0, 0); keybd_event(0x0D, 0, KEYEVENTF_KEYUP, 0); }//确定
		});
}

void QtCameraSet::SyncAnotherStartGrab(bool b)
{
	if (b)
	{
		ui.cB_feed->setEnabled(false);
		ui.cB_rotate->setEnabled(false);
		ui.cB_saveImage->setEnabled(false);
		ui.pB_StartContinueGrab->setEnabled(false);
	}
	else
	{
		ui.cB_feed->setEnabled(true);
		if (!ui.cB_feed->isChecked())
		{
			ui.cB_rotate->setEnabled(true);
		}
		ui.cB_saveImage->setEnabled(true);
		ui.pB_StartContinueGrab->setEnabled(true);
	}

}

void QtCameraSet::StartGrab(bool b)
{
	DataFromPC_typ typ;
	typ.Dummy = 0;//占空
	typ.Telegram_typ = 1;//命令报文
	if (b)
	{
		if (ui.pB_StartContinueGrab->isChecked())
		{
			ui.pB_StartContinueGrab->setChecked(false);
		}
		ui.pB_StartGrab->setText(QString::fromLocal8Bit("停止测试"));
		ui.pB_StartGrab->setStyleSheet("background: rgb(0,255,0)");

		for (int i = 0; i < g_vectorCamera.size(); i++)
		{
			if (ui.cB_saveImage->isChecked())//saveImage
			{
				m_MultGetThread[i]->SetSaveImage(true);//m_LabelShow = label
			}
		}

		if (!ui.cB_feed->isChecked())
		{
			emit STARTGRABBING(-1, 1);
			for (int i = 0; i < 3; i++)
			{
				typ.Machine_Cmd.cmdTestFlash[i] = 1;
			}
			g_SocketPLC->Communicate_PLC(&typ, nullptr);//
		}
		if (ui.cB_feed->isChecked())
		{
			emit STARTGRABBING(-1, 0);
#ifdef PLCCONNECT
			g_SocketPLC->StartWork();
#endif // PLCCONNECT
		}
		else if (ui.cB_rotate->isChecked())
		{
			//PLC转囊
			typ.Machine_Cmd.cmdRotateCtl = 1;
			g_SocketPLC->Communicate_PLC(&typ, nullptr);//
		}
	}
	else
	{
		ui.cB_flash->setChecked(false);
		ui.pB_StartGrab->setText(QString::fromLocal8Bit("采集测试"));
		ui.pB_StartGrab->setStyleSheet("");
		if (ui.cB_saveImage->isChecked())//save Image
		{
			for (int i = 0; i < m_MultGetThread.size(); i++)
			{
				m_MultGetThread[i]->SetSaveImage(false);//m_LabelShow = label
			}
		}
		StopGrab();
		QThread::msleep(100);
		if (!ui.cB_feed->isChecked())
		{
			typ.Machine_Cmd.cmdRotateCtl = 2;
			for (int i = 0; i < 3; i++)
			{
				typ.Machine_Cmd.cmdTestFlash[i] = 0;
			}
			g_SocketPLC->Communicate_PLC(&typ, nullptr);//
		}
		if (ui.cB_rotate->isChecked())
		{
			//PLC转囊
			typ.Machine_Cmd.cmdRotateCtl = 2;
			g_SocketPLC->Communicate_PLC(&typ, nullptr);//
		}
	}

}

void QtCameraSet::StopGrab()
{
#ifdef PLCCONNECT
	if (g_SocketPLC!=nullptr)
	{
		g_SocketPLC->StopWork();
	}
#endif // PLCCONNECT

	for (int i = 0; i < g_vectorCamera.size(); i++)
	{
#ifdef BASLER
		g_vectorCamera[i]->cb_Camera.StopGrabbing();
		QThread::msleep(100);
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
}
bool QtCameraSet::copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist)//拷贝文件夹/目录功能
{
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if (!targetDir.exists()) {    /**< 如果目标目录不存在,则进行创建 */
		if (!targetDir.mkpath(targetDir.absolutePath()))
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

bool QtCameraSet::deleteDir(const QString& path)//eg：deleteDir(AppPath + "/DefaultModel");//删除文件夹/目录功能
{
	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
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

void QtCameraSet::modelDelete()//删除指定模板功能
{
	QString modelPath = ui.listWidget->currentItem()->text();
	if (g_qModelName == modelPath)//正在应用的模板不可删除
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("当前正在应用该模板,无法删除"), 2000);
		levelOut->show();
		return;
	}
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
	QString defaultModel = configIniRead.value("ProgramSetting/DefaultModel", "testA").toString();
	if (defaultModel == modelPath)//默认模板不可删除
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("该模板是默认模板,无法删除"), 2000);
		levelOut->show();
		return;
	}
	int ret = showMsgBox(QMessageBox::Warning, "警告", "<img src = './ico/warning.png'/>\t模板一经删除无法恢复，是否确认删除？", "确认", "取消");
	if (ret == QMessageBox::Yes)
	{
		deleteDir(AppPath + "/ModelFile/" + modelPath);
		delete ui.listWidget->currentItem();
	}


}
void QtCameraSet::modelAdd()
{
	QString modelPath = ui.listWidget->currentItem()->text();

	char src[100], dest[100], dest2[200];//太小会溢出

	strcpy(dest2, "请选择添加方式: \n复制：通过复制模板 ");
	strcpy(dest, modelPath.toStdString().c_str());
	strcpy(src, " 的参数添加模板\n新建：根据当前采样参数添加新建模板\n取消：取消添加模板操作");

	strcat(dest, src);
	strcat(dest2, dest);
	//👆👆👆👆👆👆

	QMessageBox msg(QMessageBox::Information, QString::fromLocal8Bit("新建方式确认"), QString::fromLocal8Bit(dest2), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	msg.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("复制"));
	msg.setButtonText(QMessageBox::No, QString::fromLocal8Bit("新建"));
	msg.setButtonText(QMessageBox::Cancel, QString::fromLocal8Bit("取消"));
	msg.setWindowIcon(QIcon("./ico/dr.ico"));
	int res = msg.exec();

	if (res == QMessageBox::Yes)//复制当前并取名
	{
		inputModel = new InputDialog;
		connect(inputModel, SIGNAL(toFather(QString)), this, SLOT(applyNameChange(QString)));//难点
		inputModel->setWindowModality(Qt::ApplicationModal);
		inputModel->show();
	}
	else if (res == QMessageBox::No)//新建
	{
	}
	else if (res == QMessageBox::Cancel)//取消
	{
	}

}

void QtCameraSet::modelChangeName()
{
	QString modelPath = ui.listWidget->currentItem()->text();
	if (g_qModelName == modelPath)//正在应用的模板不可重命名
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("当前正在应用该模板,不可重命名"), 2000);
		levelOut->show();
		return;
	}
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);
	QString defaultModel = configIniRead.value("ProgramSetting/DefaultModel", "testA").toString();
	if (defaultModel == modelPath)//默认模板不可删除
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("该模板是默认模板,不可重命名"), 2000);
		levelOut->show();
		return;
	}
	inputModel = new InputDialog;
	connect(inputModel, SIGNAL(toFather(QString)), this, SLOT(applyNameChange(QString)));//难点
	inputModel->setWindowModality(Qt::ApplicationModal);
	inputModel->show();


}


void QtCameraSet::applyNameChange(QString str)//重命名成功
{
	if (modelID == 1)
	{
		QString strNow = AppPath + "\\ModelFile\\" + ui.listWidget->currentItem()->text();//点中的
		QString strShouldBeChanged = AppPath + "\\ModelFile\\" + str;
		std::fstream f;
		f.open(strNow.toStdString().c_str());//修改该目录名称
		rename(strNow.toStdString().c_str(), strShouldBeChanged.toStdString().c_str());//后侧覆盖前侧
		f.close();
		//QFile file(strShouldBeChanged + "/" + ui.listWidget->currentItem()->text() + ".txt");//修改指定文件名称
		//file.rename(strShouldBeChanged + "/" + str+ ".txt");//后侧覆盖前侧
		//找到所有模板名称👇👇
		QDir dir(AppPath + "\\ModelFile");
		dir.setFilter(QDir::Dirs);//筛选目录
		QFileInfoList list = dir.entryInfoList();//文件信息list

		int file_count = list.count();
		QStringList string_list;
		int i;
		for (i = 0; i < file_count; i++)
		{
			QFileInfo file_info = list.at(i);
			QString folderName = file_info.fileName();
			string_list.append(folderName);
		}

		string_list.removeAt(0);//去掉.
		string_list.removeAt(0);//去掉..
		ui.listWidget->clear();
		ui.listWidget->addItems(string_list);
		//显示默认模板👇👇
		for (i = 0; i < string_list.count(); i++)
		{

			if (string_list.at(i) == str)//找到的刚才修改的那个名称
			{
				ui.listWidget->setCurrentRow(i);
			}
		}
	}

	if (modelID == 2)
	{
		QString strNow = AppPath + "\\ModelFile\\" + ui.listWidget->currentItem()->text();//点中的
		QString strAdded = AppPath + "\\ModelFile\\" + str;

		copyDirectoryFiles(strNow, strAdded, true);//true代表覆盖已有的,前面拷贝到后面

		QDir dir(AppPath + "\\ModelFile");
		dir.setFilter(QDir::Dirs);//筛选目录
		QFileInfoList list = dir.entryInfoList();//文件信息list

		int file_count = list.count();
		QStringList string_list;
		int i;
		for (i = 0; i < file_count; i++)
		{
			QFileInfo file_info = list.at(i);
			QString folderName = file_info.fileName();
			string_list.append(folderName);
		}

		string_list.removeAt(0);//去掉.
		string_list.removeAt(0);//去掉..
		ui.listWidget->clear();
		ui.listWidget->addItems(string_list);
		//显示默认模板👇👇
		for (i = 0; i < string_list.count(); i++)
		{

			if (string_list.at(i) == str)//找到的刚才修改的那个名称
			{
				ui.listWidget->setCurrentRow(i);
			}
		}
	}
}

void QtCameraSet::onCameraKeyboardOut(int r, int c)
{
	if (r != 2 && c > 0)
	{
		SWITCHOSK();
		//SHOWOSK();
	}
}

int QtCameraSet::showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str)//全是中文
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


void QtCameraSet::SHOWOSK()//键盘显示
{
	//The code from david!!!
	PVOID OldValue;
	BOOL bRet = Wow64DisableWow64FsRedirection(&OldValue);
	//QString csProcess = "C:/Program/FilesCommon/Filesmicrosoft/sharedink/TabTip.exe";
	//QString csProcess = AppPath + "/osk.exe";
	QString csProcess = "C:/WINDOWS/system32/osk.exe";
	QString params = "";
	ShellExecute(NULL, L"open", (LPCWSTR)csProcess.utf16(), (LPCWSTR)params.utf16(), NULL, SW_SHOWNORMAL);
	if (bRet)
	{
		Wow64RevertWow64FsRedirection(OldValue);
	}
}
void QtCameraSet::SWITCHTABANDOSK()
{
	SWITCHOSK();
	ui.tabWidget_Users->setCurrentIndex(1);
}
void QtCameraSet::SWITCHOSK()//快捷键
{
	keybd_event(0x11, 0, 0, 0);
	keybd_event(0x5B, 0, 0, 0);
	keybd_event(0x4F, 0, 0, 0);
	keybd_event(0x11, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x5B, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x4F, 0, KEYEVENTF_KEYUP, 0);//ctrl+win+o切换
}

bool QtCameraSet::IsNumber(QString& qstrSrc)//是否是纯数字
{
	QByteArray ba = qstrSrc.toLatin1();
	const char* s = ba.data();
	bool bret = true;
	while (*s)
	{
		if ((*s >= '0' && *s <= '9'))
		{

		}
		else
		{
			bret = false;
			break;
		}
		s++;
	}
	return bret;
}
//————————————————
//版权声明：本文为CSDN博主「Good运」的原创文章，遵循 CC 4.0 BY - SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https ://blog.csdn.net/qq_36391817/article/details/80708590


void QtCameraSet::updateParentItem(QTreeWidgetItem* item)
{

	QTreeWidgetItem* parent = item->parent();

	if (parent == NULL)
		return;

	int nSelectedCount = 0;//选中数
	int nHalfSelectedCount = 0;//半选中数
	int childCount = parent->childCount();//孩子数

	for (int i = 0; i < childCount; i++) //判断有多少个子项被选中
	{
		QTreeWidgetItem* childItem = parent->child(i);
		if (childItem->checkState(0) == Qt::Checked)
		{
			nSelectedCount++;
		}
		else if (childItem->checkState(0) == Qt::PartiallyChecked)
		{
			nHalfSelectedCount++;
		}
	}

	if ((nSelectedCount + nHalfSelectedCount) <= 0)  //如果没有子项被选中，父项设置为未选中状态
		parent->setCheckState(0, Qt::Unchecked);
	else if ((childCount > nHalfSelectedCount && nHalfSelectedCount > 0) || (childCount > nSelectedCount && nSelectedCount > 0))// && nSelectedCount < childCount)    //如果有部分子项被选中，父项设置为部分选中状态，即用灰色显示
		parent->setCheckState(0, Qt::PartiallyChecked);
	else if (nSelectedCount == childCount)    //如果子项全部被选中，父项则设置为选中状态
		parent->setCheckState(0, Qt::Checked);

	updateParentItem(parent);//

}

void QtCameraSet::onTreeItemChanged(QTreeWidgetItem* item)//利用changed自动递归。
{
	int count = item->childCount(); //返回子项的个数

	if (Qt::Checked == item->checkState(0))
	{
		item->setCheckState(0, Qt::Checked);
		if (count > 0)
		{
			for (int i = 0; i < count; i++)
			{
				item->child(i)->setCheckState(0, Qt::Checked);
			}
		}
		else { updateParentItem(item); }
	}
	else if (Qt::Unchecked == item->checkState(0))
	{
		if (count > 0)
		{
			for (int i = 0; i < count; i++)
			{
				item->child(i)->setCheckState(0, Qt::Unchecked);
			}
		}
		else { updateParentItem(item); }
	}

}

//上面是树型多选框函数

//下面是查看权限
void QtCameraSet::checkPermission()
{
	ui.treeWidget_2->clear();    //初始化树形控件
	ui.treeWidget_2->setHeaderHidden(true);  //隐藏表头
	QFont serifFont("Times", 16);
	ui.treeWidget_2->setFont(serifFont);

	//定义第一个树形组 爷爷项
	checkPermissionGroup = new QTreeWidgetItem(ui.treeWidget_2);
	QString str = ui.cB_Users->currentText();
	checkPermissionGroup->setText(0, str);    //树形控件显示的文本信息
	//group->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);   //设置树形控件子项的属性
	//Qt::ItemIsUserCheckable | Qt::ItemIsSelectable 两个都是方框是否可选状态，暂时没用
	//Qt::ItemIsEnabled 使能，不使能会显示为灰色，可以在查看的时候而非添加的时候用
	//Qt::ItemIsEditable 文字可编辑与否，我们都不让编辑
	checkPermissionGroup->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);   //设置树形控件子项的属性
	checkPermissionGroup->setCheckState(0, Qt::Checked); //初始状态没有被选中
	checkPermissionGroup->setBackground(0, QBrush(QColor("#880f97ff")));//AARRGGBB /RRGGBB

	//第一组子项
	QTreeWidgetItem* group1 = new QTreeWidgetItem(checkPermissionGroup);
	// 	QFont headFont("Times", 16,QFont::Bold);
	group1->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group1->setText(0, QString::fromLocal8Bit("设备运行"));  //设置子项显示的文本
	group1->setCheckState(0, Qt::Checked); //设置子选项的显示格式和状态

	QTreeWidgetItem* group2 = new QTreeWidgetItem(checkPermissionGroup);
	group2->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group2->setText(0, QString::fromLocal8Bit("设置"));
	group2->setCheckState(0, Qt::Checked);
	//设置蓝色group2->setBackground(0, QBrush(QColor("#0000FF")));

	//父亲项
	QTreeWidgetItem* group21 = new QTreeWidgetItem(group2);
	group21->setText(0, QString::fromLocal8Bit("模板管理"));
	group21->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group21->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group22 = new QTreeWidgetItem(group2);
	group22->setText(0, QString::fromLocal8Bit("相机参数"));
	group22->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group22->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group23 = new QTreeWidgetItem(group2);
	group23->setText(0, QString::fromLocal8Bit("PLC设置"));
	group23->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group23->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group24 = new QTreeWidgetItem(group2);
	group24->setText(0, QString::fromLocal8Bit("用户管理"));
	group24->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group24->setCheckState(0, Qt::Checked);

	//孙子项1
	QTreeWidgetItem* group211 = new QTreeWidgetItem(group21);   //指定子项属于哪一个父项
	group211->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group211->setText(0, QString::fromLocal8Bit("保存/应用"));
	group211->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group212 = new QTreeWidgetItem(group21);
	group212->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group212->setText(0, QString::fromLocal8Bit("添加"));
	group212->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group213 = new QTreeWidgetItem(group21);
	group213->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group213->setText(0, QString::fromLocal8Bit("删除"));
	group213->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group214 = new QTreeWidgetItem(group21);
	group214->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group214->setText(0, QString::fromLocal8Bit("修改名称"));
	group214->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group215 = new QTreeWidgetItem(group21);
	group215->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group215->setText(0, QString::fromLocal8Bit("算法设置"));
	group215->setCheckState(0, Qt::Checked);
	//孙子项2
	QTreeWidgetItem* group221 = new QTreeWidgetItem(group22);
	group221->setText(0, QString::fromLocal8Bit("相机调试"));
	group221->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group221->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group222 = new QTreeWidgetItem(group22);
	group222->setText(0, QString::fromLocal8Bit("采集测试"));
	group222->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group222->setCheckState(0, Qt::Checked);

	//孙子项3
	QTreeWidgetItem* group231 = new QTreeWidgetItem(group23);
	group231->setText(0, QString::fromLocal8Bit("参数读取"));
	group231->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group231->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group232 = new QTreeWidgetItem(group23);
	group232->setText(0, QString::fromLocal8Bit("参数写入"));
	group232->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group232->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group233 = new QTreeWidgetItem(group23);
	group233->setText(0, QString::fromLocal8Bit("采集"));
	group233->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group233->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group234 = new QTreeWidgetItem(group23);
	group234->setText(0, QString::fromLocal8Bit("控制测试"));
	group234->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group234->setCheckState(0, Qt::Checked);

	//孙子项4
	QTreeWidgetItem* group241 = new QTreeWidgetItem(group24);
	group241->setText(0, QString::fromLocal8Bit("添加用户"));
	group241->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group241->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group242 = new QTreeWidgetItem(group24);
	group242->setText(0, QString::fromLocal8Bit("切换用户"));
	group242->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group242->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group243 = new QTreeWidgetItem(group24);
	group243->setText(0, QString::fromLocal8Bit("删除用户"));
	group243->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group243->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group244 = new QTreeWidgetItem(group24);
	group244->setText(0, QString::fromLocal8Bit("查看权限"));
	group244->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group244->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group245 = new QTreeWidgetItem(group24);
	group245->setText(0, QString::fromLocal8Bit("更改权限"));
	group245->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group245->setCheckState(0, Qt::Checked);

	QTreeWidgetItem* group3 = new QTreeWidgetItem(checkPermissionGroup);
	group3->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
	group3->setText(0, QString::fromLocal8Bit("数据导出"));
	group3->setCheckState(0, Qt::Checked);

	ui.treeWidget_2->expandAll();  //展开树
	//ui.treeWidget_2->expandToDepth(1);
}

void QtCameraSet::updateCheckPermission(const QString& str)
{
	checkPermissionGroup->setText(0, str);
	QTreeWidgetItemIterator it(ui.treeWidget_2);
	if (checkPermissionGroup->text(0) == QString::fromLocal8Bit("管理员"))//0
	{
		while (*it) {
			if ((*it)->text(0) == QString::fromLocal8Bit("设备运行")
				|| (*it)->text(0) == QString::fromLocal8Bit("设置")
				|| (*it)->text(0) == QString::fromLocal8Bit("数据导出"))
			{
				(*it)->setCheckState(0, Qt::Checked);
			}
			++it;
		}
	}
	else if (checkPermissionGroup->text(0) == QString::fromLocal8Bit("工程师"))//1
	{
		while (*it) {
			if ((*it)->text(0) == QString::fromLocal8Bit("设备运行")
				|| (*it)->text(0) == QString::fromLocal8Bit("设置"))
			{
				(*it)->setCheckState(0, Qt::Checked);
			}
			if ((*it)->text(0) == QString::fromLocal8Bit("用户管理")
				|| (*it)->text(0) == QString::fromLocal8Bit("数据导出"))
			{
				(*it)->setCheckState(0, Qt::Unchecked);
			}
			++it;
		}
	}
	else if (checkPermissionGroup->text(0) == QString::fromLocal8Bit("操作员"))//2
	{
		while (*it) {
			if ((*it)->text(0) == QString::fromLocal8Bit("设备运行"))
			{
				(*it)->setCheckState(0, Qt::Checked);
			}
			if ((*it)->text(0) == QString::fromLocal8Bit("设置")
				|| (*it)->text(0) == QString::fromLocal8Bit("数据导出"))
			{
				(*it)->setCheckState(0, Qt::Unchecked);
			}
			++it;
		}
	}
	else if (checkPermissionGroup->text(0) == QString::fromLocal8Bit("质检员"))//3
	{
		while (*it) {
			if ((*it)->text(0) == QString::fromLocal8Bit("数据导出"))
			{
				(*it)->setCheckState(0, Qt::Checked);
			}
			if ((*it)->text(0) == QString::fromLocal8Bit("设备运行")
				|| (*it)->text(0) == QString::fromLocal8Bit("设置"))
			{
				(*it)->setCheckState(0, Qt::Unchecked);
			}
			++it;
		}
	}

}

void QtCameraSet::initTableOfUserPermission()
{
	QFont font;
	font = ui.tabWidget_CameraVec->font();//左下角白框的字体——[SimSun,21]
	font.setPointSize(21);

	QWidget* tab = new QWidget();//新建tab
	tab->setFont(font);//设置tab字体
	tab->setObjectName(QString::fromUtf8("tab_0"));//tab_23170685
	ui.tabWidget_Users->addTab(tab, QString::fromLocal8Bit("用户权限"));//将tab添加到左下角tabwidget boject name:tab_23170685 tttle:23170685
	QTableWidget* tableWidget = new QTableWidget(tab);//tab下面加tablewidget
	tableWidget->setObjectName(QString::fromLocal8Bit("tableWidget_permission"));//tableWidget_23170685
	tableWidget->setGeometry(QRect(9, 9, tab->height() - 50, tab->width() - 80));//设置widget尺寸 黑边是边界

	QStringList strlist;
	strlist << QString::fromLocal8Bit("权限名称") << QString::fromLocal8Bit("权限级别");
	tableWidget->setColumnCount(2);//两列
	tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分填充表头
	tableWidget->verticalHeader()->setDefaultSectionSize(35);//默认行高20
	tableWidget->setHorizontalHeaderLabels(strlist);//水平表头参数、数值
	tableWidget->verticalHeader()->setVisible(false);
	tableWidget->horizontalHeader()->setVisible(true);//表头可见
	font = tableWidget->horizontalHeader()->font();//表头字体
	font.setPointSize(18);//字号
	tableWidget->horizontalHeader()->setFont(font);//设置表头字体

	font.setPointSize(16);//字号
	tableWidget->setFont(font);//设置内容字体

	int currentcolumn = tableWidget->rowCount();//行数
	tableWidget->insertRow(currentcolumn);//插入行
	tableWidget->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("管理员")));//0列设置
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));//单元格不可编辑
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));//单元格不可选择
	tableWidget->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(0)));//1列设置
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));
	if (g_IUserLevel == 0)
	{
		tableWidget->item(currentcolumn, 0)->setBackground(QBrush(QColor("#8889ff81")));//AARRGGBB /RRGGBB
		tableWidget->item(currentcolumn, 1)->setBackground(QBrush(QColor("#8889ff81")));//AARRGGBB /RRGGBB
	}

	//currentcolumn->setBackgroundColor(QColor(255, 0, 0));

	currentcolumn = tableWidget->rowCount();
	tableWidget->insertRow(currentcolumn);
	tableWidget->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("工程师")));
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	tableWidget->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(1)));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

	currentcolumn = tableWidget->rowCount();
	tableWidget->insertRow(currentcolumn);
	tableWidget->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("操作员")));
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	tableWidget->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(2)));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

	currentcolumn = tableWidget->rowCount();
	tableWidget->insertRow(currentcolumn);
	tableWidget->setItem(currentcolumn, 0, new QTableWidgetItem(QString::fromLocal8Bit("质检员")));
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 0)->setFlags(tableWidget->item(currentcolumn, 0)->flags() & (~Qt::ItemIsSelectable));
	tableWidget->setItem(currentcolumn, 1, new QTableWidgetItem(QString::number(3)));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsEditable));
	tableWidget->item(currentcolumn, 1)->setFlags(tableWidget->item(currentcolumn, 1)->flags() & (~Qt::ItemIsSelectable));

	
}

void QtCameraSet::btn_Enabled(int i)
{
	if (i == 0)
	{
		ui.widget_UsersChoice_2->setVisible(1);
	}
	else {
		ui.widget_UsersChoice_2->setVisible(0);
	}
}


void QtCameraSet::slotCb_flash(bool b)
{
	if (b)
	{
		ui.pb_cmdTestFlash0->setChecked(true);
		ui.pb_cmdTestFlash1->setChecked(true);
		ui.pb_cmdTestFlash2->setChecked(true);
	}
	else
	{
		ui.pb_cmdTestFlash0->setChecked(false);
		ui.pb_cmdTestFlash1->setChecked(false);
		ui.pb_cmdTestFlash2->setChecked(false);
	}
}

void QtCameraSet::slotCb_feed(bool b)
{
	if (b)
	{
		ui.cB_rotate->setChecked(true);
		ui.cB_rotate->setEnabled(false);
	}
	else
	{
		ui.cB_rotate->setChecked(false);
		ui.cB_rotate->setEnabled(true);
	}
}

void QtCameraSet::MoveOut()
{
	int i = 2;//first x
	int x = 670;// last x
	int y = 10;
	int j = 55;//增量
	int delay = 600;
	int delay_1 = 300;
	
	if (ui.pushButton_13->text() == "")
	{
		QPropertyAnimation* animation = new QPropertyAnimation(ui.pushButton_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_2, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_3, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_4, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_5, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_6, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_7, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_8, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_9, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_10, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		animation = new QPropertyAnimation(ui.pushButton_14, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y + 30));
		animation->setEndValue(QPoint(i, y + 30));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_12, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(i, y));
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		ui.pushButton_13->setText(">>");
		ui.pushButton_12->setIcon(QIcon(""));//文件copy到了exe所在目录
		ui.pushButton_12->setText(QString::fromLocal8Bit("删"));//文件copy到了exe所在目录
	}
	else if(((QPushButton*)sender())==ui.pushButton_13)
	{
		QPropertyAnimation* animation = new QPropertyAnimation(ui.pushButton_1, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_2, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_3, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_4, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_5, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_6, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_7, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_8, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_9, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_10, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		animation = new QPropertyAnimation(ui.pushButton_14, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y + 30));
		animation->setEndValue(QPoint(x, y + 30));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_12, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		ui.pushButton_13->setText("");
		ui.pushButton_12->setIcon(QIcon("./ico/dr_keyboard.ico"));//文件copy到了exe所在目录
		ui.pushButton_12->setText("");//文件copy到了exe所在目录
	}
}


void QtCameraSet::MoveOutWhenWrite()
{
	int i = 2;//first x
	int x = 670;// last x
	int y = 10;
	int j = 55;//增量
	int delay = 600;
	int delay_1 = 300;

	if (ui.pushButton_13->text()== ">>")
	{
		QPropertyAnimation* animation = new QPropertyAnimation(ui.pushButton_1, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_2, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_3, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_4, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_5, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_6, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_7, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_8, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_9, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_10, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		animation = new QPropertyAnimation(ui.pushButton_14, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y + 30));
		animation->setEndValue(QPoint(x, y + 30));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_12, "pos");
		animation->setDuration(delay_1);
		animation->setStartValue(QPoint(i, y));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		ui.pushButton_13->setText("");
		ui.pushButton_12->setIcon(QIcon("./ico/dr_keyboard.ico"));//文件copy到了exe所在目录
		ui.pushButton_12->setText("");//文件copy到了exe所在目录
	}
}


void QtCameraSet::MoveOut_1()
{
	int i = 0;
	int x = 660;// last x//1  538  2  599   3 660
	int y = 550;  //1  221  2  282  3  343   4 404
	int j = 0;
	int delay = 200;
	if (ui.pushButton_13_1->text() == "")
	{
		QPropertyAnimation* animation = new QPropertyAnimation(ui.pushButton_1_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(540, 430));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_2_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(600, 430));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_3_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(660, 430));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_4_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(540, 370));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_5_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(600, 370));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_6_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(660, 370));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_7_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(540, 310));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_8_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(600, 310));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_9_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(660, 310));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_10_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(600, 490));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(540, 490));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11_2, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(540, 520));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_12_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(x, y));
		animation->setEndValue(QPoint(660, 490));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		ui.pushButton_13_1->setText(">>");
		ui.pushButton_13_2->setVisible(true);

		ui.pushButton_12_1->setIcon(QIcon(""));//文件copy到了exe所在目录
		ui.pushButton_12_1->setText(QString::fromLocal8Bit("删"));//文件copy到了exe所在目录
	}
	else
	{
		QPropertyAnimation* animation = new QPropertyAnimation(ui.pushButton_1_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(540, 430));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_2_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(600, 430));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_3_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(660, 430));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_4_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(540, 370));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_5_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(600, 370));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_6_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(660, 370));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_7_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(540, 310));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_8_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(600, 310));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		i += j;
		animation = new QPropertyAnimation(ui.pushButton_9_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(660, 310));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_10_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(600, 490));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(540, 490));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_11_2, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(540, 490));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		i += j;
		animation = new QPropertyAnimation(ui.pushButton_12_1, "pos");
		animation->setDuration(delay);
		animation->setStartValue(QPoint(660, 490));
		animation->setEndValue(QPoint(x, y));
		animation->setEasingCurve(QEasingCurve::Linear);
		animation->start(QAbstractAnimation::DeleteWhenStopped);

		ui.pushButton_13_1->setText("");
		ui.pushButton_13_2->setVisible(false);

		ui.pushButton_12_1->setIcon(QIcon("./ico/dr_keyboard.ico"));//文件copy到了exe所在目录
		ui.pushButton_12_1->setText("");//文件copy到了exe所在目录
	}
}


void QtCameraSet::switchTabPage(int i)
{
	ui.tabWidget_CameraVec->blockSignals(true);
	if (ui.tabWidget_CameraVec->count()>=i)
	{
		ui.tabWidget_CameraVec->setCurrentIndex(i);
	}
	ui.tabWidget_CameraVec->blockSignals(false);

}

void QtCameraSet::showCurrentImage(int i)
{
	if (i < g_vectorCamera.size())//最后一个全局调整不执行此槽函数
	{
		if (ui.pB_StartContinueGrab->isChecked())
		{
			g_vectorCamera[m_iTabCurrentIndex]->cb_Camera.StopGrabbing();//停止取图
			emit STARTGRABBING(i, 1);
		}
		if (m_iTabCurrentIndex != -1)
		{
			MyLabel* lab = ui.gB_ShowArea->findChild<MyLabel*>(QString("label") + QString::number(m_iTabCurrentIndex));
			if (lab->b_scale)
			{
				lab->mousePressEvent(nullptr);
			}

		}
		m_iTabCurrentIndex = i;
		MyLabel* lab = ui.gB_ShowArea->findChild<MyLabel*>(QString("label") + QString::number(i));
		lab->mousePressEvent(nullptr);

	}
}

void QtCameraSet::on_pb_cmdParaSave_clicked()//加一个按钮保存
{
	DataFromPC_typ typ;
	typ = getPCData();
	char* plcParam = new char[sizeof(DataFromPC_typ)+1];
	memset(plcParam, 0, sizeof(DataFromPC_typ)+1); //1实验用
	memcpy(plcParam, &typ, sizeof(DataFromPC_typ));
	int a = strlen(plcParam);//字符串的长度

	QString plcParamPath = AppPath + "/plcParam/plc.txt";

	QFile f(plcParamPath);
	f.resize(0);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		showMsgBox(QMessageBox::Question, "错误提示", "<img src = './ico/critical.png'/>\t参数文件写入错误，请重试", "我知道了", "");
		return ;
	}
	f.write(plcParam,sizeof(DataFromPC_typ));
	f.close();
	levelOut = new WindowOut;
	levelOut->getString(QString::fromLocal8Bit("恭喜，参数文件保存成功！"), 2000);
	levelOut->show();

}

void QtCameraSet::on_pb_cmdParaLoad_clicked()//
{
	DataFromPC_typ typ;

	QString plcParamPath = AppPath + "/plcParam/plc.txt";
	QFile f(plcParamPath);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		showMsgBox(QMessageBox::Question, "错误提示", "<img src = './ico/critical.png'/>\t参数文件读取错误，请重试", "我知道了", "");
		return;
	}
	char* ch = new char[sizeof(DataFromPC_typ) + 1];
	memset(ch, 0, sizeof(DataFromPC_typ)+1);
	//txtInput.readAll();
	f.read(ch, sizeof(DataFromPC_typ));

	memcpy(&typ, ch, sizeof(DataFromPC_typ));//赋值回来

	f.close();

	levelOut = new WindowOut;
	levelOut->getString(QString::fromLocal8Bit("恭喜，参数文件读取成功，已写入PLC！"), 2000);
	levelOut->show();

	typ.Telegram_typ = 2;
	g_SocketPLC->Communicate_PLC(&typ, nullptr);//系统

	typ.Telegram_typ = 4;
	g_SocketPLC->Communicate_PLC(&typ, nullptr);//运行
}

//新建用户
void QtCameraSet::on_lE_SetUserName_textChanged(const QString &arg1)
{
	if (arg1 != "")
	{
		ui.cB_SetUserPermission->setEnabled(true);
		ui.lE_SetUserSecretNum->setEnabled(true);
		if (ui.lE_SetUserSecretNum->text().length()>=4)
		{
			ui.pB_AddUser->setEnabled(true);
		}
	}
	else {

		ui.cB_SetUserPermission->setEnabled(false);
		ui.lE_SetUserSecretNum->setEnabled(false);
		ui.pB_AddUser->setEnabled(false);
	}
}

void QtCameraSet::on_lE_SetUserSecretNum_textChanged(const QString &arg1)
{
	if (arg1.length() >= 4)
	{
		ui.pB_AddUser->setEnabled(true);
	}
	else 
	{
		ui.pB_AddUser->setEnabled(false);
	}
}

void QtCameraSet::initTableWidget()
{//第二个表

	QWidget *tab = new QWidget();//新建tab
	tab->setObjectName(QString::fromUtf8("tab_1"));//tab_23170685
	ui.tabWidget_Users->addTab(tab, QString::fromLocal8Bit("自定义用户"));//将tab添加到左下角tabwidget boject name:tab_23170685 tttle:23170685
	tableWidget = new QTableWidget(tab);//tab下面加tablewidget
	tableWidget->setObjectName(QString::fromLocal8Bit("tableWidget_username"));//tableWidget_23170685
	tableWidget->setGeometry(QRect(9, 9, tab->height() - 50, tab->width() - 80));//设置widget尺寸 黑边是边界

	QStringList strlist2;
	strlist2 << QString::fromLocal8Bit("用户名") << QString::fromLocal8Bit("权限级别");
	tableWidget->setColumnCount(2);//两列
	tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分填充表头
	tableWidget->verticalHeader()->setDefaultSectionSize(35);//默认行高20
	tableWidget->setHorizontalHeaderLabels(strlist2);//水平表头参数、数值
	tableWidget->verticalHeader()->setVisible(false);
	tableWidget->horizontalHeader()->setVisible(true);//表头可见
	font = tableWidget->horizontalHeader()->font();//表头字体
	tableWidget->horizontalHeader()->setFont(font);//设置表头字体


	QSettings FinalDir(AppPath + "/ModelFile/ProgramSet.ini", QSettings::IniFormat);//所有结果文件
	QStringList str = FinalDir.childGroups();

	int count = str.size();

	int j = 0;
	for (int i = 0; i < count; i++)
	{
		QString One = str.at(i);//节点
		if (One.mid(0, 4) == "USER")
		{
			tableWidget->insertRow(j);//加一行
			QString j0 = One.mid(5);
			tableWidget->setItem(j, 0, new QTableWidgetItem(j0));//添加日期result行
			tableWidget->item(j, 0)->setFlags(tableWidget->item(j, 0)->flags() & (~Qt::ItemIsEditable));//单元格不可编辑
			QString j1 = FinalDir.value(One + "/" + "Level", -1).toString();
			tableWidget->setItem(j, 1, new QTableWidgetItem(j1));//添加日期result行
			tableWidget->item(j, 1)->setFlags(tableWidget->item(j, 1)->flags() & (~Qt::ItemIsEditable));//单元格不可编辑
			tableWidget->item(j, 1)->setFlags(tableWidget->item(j, 1)->flags() & (~Qt::ItemIsSelectable));//单元格不可选择
			j++;
		}
	}

	connect(tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(selectedName(int, int)));
}

void QtCameraSet::selectedName(int r, int c)
{
	if (c==0)
	{
		m_SelectedName = ((QTableWidget*)sender())->item(r, c)->text();
	}
	else
	{
		m_SelectedName = "";
	}
}

void QtCameraSet::on_pB_Users_Delete_clicked()
{
	if (m_SelectedName == "")
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("请先选择用户，然后进行删除！"), 2000);
		levelOut->show();
		return;
	}
	else if (m_SelectedName=="Admin")
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("Admin管理员账户不可删除，请选择其他用户！"), 2000);
		levelOut->show();
		return;
	}
	else 
	{
		if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "删除确认", "<img src = './ico/question.png'/>\t确认删除该用户？", "确认", "取消"))
		{
			QSettings Dir(AppPath + "/ModelFile/ProgramSet.ini", QSettings::IniFormat);//所有结果文件
			QString path = AppPath + "/ModelFile/ProgramSet.ini";
			QString fullName = "USER_" + m_SelectedName;
			::WritePrivateProfileStringA(fullName.toStdString().c_str(), NULL, NULL, path.toStdString().c_str());

			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，") + m_SelectedName + QString::fromLocal8Bit("删除成功！"), 2000);
			levelOut->show();

			initTableWidget();
			ui.tabWidget_Users->removeTab(1);
			m_SelectedName = "";
		}
	}
}

void QtCameraSet::addUser()
{
	QSettings Dir(AppPath + "/ModelFile/ProgramSet.ini", QSettings::IniFormat);//所有结果文件
	QStringList str = Dir.childGroups();

	int count = str.size();

	for (int i = 0; i < count; i++)
	{
		QString One = str.at(i);//节点
		if (One.mid(0, 4) == "USER")
		{
			QString j0 = One.mid(5);
			
			if (!ui.lE_SetUserName->text().compare(j0, Qt::CaseInsensitive))//不区分大小写比较
			{
				if (QMessageBox::Yes == showMsgBox(QMessageBox::Question, "冲突确认", "<img src = './ico/question.png'/>\t用户名已存在(大小写不敏感)，是否更新？", "更新", "取消"))
				{
					Dir.setValue("USER_" + ui.lE_SetUserName->text() + "/Password", ui.lE_SetUserSecretNum->text());//写当前模板
					Dir.setValue("USER_" + ui.lE_SetUserName->text() + "/Level", QString::number(ui.cB_SetUserPermission->currentIndex()));//写当前模板
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，用户更新成功！"), 2000);
					levelOut->show();
					SWITCHOSK();
					initTableWidget();
					ui.tabWidget_Users->removeTab(1);
					return;
				}
				else 
				{
					return; 
				}
			}
		}
	}
	Dir.setValue("USER_" + ui.lE_SetUserName->text() + "/Password", ui.lE_SetUserSecretNum->text());//写当前模板
	Dir.setValue("USER_" + ui.lE_SetUserName->text() + "/Level", QString::number(ui.cB_SetUserPermission->currentIndex()));//写当前模板
	levelOut = new WindowOut;
	levelOut->getString(QString::fromLocal8Bit("恭喜，新建用户成功！"), 2000);
	levelOut->show();
	initTableWidget();
	ui.tabWidget_Users->removeTab(1);
	SWITCHTABANDOSK();
}

void QtCameraSet::on_lE_SetUserSecretNum_returnPressed()
{
	addUser();
}

void QtCameraSet::on_lE_SetUserName_returnPressed()
{
	ui.lE_SetUserSecretNum->setFocus();
}

//确定修改后键盘隐藏
void QtCameraSet::on_pB_changeIPPort_clicked()
{
	MoveOutWhenWrite();
}


//continueKick
void QtCameraSet::initContinueKick()
{
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
	ui.cB_ContinueKickAlarm ->setCurrentIndex(configIniRead.value("ProgramSetting/ContinueKickAlarm", 0).toInt());//连剔报警开关
	ui.lE_ContinueKickAlarmCount ->setText(configIniRead.value("ProgramSetting/ContinueKickAlarmCount", 50).toString());//连剔报警数量
	ui.cB_ContinueKickStop->setCurrentIndex(configIniRead.value("ProgramSetting/ContinueKickStop", 0).toInt());//连剔停机开关
	ui.lE_ContinueKickStopCount->setText(configIniRead.value("ProgramSetting/ContinueKickStopCount", 60).toString());//连剔停机数量

	QRegExp regx("[0-9]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui.lE_ContinueKickAlarmCount->setValidator(new QRegExpValidator(regx, this));
	ui.lE_ContinueKickStopCount->setValidator(new QRegExpValidator(regx, this));
}


void QtCameraSet::on_pB_ContinueKickOK_clicked()//保存连剔
{
	int alarmCount = ui.lE_ContinueKickAlarmCount->text().toInt();//为了去掉头部的0
	int stopCount = ui.lE_ContinueKickStopCount->text().toInt();

	ui.lE_ContinueKickAlarmCount->setText(QString::number(alarmCount));//连剔报警数量
	ui.lE_ContinueKickStopCount->setText(QString::number(stopCount));//连剔停机数量

	if (alarmCount <1|| stopCount<1)
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("设置胶囊数不能小于1！"), 2000);
		levelOut->show();
		initContinueKick();
		return;
	}
	else if (alarmCount > stopCount)
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("报警胶囊数不能大于停机胶囊数！"), 2000);
		levelOut->show();
		initContinueKick();
		return;
	}
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
	configIniRead.setValue("ProgramSetting/ContinueKickAlarm", ui.cB_ContinueKickAlarm->currentIndex());
	configIniRead.setValue("ProgramSetting/ContinueKickAlarmCount", alarmCount);
	configIniRead.setValue("ProgramSetting/ContinueKickStop", ui.cB_ContinueKickStop->currentIndex());
	configIniRead.setValue("ProgramSetting/ContinueKickStopCount", stopCount);


	levelOut = new WindowOut;
	levelOut->getString(QString::fromLocal8Bit("恭喜，连剔设置成功！"), 2000);
	levelOut->show();
	MoveOutWhenWrite();//键盘移开

}
void QtCameraSet::on_pB_ContinueKickCancel_clicked()//取消连剔
{
	initContinueKick();
	MoveOutWhenWrite();//键盘移开
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void QtCameraSet::initFrame_light()
{
	ui.frame_light->setVisible(false);
	QRegExp regx("[0-9]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui.lE_light1->setValidator(new QRegExpValidator(regx, this));
	ui.lE_light2->setValidator(new QRegExpValidator(regx, this));
	ui.lE_light3->setValidator(new QRegExpValidator(regx, this));

	ui.cB_232Port->setCurrentIndex(g_232Port - 1);
	connect(ui.cB_232Port, QOverload<const QString&>::of(&QComboBox::activated),
		[=](const QString& text) {
		QSettings Dir(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//找到文件
		Dir.setValue("ProgramSetting/232Port", text);//写当前模板
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("恭喜，串口号设置成功！"), 2000);
		levelOut->show();
	});
}


void QtCameraSet::on_pB_brightness_toggled(bool checked)
{

	if (checked)
	{

		QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
		ui.lE_light1->setText(configIniRead.value("ProgramSetting/Light1", 50).toString());
		ui.lE_light2->setText(configIniRead.value("ProgramSetting/Light2", 50).toString());
		ui.lE_light3->setText(configIniRead.value("ProgramSetting/Light3", 50).toString());

		ui.frame_light->setVisible(true);			
		if (ui.pushButton_13_1->text() == "")
		{
			MoveOut_1();
		}
		ui.lE_light1->setFocus();
		ui.lE_light1->selectAll();
	}
	else
	{
		ui.frame_light->setVisible(false);		
		if (ui.pushButton_13_1->text() != "")
		{
			MoveOut_1();
		}
		ui.lE_light1->clearFocus();
		ui.lE_light2->clearFocus();
		ui.lE_light3->clearFocus();

		if (nullptr != m_serialPort)
		{
			if (m_serialPort->isOpen())//��������Ѿ����� �ȸ����ر���
			{
				m_serialPort->clear();
				m_serialPort->close();
			}
		}
	}
}


void QtCameraSet::on_pB_adjustBrightness_clicked()//保存亮度调节
{

	onConnectPort();

	int light1 = ui.lE_light1->text().toInt();//为了去掉头部的0
	int light2 = ui.lE_light2->text().toInt();
	int light3 = ui.lE_light3->text().toInt();

	ui.lE_light1->setText(QString::number(light1));
	ui.lE_light2->setText(QString::number(light2));
	ui.lE_light3->setText(QString::number(light3));

	if (light1 > 255 || light2 > 255 || light3 > 255)
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("亮度值大于255时，自动设置最大值为255！"), 2000);
		levelOut->show();

		if (light1 > 255)
		{
			ui.lE_light1->setText("255");
			light1 = 255;
		}		
		if (light2 > 255)
		{
			ui.lE_light2->setText("255");
			light2 = 255;
		}	
		if (light3> 255)
		{
			ui.lE_light3->setText("255");
			light3 = 255;
		}
	}
	if (m_serialPort->isOpen())
	{

		connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receiveInfo()));

		ui.pB_adjustBrightness->setEnabled(false);
		int light1 = ui.lE_light1->text().toInt();

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
	QSettings configIniRead(AppPath + "\\ModelFile\\ProgramSet.ini", QSettings::IniFormat);//读取ini文件
	configIniRead.setValue("ProgramSetting/Light1", light1);
	configIniRead.setValue("ProgramSetting/Light2", light2);
	configIniRead.setValue("ProgramSetting/Light3", light3);
	

}

void QtCameraSet::on_pushButton_13_2_clicked()
{
	if (ui.lE_light1->hasFocus()|| ui.lE_light2->hasFocus()||ui.lE_light3->hasFocus())
	{
		on_pB_adjustBrightness_clicked();
	}
}

//下面是232串口部分相关


void QtCameraSet::onConnectPort()
{
	if (m_serialPort == nullptr)
	{
		m_serialPort = new QSerialPort();//新串口
	}
	
	g_232Port = ui.cB_232Port->currentText().toInt();
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
				levelOut->getString(QString::fromLocal8Bit("串口打开失败，参数值仅可以保存至本地！"), 2000);
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


void QtCameraSet::initTableForAllPara()//增加调整所有相机参数的表
{
	QWidget* tab = new QWidget();//新建tab
	tab->setFont(font);//设置tab字体
	tab->setObjectName("finalTab");//tab_23170685
	ui.tabWidget_CameraVec->addTab(tab, QString::fromLocal8Bit("统一设置"));// g_vectorCamera[i]->c_CameraName);//将tab添加到左下角tabwidget boject name:tab_23170685 title:23170685

	tableWidget = new QTableWidget(tab);//tab下面加tablewidget
	tableWidget->setObjectName("finalTable");//tableWidget_23170685
	int tw = tab->width();
	int th = tab->height();
	tableWidget->setGeometry(QRect(9, 9, tab->height() + 15, tab->width() - 85));//设置widget尺寸 黑边是边界

	QStringList strlist;
	strlist << QString::fromLocal8Bit("参数") << QString::fromLocal8Bit("数值");
	tableWidget->setColumnCount(2);//两列
	tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分填充表头
	tableWidget->verticalHeader()->setDefaultSectionSize(35);//默认行高20
	tableWidget->setHorizontalHeaderLabels(strlist);//水平表头参数、数值
	tableWidget->horizontalHeader()->setVisible(true);//表头可见
	font = tableWidget->horizontalHeader()->font();//表头字体
	font.setPointSize(20);//字号
	tableWidget->horizontalHeader()->setFont(font);//设置表头字体
	tableWidget->setFont(font);//设置内容字体

	tableForAllPara(tableWidget);//初始化tabwidget

	connect(tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(onCameraCellChangeForAll(int, int)));//int-row,int-column 伴有键盘取消操作
	connect(tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(keyboardMoveOut(int, int)));
}

void QtCameraSet::receiveInfo()//read 232
{
	QByteArray info = m_serialPort->readAll();
	//QByteArray hexData = info.toHex();
	char* s = info.data();

	OPS_logger->warn("232read {}",s);
	onCircleWrite();
}

void QtCameraSet::onCircleWrite()
{

	int light2 = ui.lE_light2->text().toInt();
	int light3 = ui.lE_light3->text().toInt();

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
	else if (m_write==4)
	{

		disconnect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receiveInfo()));
		m_write = 0;

		ui.pB_adjustBrightness->setEnabled(true);
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("恭喜，亮度设置成功！"), 2000);
		levelOut->show();
	}

}
///////////////////////上面232port
void QtCameraSet::on_pB_AlgSetting_toggled(bool checked)
{

	if (checked)
	{
		ui.frame_Alg->setVisible(true);
	}
	else
	{
		ui.frame_Alg->setVisible(false);
	}
}
void QtCameraSet::on_pB_AlgCam0_clicked()
{
	ui.tabWidget_CameraVec->setCurrentIndex(0);
	onShowAlgSet();
}

void QtCameraSet::on_pB_AlgCam1_clicked()
{
	ui.tabWidget_CameraVec->setCurrentIndex(1);
	onShowAlgSet();
}

void QtCameraSet::on_pB_AlgCam2_clicked()
{
	ui.tabWidget_CameraVec->setCurrentIndex(2);
	onShowAlgSet();
}

void QtCameraSet::on_pB_AlgCam3_clicked()
{
	ui.tabWidget_CameraVec->setCurrentIndex(3);
	onShowAlgSet();
}

void QtCameraSet::on_pB_AlgCam4_clicked()//加一个按钮保存
{
	ui.tabWidget_CameraVec->setCurrentIndex(4);
	onShowAlgSet();
}