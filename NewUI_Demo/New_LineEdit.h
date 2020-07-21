#pragma once

#include <QLineEdit>

class New_LineEdit : public QLineEdit
{
	Q_OBJECT

public:
	New_LineEdit(QWidget *parent);
	~New_LineEdit();
protected:
	void mousePressEvent(QMouseEvent *);
};
