#include "New_LineEdit.h"

New_LineEdit::New_LineEdit(QWidget *parent)
	: QLineEdit(parent)
{
}

New_LineEdit::~New_LineEdit()
{
}

void New_LineEdit::mousePressEvent(QMouseEvent *)
{
	this->selectAll();
}
