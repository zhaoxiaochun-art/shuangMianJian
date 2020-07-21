#pragma once

#include <QDialog>
#include "ui_WindowOut.h"

class WindowOut : public QDialog
{
	Q_OBJECT
public:
	WindowOut(QWidget *parent = Q_NULLPTR);
	~WindowOut();
	void getString(QString str,int time);

private:
	Ui::WindowOut ui;
};
