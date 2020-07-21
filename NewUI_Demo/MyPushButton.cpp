#include "MyPushButton.h"
#include "AllRelayHead.h"
extern std::shared_ptr<spd::logger> OPS_logger;//²Ù×÷

MyPushButton::MyPushButton(QWidget *parent)
	: QPushButton(parent)
{
}

MyPushButton::~MyPushButton()
{
}

void MyPushButton::mousePressEvent(QMouseEvent*)
{
	OPS_logger->info("asdfad");
}
//
//void MyPushButton::focusInEvent(QFocusEvent *event)
//{
//	emit CHANGERUNSPEED();
//}