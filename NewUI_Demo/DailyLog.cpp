#include "DailyLog.h"
#include "AllRelayHead.h"
#include <QScrollBar>

extern QString logpath1;
extern QString logpath2;
extern QString logpath3;

DailyLog::DailyLog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	//this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowCloseButtonHint);
	//this->setWindowOpacity(transLevel / 100);//透明度
	this->move(0, 0);
	//this->setWindowModality(Qt::ApplicationModal);
	//connect(ui.sld_transParent, &QSlider::valueChanged, [=]() {
	//	transLevel = ui.sld_transParent->value();
	//	this->setWindowOpacity(transLevel / 100.0);//透明度
	//	});//利用lambda表达式可用

	//QWidget::installEventFilter(this);//为这个窗口安装过滤器  

	connect(ui.pB_closeLog, &QPushButton::clicked, [=]() {
		this->close();
	});

	//ui.comboBox->index(0)->setStyleSheet();
	connect(ui.comboBox, QOverload<int>::of(&QComboBox::activated),
		[=](int index) {
		for (int i = 0; i < logLineCount; i++)
		{
			ui.tableWidget->removeRow(0);
		}

		logLineCount = 0;
		ui.tableWidget->setColumnWidth(2, 435);
		myReadLine(); });
	/*connect(ui.comboBox, QOverload<const QString&>::of(&QComboBox::activated),
		[=](const QString& text) {
			for (int i = 0; i < logLineCount; i++)
			{
				ui.tableWidget->setRowHidden(i, true);//显示所有行
			}
			if (text !="all")
			{
				QList <QTableWidgetItem*> item = ui.tableWidget->findItems(text, Qt::MatchContains);
				if (!item.isEmpty()) { //不为空
					for (int i = 0; i < item.count(); i++) {
						ui.tableWidget->setRowHidden(item.at(i)->row(), false);//item.at(i).row()输出行号
					}
				}
			}
			else
			{
				for (int i = 0; i < logLineCount; i++)
				{
					ui.tableWidget->setRowHidden(i, false);//显示所有行
				}
			}
		});*/

		/*	ui.sld_transParent->setStyleSheet("  \
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
				height: 20px;\
				width:32px;\
				border-image: url(./ico/btn.png);\
				margin: -13 0px; \
			}\
			");*/

			//设置水平、垂直滚动条样式
	ui.tableWidget->horizontalScrollBar()->setStyleSheet("QScrollBar{background:white; width: 40px;}"
		"QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
		"QScrollBar::handle:hover{background:gray;}"
		"QScrollBar::sub-line{background:transparent;}"
		"QScrollBar::add-line{background:transparent;}");
	ui.tableWidget->verticalScrollBar()->setStyleSheet("QScrollBar{background:white; width: 40px;}"
		"QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
		"QScrollBar::handle:hover{background:gray;}"
		"QScrollBar::sub-line{background:transparent;}"
		"QScrollBar::add-line{background:transparent;}");

	logLineCount = 0;
	initLogLine();
	myReadLine();
}

DailyLog::~DailyLog()
{
}

void DailyLog::on_pB_Refresh_clicked()
{
	for (int i = 0; i < logLineCount; i++)
	{
		ui.tableWidget->removeRow(0);
	}

	myReadLine();

	levelOut = new WindowOut;
	levelOut->getString(QString::fromLocal8Bit("日志已刷新！"), 2000);
	levelOut->show();
}
void DailyLog::myReadLine()
{

	logLineCount = 0;
	ui.tableWidget->setColumnWidth(2, 435);

	QString logFilePath = "";
	if (ui.comboBox->currentIndex()==0)
	{
		logFilePath=logpath1;
	}
	else if (ui.comboBox->currentIndex() == 1)
	{
		logFilePath = logpath2;
	}
	else
	{
		logFilePath = logpath3;
	}
	QFile fileOpen(logFilePath);
	if (!fileOpen.open(QFile::ReadOnly | QIODevice::Text))
	{
		QMessageBox::about(nullptr, "error", "no file");
	}
	QTextStream in(&fileOpen); //创建输出流
	while (!in.atEnd())
	{
		QString oneLine = in.readLine();  //读取一行
		QString str_1st = oneLine.mid(1, 23);
		QString str_2nd = oneLine.mid(40, 1);
		QString str_3rd;
		if (str_2nd == "w")
		{
			str_2nd = oneLine.mid(40, 7);
			str_3rd = oneLine.mid(48);
		}
		else if (str_2nd == "i")
		{
			str_2nd = oneLine.mid(40, 4);
			str_3rd = oneLine.mid(45);
		}
		else if (str_2nd == "e")
		{
			str_2nd = oneLine.mid(40, 5);
			str_3rd = oneLine.mid(46);
		}
		else if (str_2nd == "c")
		{
			str_2nd = oneLine.mid(40, 8);
			str_3rd = oneLine.mid(49);
		}
		logAddLine(str_1st, str_2nd, str_3rd);

	}
}
//在类中重载eventFilter函数：
//bool DailyLog::eventFilter(QObject* watched, QEvent* event)
//{
//	if (watched == this)
//	{
//		//窗口停用，变为不活动的窗口  
//		if (event->type() == QEvent::WindowDeactivate)
//		{
//			this->setWindowOpacity(transLevel / 100.0);//透明度;
//			return true;
//		}
//		else if (event->type() == QEvent::WindowActivate)
//		{
//			if (ui.checkBox->isChecked())
//			{
//				this->setWindowOpacity(1);//本来要设置不同的透明度;
//			}
//			return false;
//		}
//	}
//	return false;
//}

void DailyLog::logAddLine(QString dateTime, QString type, QString content)
{

	ui.tableWidget->insertRow(logLineCount);//加一行

	QTableWidgetItem* myItem = new QTableWidgetItem(dateTime);
	if (type == "warning")
	{
		myItem->setBackgroundColor(QColor(220, 220, 0));
	}
	else if (type == "error" || type == "critical")
	{
		myItem->setBackgroundColor(QColor(220, 0, 0));
	}
	ui.tableWidget->setItem(logLineCount, 0, myItem);//第0列
	myItem = new QTableWidgetItem(type);
	if (type == "warning")
	{
		myItem->setBackgroundColor(QColor(220, 220, 0));
	}
	else if (type == "error" || type == "critical")
	{
		myItem->setBackgroundColor(QColor(220, 0, 0));
	}
	ui.tableWidget->setItem(logLineCount, 1, myItem);//第1列	
	myItem = new QTableWidgetItem(content);
	if (type == "warning")
	{
		myItem->setBackgroundColor(QColor(220, 220, 0));
	}
	else if (type == "error"|| type == "critical")
	{
		myItem->setBackgroundColor(QColor(220, 0, 0));
	}
	ui.tableWidget->setItem(logLineCount, 2, myItem);//第2列

	//if (ui.comboBox->currentText() != type &&ui.comboBox->currentText() != "all")//提示信息QString::fromLocal8Bit("一般警告")错误报警
	//{
	//	ui.tableWidget->setRowHidden(logLineCount, true);//显示所有行
	//}

	logLineCount++;

	if (logLineCount >= 43 && logLineCount < 100)
	{
		ui.tableWidget->setColumnWidth(2, 395);
	}
	else if (logLineCount >= 100 && logLineCount < 1000)
	{
		ui.tableWidget->setColumnWidth(2, 388);
	}
	else if (logLineCount >= 1000 && logLineCount < 10000)
	{
		ui.tableWidget->setColumnWidth(2, 381);
	}
	else if (logLineCount >= 10000 && logLineCount < 100000)
	{
		ui.tableWidget->setColumnWidth(2, 374);
	}
}

void DailyLog::initLogLine()
{
	QStringList title;
	title << QString::fromLocal8Bit("时间") << QString::fromLocal8Bit("类型") << QString::fromLocal8Bit("说明");
	ui.tableWidget->setColumnCount(3);//3列
	ui.tableWidget->setHorizontalHeaderLabels(title);//加表头
	//myLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//
	ui.tableWidget->setColumnWidth(0, 190);
	ui.tableWidget->setColumnWidth(1, 100);
	ui.tableWidget->setColumnWidth(2, 435);

	// 	myLog->verticalHeader()->setDefaultSectionSize(35);//默认行高20
	// 	myLog->verticalHeader()->setVisible(false);//不显示行头
}