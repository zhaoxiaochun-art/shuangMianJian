#pragma once

#include <QDialog>
#include "ui_ResultData.h"
#include <QMessageBox>
#include "WindowOut.h"

class ResultData : public QDialog
{
	Q_OBJECT
public slots:
	void on_lineEdit_textChanged(const QString& arg1);
	void on_pB_Choose_clicked();
	void on_pB_LeadOut_clicked();
	void slotDataChanged();
	void slotDataChanged_2();

	void deLoseFocus();//ʧȥȥ����Ŀ��Ϊ��editingfinished
	void SWITCHOSK();
public:
	ResultData(QWidget *parent = Q_NULLPTR);
	~ResultData();
	void initListWidget();
	void initTableWidget();
	int showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//ȫ������
	bool exportExecl(QTableWidget* tableWidget, QString dirName, QString fileName);
	void initDateEdit();
	bool copyDirectoryFiles_Log1(const QString& fromDir, const QString& toDir, bool coverFileIfExist);//ϵͳ��־
	bool copyDirectoryFiles_Log2(const QString& fromDir, const QString& toDir, bool coverFileIfExist);//������־
	bool copyDirectoryFiles_Log3(const QString& fromDir, const QString& toDir, bool coverFileIfExist);//������־
private:
	Ui::ResultData ui;
	WindowOut* levelOut;
	QStringList title;//ÿһ�е�ͷ
	//QStringList	titleEng{ "total" };//0��
	QStringList	titleEng;//0��
	int date_1;
	int date_2;

	QStringList m_ls_user;//���е��û�
};
