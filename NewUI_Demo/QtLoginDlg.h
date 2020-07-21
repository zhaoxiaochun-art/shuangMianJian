#pragma once

#include <QDialog>
#include "ui_QtLoginDlg.h"
#include <QMessageBox>
#include <QCloseEvent>
#include "WindowOut.h"

class QtLoginDlg : public QDialog
{
	Q_OBJECT

public:
	QtLoginDlg(QDialog *parent = Q_NULLPTR);
	~QtLoginDlg();
	int showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//全是中文
private:
	Ui::QtLoginDlg ui;

	WindowOut *levelOut;//show默认为非模态modal，如果是局部变量会闪现消失
	QString m_autoStartPath;
public:
	bool deleteDir(const QString& path);
	bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist);
public slots:
	void onEnsure();
	void onUserListChange(QString);
	void onLineeditChanged();
	void keyPressEvent(QKeyEvent*);
	bool eventFilter(QObject * obj, QEvent * event);
	void setAutoRun(bool isAutoRun);
	void setAutoClose(bool isAutoClose);

};
