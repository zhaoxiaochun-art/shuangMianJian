#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <Windows.h>
#pragma comment(lib, "user32.lib")//这两句和键盘有关
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
    int showMsgBox(QMessageBox::Icon icon, const char* titleStr, const char* contentStr, const char* button1Str, const char* button2Str);//全是中文
private:
    Ui::InputDialog *ui;

	WindowOut *levelOut;//show默认为非模态modal，如果是局部变量会闪现消失
protected:
    void closeEvent(QCloseEvent* event);
};

#endif // INPUTDIALOG_H
