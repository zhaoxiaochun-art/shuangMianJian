#pragma once

#include <QLineEdit>

class UserMyLineEdit : public QLineEdit
{
	Q_OBJECT
signals:
	void POPUPKEYBOARD();
public:
	UserMyLineEdit(QWidget *parent);
	~UserMyLineEdit();
public slots:
	void mousePressEvent(QMouseEvent*);
};
