#pragma once
#include "AllRelayHead.h"
#include <QObject>
#include "WindowOut.h"
class InitFunction : public QObject
{
	Q_OBJECT

public:
	InitFunction(QObject *parent = nullptr);
	~InitFunction();

	int TypeOFCamera;
	bool StartModel;
private:
	//QVector<CAMERASTRUCT*> m_vectorCamera;
	WindowOut *levelOut;
public:
	bool GetAllCamera();
	int ReadConfig();
};
