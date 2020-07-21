#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <Windows.h>
#pragma comment(lib, "user32.lib")//������ͼ����й�
#include <QCloseEvent>
#include "WindowOut.h"
#include <QMessageBox>

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
    Q_OBJECT


signals:
     void toFather(QString);
public:
    explicit InputDialog(QWidget *parent = nullptr);
    ~InputDialog();
	void SWITCHOSK();
    void SHOWOSK();
    void HideOSK();
    void changingModel();
    int showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//ȫ������
private:
    Ui::InputDialog *ui;

	WindowOut *levelOut;//showĬ��Ϊ��ģ̬modal������Ǿֲ�������������ʧ
protected:
    void closeEvent(QCloseEvent* event);
};

#endif // INPUTDIALOG_H
