#include "NewUI_Demo.h"
#include <QtWidgets/QApplication>
#include "WindowOut.h"
#include <QStyleFactory>
#include "AllRelayHead.h"
//#include "vld/vld.h"
int windowCount = 0;

quint64 getDiskFreeSpace(QString driver)
{
	LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();
	ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;
	if (!GetDiskFreeSpaceEx(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes))
	{
		return 0;
	}
	return (quint64)liTotalFreeBytes.QuadPart / 1024 / 1024 / 1024;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSharedMemory mem("NewUI_Demo");//以系统exe名称为参数，定义共享内存mem

	WindowOut* levelOut = new WindowOut;
	windowCount = 0;

	if (!mem.create(1))//创建共享内存mem，如果该共享内存已存在，则弹出提示对话框，并退出
	{
		levelOut->getString(QString::fromLocal8Bit("系统已运行一实例，请勿重复开启！"), 2000);
		levelOut->exec();
		return 0;
	}
	quint64 freeSpace = getDiskFreeSpace(QString("D:/"));
	if (freeSpace < 5)
	{
		levelOut->getString(QString::fromLocal8Bit("无法开启系统！\n磁盘空间不足，请清理D盘至剩余至少5GB！"), 2000);
		levelOut->exec();
		return 0;
	}
	QApplication::setStyle(QStyleFactory::create("fusion"));

	NewUI_Demo w;
	w.show();
	return a.exec();
}
