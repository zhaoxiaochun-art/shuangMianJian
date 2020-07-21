#include "UserMyLineEdit.h"

UserMyLineEdit::UserMyLineEdit(QWidget *parent)
	: QLineEdit(parent)
{
}

UserMyLineEdit::~UserMyLineEdit()
{
}

void UserMyLineEdit::mousePressEvent(QMouseEvent*)
{
	this->selectAll();
	emit POPUPKEYBOARD();
}
