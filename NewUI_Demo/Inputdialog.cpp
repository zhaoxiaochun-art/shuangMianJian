#include "Inputdialog.h"
#include "ui_Inputdialog.h"
#include <Tlhelp32.h>//terminate process
extern QString AppPath;
#include <QMessageBox>
#include <QIntValidator>
#include <QDir>
#include <QFileInfoList>
#include <QBitmap>
#include <QPainter>
#include "QDesktopWidget"//获取屏幕尺寸

InputDialog::InputDialog(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::InputDialog)
{
	ui->setupUi(this);

	QDesktopWidget* desktopWidget = QApplication::desktop();
	QRect deskRect = desktopWidget->availableGeometry();  //可用区域
	QRect screenRect = desktopWidget->screenGeometry();  //屏幕区域
	int w = screenRect.width();
	int h = screenRect.height();
	int w2 = this->width();
	int h2 = this->height();
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
	this->setGeometry((w - this->width()) / 2, 320, this->width(), this->height());

	//圆角👇
	QBitmap bmp(this->size());
	bmp.fill();
	QPainter p(&bmp);
	p.setPen(Qt::NoPen);
	p.setBrush(Qt::black);
	p.drawRoundedRect(bmp.rect(), 5, 5);
	setMask(bmp);

	ui->lineEdit->setFocus();
	QRegExp regx("[a-zA-Z0-9_]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui->lineEdit->setValidator(new QRegExpValidator(regx, this));
	connect(ui->lineEdit, &QLineEdit::returnPressed, [=]() {changingModel(); });
	connect(ui->pushButton, &QPushButton::clicked, [=]() {changingModel(); });//利用lambda表达式可用
	connect(ui->pB_Exit, &QPushButton::clicked, [=]() {close(); });
	SWITCHOSK();
}

InputDialog::~InputDialog()
{
	delete ui;
}

int InputDialog::showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str)//全是中文
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
void InputDialog::SWITCHOSK()//快捷键
{
	keybd_event(0x11, 0, 0, 0);
	keybd_event(0x5B, 0, 0, 0);
	keybd_event(0x4F, 0, 0, 0);
	keybd_event(0x11, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x5B, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x4F, 0, KEYEVENTF_KEYUP, 0);//ctrl+win+o切换
}
void InputDialog::SHOWOSK()
{
	//The code from david!!!
	PVOID OldValue;
	BOOL bRet = Wow64DisableWow64FsRedirection(&OldValue);
	//QString csProcess = "C:/Program/FilesCommon/Filesmicrosoft/sharedink/TabTip.exe";
	QString csProcess = AppPath + "/osk.exe";
	QString params = "";
	ShellExecute(NULL, L"open", (LPCWSTR)csProcess.utf16(), (LPCWSTR)params.utf16(), NULL, SW_SHOWNORMAL);
	if (bRet)
	{
		Wow64RevertWow64FsRedirection(OldValue);
	}

}

void InputDialog::HideOSK()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//现在我们获得了所有进程的信息。
//将从hSnapShot中抽取数据到一个PROCESSENTRY32结构中
//这个结构代表了一个进程，是ToolHelp32 API的一部分。
//抽取数据靠Process32First()和Process32Next()这两个函数。
//这里我们仅用Process32Next()，他的原形是：
//BOOL WINAPI Process32Next(HANDLE hSnapshot,LPPROCESSENTRY32 lppe);
//我们程序的代码中加入：
	PROCESSENTRY32* processInfo = new PROCESSENTRY32;
	// 必须设置PROCESSENTRY32的dwSize成员的值 ;
	processInfo->dwSize = sizeof(PROCESSENTRY32);
	int index = 0;
	//这里我们将快照句柄和PROCESSENTRY32结构传给Process32Next()。
	//执行之后，PROCESSENTRY32 结构将获得进程的信息。我们循环遍历，直到函数返回FALSE。
	//printf("****************开始列举进程****************/n");
	int ID = 0;
	while (Process32Next(hSnapShot, processInfo) != FALSE)
	{
		index++;
		//printf("****************** %d ******************/n",index);
		//printf("PID       Name      Current Threads/n");
		//printf("%-15d%-25s%-4d/n",processInfo->th32ProcessID,processInfo->szExeFile,processInfo->cntThreads);
		int size = WideCharToMultiByte(CP_ACP, 0, processInfo->szExeFile, -1, NULL, 0, NULL, NULL);
		char* ch = new char[size + 1];
		if (WideCharToMultiByte(CP_ACP, 0, processInfo->szExeFile, -1, ch, size, NULL, NULL))
		{
			if (strstr(ch, "osk.exe"))//使用这段代码的时候只需要改变"cmd.exe".将其改成你要结束的进程名就可以了。//win10自带的无修改权限，所以进程杀不掉。用的第三方exe
			{
				ID = processInfo->th32ProcessID;
				// qDebug()<<"ID ="<<ID;
				HANDLE hProcess;
				// 现在我们用函数 TerminateProcess()终止进程：
				// 这里我们用PROCESS_ALL_ACCESS
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ID);
				//if(hProcess==NULL)
				//{
				  //  printf("Unable to get handle of process: ");
				  //  printf("Error is: %d",GetLastError());
				//}
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}
		}
	}
	CloseHandle(hSnapShot);
	delete processInfo;
}

void InputDialog::changingModel()
{
	if (ui->lineEdit->text().isEmpty())//未输入字符
	{
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("模板名称不可以为空！"), 2000);
		levelOut->show();
		ui->lineEdit->setFocus();
	}
	else//输入了字符，最大15个，只支持字母数字和_,新机器上需要调默认输入状态为英文
	{
		QString str = ui->lineEdit->text();
		QDir dir(AppPath + "\\ModelFile");
		dir.setFilter(QDir::Dirs);//筛选目录
		QFileInfoList list = dir.entryInfoList();//文件信息list

		int file_count = list.count();
		QStringList string_list;
		int i;//j用于标记是否存在默认模板
		for (i = 0; i < file_count; i++)
		{
			QFileInfo file_info = list.at(i);
			QString folderName = file_info.fileName();
			if (folderName == str)//重名
			{
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("模板名称不可以与现有模板名称重复！"), 2000);
				levelOut->show();
				ui->lineEdit->setFocus();
				return;
			}
		}

		close();
		SWITCHOSK();
		//HideOSK();
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("恭喜！您的操作已成功！"), 2000);
		levelOut->show();
		emit toFather(str);
	}
}

void InputDialog::closeEvent(QCloseEvent* event) //窗口关闭Event重写
{
	SWITCHOSK();
	//HideOSK();
	this->close();
}
