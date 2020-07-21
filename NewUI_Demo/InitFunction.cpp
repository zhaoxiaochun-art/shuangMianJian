#include "InitFunction.h"
extern QString AppPath;
extern QVector<CAMERASTRUCT*> g_vectorCamera;
extern QString g_qModelName;//��ǰӦ��ģ��
InitFunction::InitFunction(QObject *parent)
	: QObject(parent)
{
	TypeOFCamera = 1;
	StartModel = false;
}

InitFunction::~InitFunction()
{
}

int InitFunction::ReadConfig()
{
	char * configpath = new char[MAX_PATH];
	strcpy(configpath, AppPath.toStdString().c_str());
	strcat(configpath, ("\\ModelFile\\" + g_qModelName + "\\CameraConfig.ini").toStdString().c_str());//c_str����char*
	QVector<std::string> v_line;//�洢������к�
	std::ifstream cin(configpath);//���ļ�·�������׼��
	delete configpath;
	std::string filename;
	std::string line;
	int r_n = 0, ipos = 0;
	if (cin) // �и��ļ�  
	{//ȷ��ini�ļ����ж�����������ļ�������IP����v_line
		while (getline(cin, line)) // line�в�����ÿ�еĻ��з�  
		{
			if (line.find("[") != std::string::npos&&line.find("[") != std::string::npos&&line.find("]") != std::string::npos)//std::string::nposָ������
			{
				line.erase(line.length() - 1, 1);
				line.erase(0, 1);
				if (line != "DefaultCameraType")
				{
					v_line.push_back(line);//��������
				}
			}
		}
	}
	r_n = v_line.size();//��ǰsize==2
	QSettings configIniRead(AppPath + "\\ModelFile\\" + g_qModelName + "\\CameraConfig.ini", QSettings::IniFormat);//���ɶ�ȡini��ʽ�ļ��Ķ���
	configIniRead.setIniCodec("UTF-8");
	for (int i = 0; i < r_n; i++)
	{

		QString s = v_line[i].c_str();
		int itpye = configIniRead.value(s + "/TypeOFCamera").toInt();//[]�Ǳ�Ҫ�ģ����кű�ͷ�µĲ���
		if (itpye == TypeOFCamera)
		{
			CAMERASTRUCT* tempcamera = new CAMERASTRUCT();
			strcpy_s(tempcamera->c_CameraName, v_line[i].c_str());
			strcpy_s(tempcamera->c_AlgName, configIniRead.value(s + "/AlgName").toString().toStdString().c_str());
			tempcamera->i_OutModel = (OUTMODEL)configIniRead.value(s + "/OutModel").toInt();
			tempcamera->i_CheckPosNo = ipos++;
			tempcamera->TriggerBy = configIniRead.value(s + "/TriggerBy").toInt();
			tempcamera->i_ExpouseTime = configIniRead.value(s + "/ExpouseTime").toInt();
			tempcamera->i_LineRateHZ = configIniRead.value(s + "/LineRateHZ").toInt();
			tempcamera->i_imgWidth = configIniRead.value(s + "/imgWidth").toInt();
			tempcamera->i_imgHeight = configIniRead.value(s + "/imgHeight").toInt();
			tempcamera->i_OffsetX = configIniRead.value(s + "/OffsetX").toInt();
			tempcamera->i_OffsetY = configIniRead.value(s + "/OffsetY").toInt();
			tempcamera->i_SaveOKorNG = configIniRead.value(s + "/SaveOKorNG").toInt();
			tempcamera->i_SaveLoop = configIniRead.value(s + "/SaveLoop").toInt();
			strcpy(tempcamera->c_CameraSign, configIniRead.value(s + "/CameraSign").toString().toStdString().c_str());
			tempcamera->i_RealLocatPos = QString(tempcamera->c_CameraSign).mid(3).toInt();
			strcpy(tempcamera->c_CameraOtherSign, configIniRead.value(s + "/OtherCameraName").toString().toLocal8Bit());
			tempcamera->TypeOFCamera = TypeOFCamera;
#ifdef BASLER
			tempcamera->b_InvertX = configIniRead.value(s + "/InvertInt", -1).toInt() == -1 ? false : true;
#endif
			g_vectorCamera.push_back(tempcamera);//��11�����ݣ�����10������ini�ļ�
		}
	}
	switch (TypeOFCamera)
	{
#ifdef BASLER
	case MODEL_BASLER:
	{
		//////////////////////////////////////////////////////////////////////////

		PylonInitialize();
		size_t i = 0;
		// Get the transport layer factory.
		CTlFactory& tlFactory = CTlFactory::GetInstance();

		// Get all attached devices and exit application if no device is found.
		DeviceInfoList_t devices;
		if (tlFactory.EnumerateDevices(devices) == 0)//δ�ҵ������豸
		{
			//throw RUNTIME_EXCEPTION( "No camera present.");
			levelOut = new WindowOut;
			levelOut->getString(QString::fromLocal8Bit("��ʾ��δ���������"), 2000);
			levelOut->show();

			QVector<CAMERASTRUCT*>::iterator it = g_vectorCamera.begin();
			for (; it != g_vectorCamera.end();)
			{
				delete *it;
				*it = nullptr;
				g_vectorCamera.erase(it);
			}
			return -1;
		}

		// Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
		int g_iCameraTotal = std::min(devices.size(), c_maxCamerasToUse);
		//int g_iCameraTotal = m_tempCamera.size();

		QVector<QString> Vec_ID;
		for (size_t i = 0; i < g_iCameraTotal; ++i)
		{
			CDeviceInfo dev = devices[i];
			try
			{
				CBaslerUniversalInstantCamera cb_Camera;
				QString cameraID;
				cb_Camera.Attach(tlFactory.CreateDevice(dev));
				cb_Camera.Open();
#ifdef BASLERUSB
				cameraID = cb_Camera.DeviceSerialNumber.GetValue();
				//cameraID = atoi(cb_Camera.DeviceSerialNumber.GetValue());
#endif
				cb_Camera.Close();
				cb_Camera.DestroyDevice();
				//Vec_ID.push_back(cameraID);


#ifdef BASLERUSB
				for (QVector<CAMERASTRUCT*>::iterator it = g_vectorCamera.begin(); it != g_vectorCamera.end(); it++)
				{
					//for (size_t i = 0; i < zzz; i++)
					{
						if (cameraID == (*it)->c_CameraName)
						{
							(*it)->cb_Camera.Attach(tlFactory.CreateDevice(dev));//BUG
							(*it)->b_Attach = true;
							break;
						}
					}
				}
#else
				(*(g_vectorCamera.begin()+i))->cb_Camera.Attach(tlFactory.CreateDevice(dev));//BUG
				(*(g_vectorCamera.begin()+i))->b_Attach = true;
				break;
#endif

			}
			catch (const GenericException &e)
			{
				e.what();
			}
		}
		//int zzz = Vec_ID.size();

		for (QVector<CAMERASTRUCT*>::iterator it = g_vectorCamera.begin(); it != g_vectorCamera.end(); )
		{
			if ((*it)->b_Attach == false)
			{
				g_vectorCamera.erase(it);
			}
			else
			{
				it++;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		r_n = g_vectorCamera.size();//ƥ�䵽�ļ���
		break;
	}
#endif
#ifdef DAHENG
	case MODEL_DAHENG:
	{
		//��ʼ���豸��
		IGXFactory::GetInstance().Init();

		//ö���豸
		GxIAPICPP::gxdeviceinfo_vector m_vectorDeviceInfo;
		IGXFactory::GetInstance().UpdateDeviceList(1000, m_vectorDeviceInfo);
		//���豸������ʾ���豸�б���

		int g_iCameraTotal = std::min(m_vectorDeviceInfo.size(), c_maxCamerasToUse);


		QVector<QString> Vec_ID;

		for (size_t i = 0; i < g_iCameraTotal; ++i)
		{
			try
			{
				Vec_ID.push_back(m_vectorDeviceInfo[i].GetSN().c_str());
			}
			catch (const CGalaxyException &e)
			{
			}
		}
		int zzz = Vec_ID.size();
		QVector<CAMERASTRUCT*>::iterator it = g_vectorCamera.begin();
		for (; it != g_vectorCamera.end();)
		{
			bool b_connectCamera = false;
			for (size_t i = 0; i < zzz; i++)
			{
				if (Vec_ID[i] == (*it)->c_CameraName)
				{
					//try
					//{

					//	(*it)->m_objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(m_vectorDeviceInfo[i].GetSN(), GX_ACCESS_EXCLUSIVE);;//BUG
					//}

					//catch (std::exception & e)
					//{
					//	QString str = e.what();
					//	return -1;
					//}
					try
					{
						(*it)->m_objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(m_vectorDeviceInfo[i].GetSN(), GX_ACCESS_EXCLUSIVE);//BUG 0��Ϊi

					}
					catch (CGalaxyException&e)
					{
						QMessageBox::critical(nullptr, QString::fromLocal8Bit("���������2.0�ӿ���"), QString((*it)->c_CameraName));
						break;
					}
					//��ȡ��ͨ������
					uint32_t nStreamCount = (*it)->m_objDevicePtr->GetStreamCount();

					//����
					if (nStreamCount > 0)
					{
						(*it)->m_objStreamPtr = (*it)->m_objDevicePtr->OpenStream(0);
					}
					b_connectCamera = true;
					break;
				}
			}
			if (!b_connectCamera)
			{
				g_vectorCamera.erase(it);//ɾ��δƥ�䵽��
			}
			else
				it++;
			//δ��ȡ��ini�ļ������½�
	}
		break;
	}
#endif
}
	v_line.clear();
	return r_n;
}

bool InitFunction::GetAllCamera()
{

#ifdef BASLER
	TypeOFCamera = MODEL_BASLER;
#endif
#ifdef DAHENG
	TypeOFCamera = MODEL_DAHENG;
#endif
	//��ȡ�������
	ReadConfig();// read444
	return TRUE;
}