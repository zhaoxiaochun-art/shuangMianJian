#include "ResultData.h"
#include <QSettings>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <ActiveQt/QAxObject>
#include <QDesktopServices>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>
#include <QPainter>
#include <QBitmap>
#include <QPalette>
#include <QBrush>
#include <QScrollBar>
#include <QHeaderView>
#include <QCalendarWidget>
#include "AllRelayHead.h"
#include "NewUI_Demo.h"
#include "spdlog/spdlog.h"
namespace spd = spdlog;

extern bool deleteDir(const QString& path);
extern QString AppPath;
//日志工具

extern std::shared_ptr<spd::logger> SYS_logger;//系统
extern std::shared_ptr<spd::logger> ALM_logger;//报警
extern std::shared_ptr<spd::logger> OPS_logger;//操作
extern QString g_productionNumber;

ResultData::ResultData(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.pB_LeadOut->setDisabled(true);

	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	this->setWindowModality(Qt::ApplicationModal);

	QFont fo("Arial", 9);
	QFont tableHead(QString::fromLocal8Bit("幼圆"), 12);
	fo.setPointSize(9);
	ui.tableWidget->setFont(fo);//设置tableWidget字体
	int columnCount = 1;
	//title << QString::fromLocal8Bit("总数");
	//ui.tableWidget->setColumnCount(columnCount);//1列
	ui.tableWidget->horizontalHeader()->setFont(tableHead);//表头字体
	ui.tableWidget->verticalHeader()->setFont(tableHead);//表头字体
	//ui.tableWidget->setHorizontalHeaderLabels(title);//加表头
	QPalette pll = ui.tableWidget->palette();
	pll.setBrush(QPalette::Base, QBrush(QColor(255, 255, 255, 0)));
	ui.tableWidget->setPalette(pll);
	ui.tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color:rgba(85, 170, 255, 45);color: black;padding-left: 4px;border: 1px solid #6c6c6c;}");
	ui.tableWidget->horizontalHeader()->setDefaultSectionSize(150);
	ui.tableWidget->verticalHeader()->setStyleSheet("QHeaderView::section {background-color:rgba(85, 170, 255, 45);color: black;padding-left: 4px;border: 1px solid #6c6c6c;}");
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(150);

	ui.tableWidget->setColumnHidden(0, false);//不隐藏ErrorType列
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//表头
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(20);//默认行高20
	ui.tableWidget->verticalHeader()->setVisible(true);//显示行头

	//设置水平、垂直滚动条样式
	ui.tableWidget->horizontalScrollBar()->setStyleSheet("QScrollBar{background:White; height:40px;}"
		"QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
		"QScrollBar::handle:hover{background:gray;}"
		"QScrollBar::sub-line{background:transparent;}"
		"QScrollBar::add-line{background:transparent;}");
	ui.tableWidget->verticalScrollBar()->setStyleSheet("QScrollBar{background:White; width: 40px;}"
		"QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
		"QScrollBar::handle:hover{background:gray;}"
		"QScrollBar::sub-line{background:transparent;}"
		"QScrollBar::add-line{background:transparent;}");
	QPalette pal;
	pal.setColor(QPalette::Base, QColor(225, 225, 225));
	pal.setColor(QPalette::AlternateBase, QColor(255, 255, 255));
	ui.tableWidget->setPalette(pal);
	ui.tableWidget->setAlternatingRowColors(true);
	ui.tableWidget->setShowGrid(true);

	initTableWidget();

	connect(ui.pB_LeadOutClose, &QPushButton::clicked, [=]() {close(); });

	initDateEdit();
	connect(ui.dateEdit, SIGNAL(editingFinished()), this, SLOT(slotDataChanged()));
	connect(ui.dateEdit_2, SIGNAL(editingFinished()), this, SLOT(slotDataChanged_2()));

	QRegExp regx("[a-zA-Z0-9_]+$");//正则表达式QRegExp,只允许输入中文、数字、字母、下划线以及空格,[\u4e00 - \u9fa5a - zA - Z0 - 9_] + $
	ui.lE_productionNum->setValidator(new QRegExpValidator(regx, this));
	connect(ui.lE_productionNum, SIGNAL(POPUPKEYBOARD()), this, SLOT(SWITCHOSK()));
	//QString qualified = ui->lineEdit_2->text().mid(4, 10);
	//qualified.remove(QChar('_'), Qt::CaseInsensitive);
	//ui.lineEdit_3->setText(qualified);
	QString str_dt1 = ui.dateEdit->date().toString("yyyyMMdd");
	date_1 = str_dt1.toInt();
	QString str_dt2 = ui.dateEdit_2->date().toString("yyyyMMdd");
	date_2 = str_dt2.toInt();

	char* configpath = new char[MAX_PATH];
	strcpy(configpath, (AppPath + "\\ModelFile\\ProgramSet.ini").toStdString().c_str());//c_str返回char*
	QVector<std::string> v_line;//存储相机序列号
	std::ifstream cin(configpath);//将文件路径放入标准流
	delete configpath;
	std::string filename;
	std::string line;
	int r_n = 0, ipos = 0;
	if (cin) // 有该文件  
	{//确定ini文件中有多少相机配置文件，并将IP存入v_line
		while (getline(cin, line)) // line中不包括每行的换行符  
		{
			if (line.find("USER_") != std::string::npos)//std::string::npos指不存在
			{
				line.erase(0, 6);
				line.erase(line.length() - 1, 1);
				m_ls_user.push_back(line.c_str());
			}
		}
	}
	if (m_ls_user.size() != 0)
	{
		ui.cB_LeadOut->addItems(m_ls_user);
	}

	connect(ui.bG_checkBox, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),
		[=](int id, bool checked) {
		QList<QAbstractButton*> buttonsList = ui.bG_checkBox->buttons();

		if (ui.cB_data->isChecked() || ui.cB_model->isChecked() || ui.cB_sysLog->isChecked() || ui.cB_alarmLog->isChecked() || ui.cB_operationLog->isChecked())
		{
			if (ui.lineEdit->text() != "")
			{
				ui.pB_LeadOut->setEnabled(true);
				ui.pB_LeadOut->setStyleSheet("background-color: rgba(0, 170, 0, 125);font-size:20pt");
			}
		}
		else
		{
			ui.pB_LeadOut->setEnabled(false);
			ui.pB_LeadOut->setStyleSheet("font-size:14pt");
		}
	});
}


void ResultData::initDateEdit()
{
	QCalendarWidget* m_pCalendarWidget = new QCalendarWidget(this);
	m_pCalendarWidget->setFixedSize(600, 600); //日历控件的显示大小
	m_pCalendarWidget->setGridVisible(true);
	ui.dateEdit->setCalendarPopup(true); //使用该句可以直接调用日历控件
	ui.dateEdit->setCalendarWidget(m_pCalendarWidget);
	ui.dateEdit->setDate(QDate::currentDate());
	connect(m_pCalendarWidget, SIGNAL(clicked(QDate)), this, SLOT(deLoseFocus()));

	m_pCalendarWidget = new QCalendarWidget(this);
	m_pCalendarWidget->setFixedSize(600, 600); //日历控件的显示大小
	m_pCalendarWidget->setGridVisible(true);
	ui.dateEdit_2->setCalendarPopup(true); //使用该句可以直接调用日历控件
	ui.dateEdit_2->setCalendarWidget(m_pCalendarWidget);
	ui.dateEdit_2->setDate(QDate::currentDate());
	connect(m_pCalendarWidget, SIGNAL(clicked(QDate)), this, SLOT(deLoseFocus()));
}

void ResultData::slotDataChanged()
{
	QString str_dt1 = ui.dateEdit->date().toString("yyyyMMdd");
	date_1 = str_dt1.toInt();
	QString str_dt2 = ui.dateEdit_2->date().toString("yyyyMMdd");
	date_2 = str_dt2.toInt();
	if (date_1 > date_2)
	{
		ui.dateEdit->setDate(ui.dateEdit_2->date());
		str_dt1 = ui.dateEdit->date().toString("yyyyMMdd");
		date_1 = str_dt1.toInt();
		showMsgBox(QMessageBox::Question, "输入错误", "<img src = './ico/warning.png'/>\t开始日期不得晚于结束日期！已将您的设置改为结束日期，您可以重新设置~", "我知道了", "");//全是中文
	}
}
void ResultData::slotDataChanged_2()
{
	QString str_dt1 = ui.dateEdit->date().toString("yyyyMMdd");
	date_1 = str_dt1.toInt();
	QString str_dt2 = ui.dateEdit_2->date().toString("yyyyMMdd");
	date_2 = str_dt2.toInt();
	if (date_1 > date_2)
	{
		ui.dateEdit_2->setDate(ui.dateEdit->date());
		str_dt2 = ui.dateEdit_2->date().toString("yyyyMMdd");
		date_2 = str_dt2.toInt();
		showMsgBox(QMessageBox::Question, "输入错误", "<img src = './ico/warning.png'/>\t结束日期不得早于开始日期！已将您的设置改为开始日期，您可以重新设置~", "我知道了", "");//全是中文
	}
}

void ResultData::deLoseFocus()//失去焦点从而得到finishediting事件
{
	ui.dateEdit->clearFocus();
	ui.dateEdit_2->clearFocus();
}


ResultData::~ResultData()
{
}

void ResultData::initTableWidget()
{
	QSettings FinalDir(AppPath + "/result/AllResult.ini", QSettings::IniFormat);//所有结果文件
	QStringList str = FinalDir.childGroups();
	QStringList str1;
	str1 << "Zero" << str;//加个0行，用于比对

	int count = str1.size();
	ui.tableWidget->setRowCount(count);//设置行数，第一行隐藏，用于与之后的比对
	ui.tableWidget->setVerticalHeaderLabels(str1);//加水平表头 每行加日期结果
	//记得弄出来这个//
	ui.tableWidget->setRowHidden(0, true);//隐藏ErrorType列//隐藏0行

	//ui.tableWidget->setItem(0, 0, new QTableWidgetItem(titleEng.at(0)));//total,zero加第一个数

	for (int i = 1; i < count; i++)
	{
		QString One = str1.at(i);//节点
		//ui.tableWidget->insertRow(i);//加一行

		FinalDir.beginGroup(str1.at(i));
		QStringList str2 = FinalDir.allKeys();    // 获取所有的key
		foreach(QString key, str2)
		{

			QString value = FinalDir.value(key).toString();  // 直接用 key 获取数据
			int j = 0;
			for (j; j < titleEng.size(); j++)
			{
				if (key == titleEng.at(j))
				{
					//QString total = FinalDir.value(One + "/" + key, 0).toString();//第二个数是默认数值，如果不存在就用默认数值，下同 ;//每个总数
					ui.tableWidget->setItem(i, j, new QTableWidgetItem(value));//添加日期result行
					break;
				}
			}
			if (j == titleEng.size())
			{
				titleEng.append(key);
				ui.tableWidget->insertColumn(j);
				ui.tableWidget->setHorizontalHeaderLabels(titleEng);//加表头
				//QString total = FinalDir.value(One + "/" + key, 0).toString();//第二个数是默认数值，如果不存在就用默认数值，下同 ;//每个总数
				ui.tableWidget->setItem(0, j, new QTableWidgetItem(key));//添加日期result行
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(value));//total,zero加第一个数
			}

		}

		FinalDir.endGroup(); // 结束掉Group	

	}
	// 	如果是没有被编辑过的单元格，将不能用ui->tableWidget->item(i, j)->text()访问，因为该指针为空。
	// 		如果被编辑过，但内容仍然是空(例如text() == tr("")), 则要进行这一步的判断。
	// 		所以一个好的编码习惯应该是：在使用指针之前，要进行指针是否为null的判断。
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		for (int j = 0; j < ui.tableWidget->columnCount(); j++)
		{
			if (ui.tableWidget->item(i, j) == NULL ||             //判断指向该cell的指针是否为空
				(ui.tableWidget->item(i, j) &&
					ui.tableWidget->item(i, j)->text() == tr(""))) //判断该cell的text是否为空
			{
				ui.tableWidget->setItem(i, j, new QTableWidgetItem("0"));
			}
		}
	}

}

void ResultData::on_lineEdit_textChanged(const QString& arg1)
{
	if (arg1 == "")
	{
		ui.pB_LeadOut->setDisabled(true);
		ui.pB_LeadOut->setStyleSheet("font-size:14pt");
	}
	else {
		if (ui.cB_data->isChecked() || ui.cB_model->isChecked() || ui.cB_sysLog->isChecked() || ui.cB_alarmLog->isChecked() || ui.cB_operationLog->isChecked())
		{
			ui.pB_LeadOut->setDisabled(false);
			ui.pB_LeadOut->setStyleSheet("background-color: rgba(0, 170, 0, 125);font-size:20pt");

		}
	}
}

void ResultData::SWITCHOSK()//快捷键
{
	keybd_event(0x11, 0, 0, 0);
	keybd_event(0x5B, 0, 0, 0);
	keybd_event(0x4F, 0, 0, 0);
	keybd_event(0x11, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x5B, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(0x4F, 0, KEYEVENTF_KEYUP, 0);//ctrl+win+o切换
}

void ResultData::on_pB_Choose_clicked()
{
	OPS_logger->info("选择目录按下");
	QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择目录"), "C:/",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	ui.lineEdit->setText(dir);
	QByteArray ba = dir.toLatin1();//QString to char*
	const char* s = ba.data();
	SYS_logger->info("选择目录{}",s);
}

bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist);
bool ResultData::copyDirectoryFiles_Log1(const QString& fromDir, const QString& toDir, bool coverFileIfExist)//此处最后一个参数没啥用
{//系统日志拷贝
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if (!targetDir.exists()) {    /**< 如果目标目录不存在,则进行创建 */
		if (!targetDir.mkpath(targetDir.absolutePath()))
		{
			return false;
		}
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList)
	{
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;
		targetDir.remove(fileInfo.fileName());
	}
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    /**< 当为目录时,递归的进行copy */
			if (!copyDirectoryFiles_Log1(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else {            /**< 当允许覆盖操作时,将旧文件进行删除操作 */
			if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
				targetDir.remove(fileInfo.fileName());
			}

			/// 进行文件copy
			QString str = fileInfo.fileName();
			QString strCompare = str.mid(13, 10);//System_daily_2020_07_07_16_47
			strCompare.remove(QChar('_'), Qt::CaseInsensitive);//去掉下划线

			int i_compareDate = strCompare.toInt();
			if (i_compareDate<date_1 || i_compareDate>date_2)
			{
				continue;//退出本次循环，break退出for的所有循环
			}

			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

bool ResultData::copyDirectoryFiles_Log2(const QString& fromDir, const QString& toDir, bool coverFileIfExist)//此处最后一个参数没啥用
{//报警日志拷贝
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if (!targetDir.exists()) {    /**< 如果目标目录不存在,则进行创建 */
		if (!targetDir.mkpath(targetDir.absolutePath()))
		{
			return false;
		}
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList)
	{
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;
		targetDir.remove(fileInfo.fileName());
	}
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    /**< 当为目录时,递归的进行copy */
			if (!copyDirectoryFiles_Log2(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else {            /**< 当允许覆盖操作时,将旧文件进行删除操作 */
			if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
				targetDir.remove(fileInfo.fileName());
			}

			/// 进行文件copy
			QString str = fileInfo.fileName();
			QString strCompare = str.mid(12, 10);//Alarm_daily_2020_07_07_16_47
			strCompare.remove(QChar('_'), Qt::CaseInsensitive);//去掉下划线

			int i_compareDate = strCompare.toInt();
			if (i_compareDate<date_1 || i_compareDate>date_2)
			{
				continue;//退出本次循环，break退出for的所有循环
			}

			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

bool ResultData::copyDirectoryFiles_Log3(const QString& fromDir, const QString& toDir, bool coverFileIfExist)//此处最后一个参数没啥用
{//操作日志拷贝
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if (!targetDir.exists()) {    /**< 如果目标目录不存在,则进行创建 */
		if (!targetDir.mkpath(targetDir.absolutePath()))
		{
			return false;
		}
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList)
	{
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;
		targetDir.remove(fileInfo.fileName());
	}
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    /**< 当为目录时,递归的进行copy */
			if (!copyDirectoryFiles_Log3(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else {            /**< 当允许覆盖操作时,将旧文件进行删除操作 */
			if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
				targetDir.remove(fileInfo.fileName());
			}

			/// 进行文件copy
			QString str = fileInfo.fileName();
			QString strCompare = str.mid(16, 10);//Operation_daily_2020_07_07_16_47
			strCompare.remove(QChar('_'), Qt::CaseInsensitive);//去掉下划线

			int i_compareDate = strCompare.toInt();
			if (i_compareDate<date_1 || i_compareDate>date_2)
			{
				continue;//退出本次循环，break退出for的所有循环
			}

			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

void ResultData::on_pB_LeadOut_clicked()
{
	QString path = ui.lineEdit->text();//目标路径
	//QString tarDir = path + "/EXPORT/AllResult.csv";
	QString tarDir = path + "/EXPORT";
	QFileInfo fileInfo(tarDir);//文件是否存在
	if (fileInfo.exists())
	{
		int ret = showMsgBox(QMessageBox::Question, "冲突提示", "<img src = './ico/warning.png'/>\t目标目录文件(夹)已存在，是否替换？", "是", "否");//全是中文
		if (ret == QMessageBox::Yes)
		{

			QString userName = ui.cB_LeadOut->currentText();
			deleteDir(path + "/EXPORT");
			if (ui.cB_data->isChecked())
			{
				if(exportExecl(ui.tableWidget, path + "/EXPORT", "AllResult"))
				{
					SYS_logger->info("检测数据已导出");
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，检测数据已导出！"), 2000);
					levelOut->show();
				}
				else
				{
					SYS_logger->info("无符合条件的检测数据导出");			
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("注意，无符合条件的检测数据导出！"), 2000);
					levelOut->show();
				}
			}
			if (ui.cB_model->isChecked())
			{

				copyDirectoryFiles(AppPath + "/ModelFile", path + "/EXPORT/ModelFile", 1);//数据全部导出
				SYS_logger->info("模板已导出");
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("恭喜，模板已导出！"), 2000);
				levelOut->show();
			}
			
			if (ui.cB_sysLog->isChecked())
			{
				if (ui.cB_LeadOut->currentIndex() == 0)
				{
					int userCount = m_ls_user.size();
					for (int i=0;i<userCount;i++)
					{
						QString tempName = m_ls_user.at(i);
						copyDirectoryFiles_Log1(AppPath + "/Logs/" + tempName + "/System", path + "/EXPORT/Logs/" + tempName + "/System", 1);
					}						
					SYS_logger->info("系统日志已导出");
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，系统日志已导出！"), 2000);
					levelOut->show();
				}
				else
				{ 
				copyDirectoryFiles_Log1(AppPath + "/Logs/" + userName + "/System", path + "/EXPORT/Logs/" + userName + "/System", 1);//数据全部导出
				
				SYS_logger->info("系统日志已导出");
				levelOut = new WindowOut;
				levelOut->getString(QString::fromLocal8Bit("恭喜，系统日志已导出！"), 2000);
				levelOut->show();
				}
				}
			if (ui.cB_alarmLog->isChecked())
			{
				if (ui.cB_LeadOut->currentIndex() == 0)
				{
					int userCount = m_ls_user.size();
					for (int i = 0; i < userCount; i++)
					{
						QString tempName = m_ls_user.at(i);
						copyDirectoryFiles_Log2(AppPath + "/Logs/" + tempName + "/Alarm", path + "/EXPORT/Logs/" + tempName + "/Alarm", 1);
					}
					SYS_logger->info("报警日志已导出");
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，报警日志已导出！"), 2000);
					levelOut->show();
				}
				else
				{
					copyDirectoryFiles_Log2(AppPath + "/Logs/" + userName + "/Alarm", path + "/EXPORT/Logs/" + userName + "/Alarm", 1);//数据全部导出
					SYS_logger->info("报警日志已导出");
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，报警日志已导出！"), 2000);
					levelOut->show();
				}
				}
			if (ui.cB_operationLog->isChecked())
			{
				if (ui.cB_LeadOut->currentIndex() == 0)
				{
					int userCount = m_ls_user.size();
					for (int i = 0; i < userCount; i++)
					{
						QString tempName = m_ls_user.at(i);
						copyDirectoryFiles_Log3(AppPath + "/Logs/" + tempName + "/Operation", path + "/EXPORT/Logs/" + tempName + "/Operation", 1);
					}
					SYS_logger->info("操作日志已导出");
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，操作日志已导出！"), 2000);
					levelOut->show();
				}
				else
				{
					copyDirectoryFiles_Log3(AppPath + "/Logs/" + userName + "/Operation", path + "/EXPORT/Logs/" + userName + "/Operation", 1);//数据全部导出
					SYS_logger->info("操作日志已导出");
					levelOut = new WindowOut;
					levelOut->getString(QString::fromLocal8Bit("恭喜，操作日志已导出！"), 2000);
					levelOut->show();
				}
				}
			return;
		}
		else
		{
			return;
		}
	}

	QString userName = ui.cB_LeadOut->currentText();
	deleteDir(path + "/EXPORT");
	if (ui.cB_data->isChecked())
	{
		if (exportExecl(ui.tableWidget, path + "/EXPORT", "AllResult"))
		{
			SYS_logger->info("检测数据已导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，检测数据已导出！"), 2000);
			levelOut->show();
		}
		else
		{
			SYS_logger->info("无符合条件的检测数据导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("注意，无符合条件的检测数据导出！"), 2000);
			levelOut->show();
		}
	}
	if (ui.cB_model->isChecked())
	{

		copyDirectoryFiles(AppPath + "/ModelFile", path + "/EXPORT/ModelFile", 1);//数据全部导出
		SYS_logger->info("模板已导出");
		levelOut = new WindowOut;
		levelOut->getString(QString::fromLocal8Bit("恭喜，模板已导出！"), 2000);
		levelOut->show();
	}

	if (ui.cB_sysLog->isChecked())
	{
		if (ui.cB_LeadOut->currentIndex() == 0)
		{
			int userCount = m_ls_user.size();
			for (int i = 0; i < userCount; i++)
			{
				QString tempName = m_ls_user.at(i);
				copyDirectoryFiles_Log1(AppPath + "/Logs/" + tempName + "/System", path + "/EXPORT/Logs/" + tempName + "/System", 1);
			}
			SYS_logger->info("系统日志已导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，系统日志已导出！"), 2000);
			levelOut->show();
		}
		else
		{
			copyDirectoryFiles_Log1(AppPath + "/Logs/" + userName + "/System", path + "/EXPORT/Logs/" + userName + "/System", 1);//数据全部导出
			SYS_logger->info("系统日志已导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，系统日志已导出！"), 2000);
			levelOut->show();
		}
	}
	if (ui.cB_alarmLog->isChecked())
	{
		if (ui.cB_LeadOut->currentIndex() == 0)
		{
			int userCount = m_ls_user.size();
			for (int i = 0; i < userCount; i++)
			{
				QString tempName = m_ls_user.at(i);
				copyDirectoryFiles_Log2(AppPath + "/Logs/" + tempName + "/Alarm", path + "/EXPORT/Logs/" + tempName + "/Alarm", 1);
			}
			SYS_logger->info("报警日志已导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，报警日志已导出！"), 2000);
			levelOut->show();
		}
		else
		{
			copyDirectoryFiles_Log2(AppPath + "/Logs/" + userName + "/Alarm", path + "/EXPORT/Logs/" + userName + "/Alarm", 1);//数据全部导出
			SYS_logger->info("报警日志已导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，报警日志已导出！"), 2000);
			levelOut->show();
		}
	}
	if (ui.cB_operationLog->isChecked())
	{
		if (ui.cB_LeadOut->currentIndex() == 0)
		{
			int userCount = m_ls_user.size();
			for (int i = 0; i < userCount; i++)
			{
				QString tempName = m_ls_user.at(i);
				copyDirectoryFiles_Log3(AppPath + "/Logs/" + tempName + "/Operation", path + "/EXPORT/Logs/" + tempName + "/Operation", 1);
			}
			SYS_logger->info("操作日志已导出");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，操作日志已导出！"), 2000);
			levelOut->show();
		}
		else
		{
			copyDirectoryFiles_Log3(AppPath + "/Logs/" + userName + "/Operation", path + "/EXPORT/Logs/" + userName + "/Operation", 1);//数据全部导出
			SYS_logger->info("操作日志已导出");	
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("恭喜，操作日志已导出！"), 2000);
			levelOut->show();
		}
	}
	//showMsgBox(QMessageBox::Information, "操作成功", "恭喜，数据文件已成功导出", "我知道了", "");//全是中文



}


bool ResultData::exportExecl(QTableWidget* tableWidget, QString dirName, QString fileName)
{
	QDir dir(dirName);
	QString fullDirName = dirName;
	if (!dir.exists())
	{
		dir.mkpath(fullDirName);
	}

	QString dirFile = fullDirName + "/" + fileName + ".csv";
	QFile file(dirFile);
	bool ret = file.open(QIODevice::Truncate | QIODevice::ReadWrite);
	if (!ret)
	{
		qDebug() << "open failure";

		return ret;
	}

	QTextStream stream(&file);
	QString conTents;
	// 写入头
	QHeaderView* header = tableWidget->horizontalHeader();
	if (NULL != header)
	{
		conTents = ",";//空一个格
		for (int i = 0; i < header->count(); i++)
		{
			QTableWidgetItem* item = tableWidget->horizontalHeaderItem(i);
			if (NULL != item)
			{
				conTents += item->text() + ",";
			}
		}
		conTents += "\n";
	}

	// 写内容
	int wrongFlag = 0;//没有符合条件导出标志
	for (int row = 1; row < tableWidget->rowCount(); row++)//隐藏0行
	{
		QTableWidgetItem* item = tableWidget->verticalHeaderItem(row);

		if (NULL != item)
		{
			QString str;//↓↓↓↓↓↓↓↓↓↓筛选符合条件的导出
			str = item->text();
			QString strCompare;
			strCompare = str.mid(7, 10);//result_1234_56_78
			strCompare.remove(QChar('_'), Qt::CaseInsensitive);//去掉下划线
			int i_compareDate = strCompare.toInt();
			if (i_compareDate<date_1 || i_compareDate>date_2)
			{
				wrongFlag++;//没有符合条件导出标志
				continue;//退出本次循环，break退出for的所有循环

			}

			if (ui.lE_productionNum->text() != "")
			{
				QString PNCompare;
				//int PNLength = ui.lE_productionNum->text().size();//eg:Result_2020_02_29_23_47_1234567890abc.ini
				PNCompare = str.mid(24);

				if (PNCompare != ui.lE_productionNum->text())
				{
					wrongFlag++;//没有符合条件导出标志
					continue;//退出本次循环，break退出for的所有循环
				}
			}
			
			//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑筛选符合条件的导出
			conTents += item->text() + ",";
		}
		for (int column = 0; column < tableWidget->columnCount(); column++)
		{
			QTableWidgetItem* item = tableWidget->item(row, column);

			if (NULL != item)
			{
				QString str;
				str = item->text();
				str.replace(",", " ");
				conTents += str + ",";
			}
		}
		conTents += "\n";
	}
	stream << conTents;

	file.close();
	int tbRowCount = tableWidget->rowCount();//0行隐藏了
	if (wrongFlag+1== tableWidget->rowCount())
	{
		return false;
	}
	return true;
}

int ResultData::showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str)//全是中文
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