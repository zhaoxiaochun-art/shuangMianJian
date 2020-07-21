#include "WindowOut.h"
#include <QPropertyAnimation>
#include "QDesktopWidget"//获取屏幕尺寸
#include <QPainter>
#include <QBitmap>
extern int windowCount;

WindowOut::WindowOut(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	QDesktopWidget* desktopWidget = QApplication::desktop();
	QRect deskRect = desktopWidget->availableGeometry();  //可用区域
	QRect screenRect = desktopWidget->screenGeometry();  //屏幕区域
	int w = screenRect.width();
	int h = screenRect.height();
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog |Qt::WindowStaysOnTopHint);
	this->setGeometry((w-this->width())/2, 20+(windowCount*180), this->width(), this->height());

	QBitmap bmp(this->size());
	bmp.fill();
	QPainter p(&bmp);
	p.setPen(Qt::NoPen);
	p.setBrush(Qt::black);
	p.drawRoundedRect(bmp.rect(), 5, 5);
	setMask(bmp);
	windowCount += 1;
}

WindowOut::~WindowOut()
{
	windowCount -= 1;
}

void WindowOut::getString(QString str,int time)
{
	ui.label->setText(str);

	QPropertyAnimation* pAnimation = new QPropertyAnimation(this, "windowOpacity");
	pAnimation->setDuration(time);
	pAnimation->setKeyValueAt(1, 0);
	connect(pAnimation, &QPropertyAnimation::valueChanged, [=](const QVariant& value) {
		if (value.toDouble() <= 0)
		{
			this->close();
			delete this;
		}
		});
	pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}




