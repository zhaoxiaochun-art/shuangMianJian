#pragma once

#include <QDialog>
#include "ui_QtCameraSet.h"
#include "AllRelayHead.h"
#include "MultThread_Run.h"
#include "Inputdialog.h"
#include "WindowOut.h"
#include <QTreeWidgetItem>
#include <QtSerialPort/QSerialPort>

class MyLabel : public QLabel
{
	Q_OBJECT
signals:
	void SWITCHTABPAGE(int);

private:
	QSize size_ori;
	QPoint pos_ori;
	int m_iIndex = -1;
public:
	bool b_scale = false;
	MyLabel(QWidget *parent,int i=-1)
		: QLabel(parent)
	{
		m_iIndex = i;
	};
public slots:
	void showimg(Mat img)
	{
		int zz = this->frameWidth();
		QSize ss = this->size();
		ss.setWidth(ss.width() - zz * 2);
		ss.setHeight(ss.height() - zz * 2);
		Mat imgsend;
		cv::resize(img, imgsend, Size(ss.width(), ss.height()));
		if (img.channels()==1)
		{
			cv::cvtColor(imgsend, imgsend, COLOR_GRAY2RGB);//更改格式
		}
		else if (img.channels()==3)
		{
			cv::cvtColor(imgsend, imgsend, COLOR_BGR2RGB);//更改格式
		}
		QImage disImage = QImage((const unsigned char*)(imgsend.data), imgsend.cols, imgsend.rows, imgsend.step, QImage::Format_RGB888);
		this->setPixmap(QPixmap::fromImage(disImage));
		return;
	}
	void mousePressEvent(QMouseEvent*)
	{
		if (!b_scale)
		{
			size_ori = this->size();
			this->resize(((QGroupBox*)this->parent())->size());
			pos_ori = this->pos();
			this->move(QPoint(0, 0));
			this->raise();
			b_scale = true;
			emit SWITCHTABPAGE(m_iIndex);
		}
		else
		{
			this->resize(size_ori);
			this->move(pos_ori);
			b_scale = false;
		}
	};
};

class QtCameraSet : public QDialog
{
	Q_OBJECT
signals:
	void STARTGRABBING(int, bool);
	void SHOWEVERYPLCVALUE(DataToPC_typ);
	void keyBoardOut();
public:
	QtCameraSet(QWidget *parent = Q_NULLPTR);
	~QtCameraSet();

	void initCameraSetTableWidget(QTableWidget *);
	void initListWidgetofModel();
	bool compareModels();
	int showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//全是中文
	void SHOWOSK();//键盘显示
	bool IsNumber(QString& qstrSrc);//是否是纯数字

private:
	Ui::QtCameraSet ui;
	bool b_changeCamera;//相机参数有人为修改
	bool b_autoAutoChange;
	QFont font;
	bool SaveSetParam();
	QTableWidget* tableWidget;

	WindowOut *levelOut;//show默认为非模态modal，如果是局部变量会闪现消失

	QLabel* label_Offline;
	QTreeWidgetItem* checkPermissionGroup;
	int m_iTabCurrentIndex = 0;
	InputDialog* inputModel; 
	QString m_SelectedName;

	DataToPC_typ *m_data;	//获取的PLC数据
	QSerialPort *m_serialPort=nullptr;
	int m_write=0;
public slots:
	//模拟工作状态开始采集
	void StartGrab(bool);
	void StopGrab();
	void onShowAlgSet();
	//相机连续采集
	void onContinueGrab(bool);
	//相机参数修改响应函数
	void onCameraCellChange(int, int);
	void onCameraCellChangeForAll(int, int);
	void modelApply();//模板应用按钮槽函数
	void modelDelete();//删除模板文件和listwidget对应的item
	void modelChangeName();
	void modelAdd();
	void applyNameChange(QString str);
	void onCameraKeyboardOut(int r, int c);//
	void SyncAnotherStartGrab(bool);//

	void MoveOut();//键盘弹出
	void MoveOutWhenWrite();
	void MoveOut_1();//键盘弹出
	void switchTabPage(int);
	void showCurrentImage(int);

	void SWITCHTABANDOSK();
	void SWITCHOSK();//快捷键

	void on_lE_SetUserName_textChanged(const QString &arg1);//用户名
	void on_lE_SetUserSecretNum_textChanged(const QString &arg1);//密码

	void on_pB_brightness_toggled(bool checked);//亮度调节
	void on_pB_AlgSetting_toggled(bool checked);//算法设置选项
	void on_pB_AlgCam0_clicked();
	void on_pB_AlgCam1_clicked();
	void on_pB_AlgCam2_clicked();
	void on_pB_AlgCam3_clicked();
	void on_pB_AlgCam4_clicked();

	void receiveInfo();//read 232
	void onCircleWrite();//write 232

//👇👇👇👇👇树型多选
public: //申明初始化函数
	void updateParentItem(QTreeWidgetItem* item);
	void checkPermission();
	void initTableWidget();

	void savePLCParaInPLCTxt();//将combobox修改的phototimes写入txt
	void initContinueKick();//连剔4个widgets

	void initFrame_light();//灯光调整
	void onConnectPort();//232连接

	void initTableForAllPara();//增加一调整全局相机的表格
	void tableForAllPara(QTableWidget* );
public slots:   //申明信号与槽,当树形控件的子选项被改变时执行
	void onTreeItemChanged(QTreeWidgetItem* item);
	void updateCheckPermission(const QString& str);//切换用户时更新表
	void initTableOfUserPermission();
	void btn_Enabled(int i);//用户部分按钮是否可用
	void addUser();
	void on_pB_Users_Delete_clicked();
	void selectedName(int r, int c);
	void on_lE_SetUserSecretNum_returnPressed();
	void on_lE_SetUserName_returnPressed();
	void slotCb_flash(bool b);//
	void slotCb_feed(bool b);//
//👆👆👆👆👆
	//void findTreeItemText();//找特定名字的treeitem

	void closeEvent(QCloseEvent*);

	void keyboardMoveOut(int, int);//单击相机参数键盘弹出

	void on_pb_cmdParaSave_clicked();//保存PLC参数到本机plc.txt
	void on_pb_cmdParaLoad_clicked();//将本机plc.txt内PLC参数写入到PLC

//确定修改后键盘隐藏
	void on_pB_changeIPPort_clicked();
	void on_pB_ContinueKickOK_clicked();//保存连剔
	void on_pB_ContinueKickCancel_clicked();//取消连剔
	void on_pB_adjustBrightness_clicked();//保存亮度调节
	void on_pushButton_13_2_clicked();//通过键盘确定保存亮度调节
#ifdef PLCCONNECT
	void onReadPLCParam(bool);
	void onStartStore(bool);
	void onSendPLCCommand(bool);
	DataFromPC_typ getPCData();//PC数值发送给PLC
	void getPLCData(DataToPC_typ);
public:
	MESSAGE_HANDLER ShowFunc(void* context, DataToPC_typ);
	bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist);
	bool deleteDir(const QString& path);
	void connectBtnGroup();
	int modelID = 0;
	bool DismissParamChange();
#endif
};
