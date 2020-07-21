#pragma once

#include <QLineEdit>

class MyLineEdit : public QLineEdit
{
	Q_OBJECT
signals:
	void POPUPKEYBOARD(); 
	void CHANGERUNSPEED();
public:
	MyLineEdit(QWidget *parent);
	~MyLineEdit();
public slots:
	void mousePressEvent(QMouseEvent*);
	virtual void focusInEvent(QFocusEvent *event);

};
