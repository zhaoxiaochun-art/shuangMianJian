#pragma once

#include "AllRelayHead.h"
#include <QPushButton>

class MyPushButton : public QPushButton
{
	Q_OBJECT
signals:

public:
	MyPushButton(QWidget *parent);
	~MyPushButton();

	void mousePressEvent(QMouseEvent*);
	//virtual void focusInEvent(QFocusEvent *event);

};
