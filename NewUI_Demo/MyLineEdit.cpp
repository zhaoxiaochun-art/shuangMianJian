#include "MyLineEdit.h"

MyLineEdit::MyLineEdit(QWidget *parent)
	: QLineEdit(parent)
{
}

MyLineEdit::~MyLineEdit()
{
}

void MyLineEdit::mousePressEvent(QMouseEvent*)
{
	this->selectAll();
	emit POPUPKEYBOARD();
}

void MyLineEdit::focusInEvent(QFocusEvent *event)
{
	emit CHANGERUNSPEED();
}