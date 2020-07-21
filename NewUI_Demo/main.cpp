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

	QSharedMemory mem("NewUI_Demo");//��ϵͳexe����Ϊ���������干���ڴ�mem

	WindowOut* levelOut = new WindowOut;
	windowCount = 0;

	if (!mem.create(1))//���������ڴ�mem������ù����ڴ��Ѵ��ڣ��򵯳���ʾ�Ի��򣬲��˳�
	{
		levelOut->getString(QString::fromLocal8Bit("ϵͳ������һʵ���������ظ�������"), 2000);
		levelOut->exec();
		return 0;
	}
	quint64 freeSpace = getDiskFreeSpace(QString("D:/"));
	if (freeSpace < 5)
	{
		levelOut->getString(QString::fromLocal8Bit("�޷�����ϵͳ��\n���̿ռ䲻�㣬������D����ʣ������5GB��"), 2000);
		levelOut->exec();
		return 0;
	}
	QApplication::setStyle(QStyleFactory::create("fusion"));

	NewUI_Demo w;
	w.show();
	return a.exec();
}
