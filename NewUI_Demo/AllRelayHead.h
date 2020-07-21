#pragma once
//#define TEST

#define MODEL_BASLER		0
#define MODEL_DAHENG		1


//��PLCͨ��
#define PLCCONNECT
	


#define BASLER
#define BASLERUSB
//#define DAHENG


//#define CARINOUTCHECK



#pragma region QT
#include <QSettings>
#include <QTcpSocket>
#include <QtNetwork>
#include <QThread>
#include <QMessageBox>
#include <QMessageBox>
#include <QDir>
#include <QLibrary>
#include <QVector>
#include <QGraphicsScene>
#include <QCloseEvent>
#include <QTimerEvent>
#include <QtMultimedia/QSound>
#include <QFileDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#pragma endregion

#ifdef PLCCONNECT
#include "comm.h"
#include "Socket_CPP.h"
#endif // PLCCONNECT

#include "RingBuffer.h"
#include "QT_FrontCheckClass.h"
#include "opencv.hpp"
#include "highgui.hpp"
#include <io.h>  
#include <direct.h>  
#include <vector>
#include <fstream>
#include <iostream> 
#include <math.h>
#pragma region log
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
namespace spd = spdlog;
#pragma endregion

static const size_t c_maxCamerasToUse = 32;
#pragma region Basler
//basler���
#ifdef BASLER
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>

using namespace Pylon;
using namespace Basler_UniversalCameraParams;
#pragma comment(lib,"GCBase_MD_VC141_v3_1_Basler_pylon.lib")
#endif
#pragma endregion


#pragma region Daheng
#ifdef DAHENG
#include "GalaxyIncludes.h"
#endif

enum OUTMODEL
{
	OUTMODEL_STRING = 0,
	OUTMODEL_INT,
	OUTMODEL_RISING,
	OUTMODEL_Falling,
};

struct CONTROLPARAM
{
	bool bShutDown;
	HANDLE hPausle;
	HANDLE h_ReadFirstImg;			//��һ��ͼ��ȡ��־
	HANDLE hResultList;
	HANDLE hOutTrigger;
	HANDLE hResultTrigger;
	HANDLE hFinishGetImage;			//ͼ���ȡ���а�ȫ�˳���־
	HANDLE hFinishDecodeLine;		//�㷨���а�ȫ�˳���־
	HANDLE hNewImageArrive;
	bool bLoadThread;
	CONTROLPARAM()
	{
		bShutDown = false;
		bLoadThread = false;
	}
};
struct CAMERASTRUCT
{
	//�����ʾλ�ã�InitCheckClass��ӣ���������g_CheckClass��
	//δ�����㷨ʱ��ֻ����ڴˣ���������������g_CheckClass��

	//RECT u_Rect;

	int TriggerBy;//4��TriggerBy

	HANDLE TriggerHandle;
	bool b_waitrespond;

	OUTMODEL i_OutModel; //3��OutModel
	//Basler�޷���ȡIP���ò���ΪIP�����λ
	char c_CameraSign[10];		//7��CameraSign
	int i_RealLocatPos;
	char c_CameraOtherSign[30];		// 8��OtherCameraName
	int i_CheckPosNo;		//�����ڲ���� 3--4==ipos++
	char c_CameraName[20];	//������� //1�����к�
							//��ʶ��	
	int TypeOFCamera;		//��ż��� //10��TypeOFCamera
	char c_AlgName[30];                //2��AlgName
	int i_FrameWidth;		//ͼ��߶�
	int i_FrameHeight;		//ͼ����
	int i_FrameChannel;		//ͼ��ͨ����
	int i_ExpouseTime;		//�ع�ʱ�� //5��ExpouseTime
	int i_LineRateHZ;		//�����ɼ�֡�� //6��LineRateHZ
	int i_SaveOKorNG;	//��ͼ��ʶ����-1��NG��ͼ��0��ȫ�����棻1��OK��ͼ
	int i_SaveLoop;		//��ͼѭ����־����0��ȫ����ͼ��>0��������ѭ��

	int i_imgWidth;		//ROI ���
	int i_imgHeight;	//ROI �߶�
	int i_OffsetX;		//ROI X ƫ����
	int i_OffsetY;		//ROI Y ƫ����



#ifdef LOCALPATH
	char* file_path;
#endif
#ifdef LOCALVIDEO
	char* video_path;
#endif
#ifdef HAIKANGMV
	void*	m_hDevHandle;
#endif
#ifdef BASLER
	CBaslerUniversalInstantCamera cb_Camera;
	bool b_Attach;
	int u_iTriggerSelect;	//0:�ⴥ��Line1,1���������
	bool b_InvertX;				//9��InvertInt
#endif
#ifdef DAHENG
	CGXDevicePointer                m_objDevicePtr;               ///< �豸���
	CGXStreamPointer                m_objStreamPtr;               ///< ������
#endif


	CAMERASTRUCT()
	{
		b_waitrespond = false;//
		strcpy(c_CameraName, "");//1���к�
		TriggerBy = -1;//
		i_CheckPosNo = -1;
		TypeOFCamera = -1;
		i_FrameChannel = 0;
		i_FrameHeight = 0;
		i_FrameWidth = 0;
		i_OutModel = OUTMODEL_STRING;
		i_ExpouseTime = 0;
		TriggerHandle = NULL;
		i_SaveOKorNG = 0;
		i_SaveLoop = 0;
#ifdef BASLER
		b_Attach = false;
#endif // BASLER

	}
	~CAMERASTRUCT()
	{
	}
};
