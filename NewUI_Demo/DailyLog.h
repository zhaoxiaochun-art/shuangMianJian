#pragma once

#include <QDialog>
#include "ui_DailyLog.h"
#include "WindowOut.h"

class DailyLog : public QDialog
{
	Q_OBJECT

public:
	DailyLog(QWidget *parent = Q_NULLPTR);
	~DailyLog();
	void logAddLine(QString dateTime,QString type, QString content);
	void initLogLine();
	void myReadLine();
protected:
	//bool eventFilter(QObject* o, QEvent* e);//事件过滤器  
private:
	Ui::DailyLog ui;
	//int transLevel{ 50 };	
	int logLineCount;
	WindowOut *levelOut;//show默认为非模态modal，如果是局部变量会闪现消失
public slots:
	void on_pB_Refresh_clicked();
};
