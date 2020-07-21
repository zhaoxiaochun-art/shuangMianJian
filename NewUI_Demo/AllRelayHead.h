#pragma once
//#define TEST

#define MODEL_BASLER		0
#define MODEL_DAHENG		1


//与PLC通信
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
//basler相机
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
	HANDLE h_ReadFirstImg;			//第一张图读取标志
	HANDLE hResultList;
	HANDLE hOutTrigger;
	HANDLE hResultTrigger;
	HANDLE hFinishGetImage;			//图像获取队列安全退出标志
	HANDLE hFinishDecodeLine;		//算法队列安全退出标志
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
	//相机显示位置，InitCheckClass添加，并加载入g_CheckClass。
	//未设置算法时，只标记在此，方便后续补充加载g_CheckClass；

	//RECT u_Rect;

	int TriggerBy;//4：TriggerBy

	HANDLE TriggerHandle;
	bool b_waitrespond;

	OUTMODEL i_OutModel; //3：OutModel
	//Basler无法读取IP，该参数为IP最后三位
	char c_CameraSign[10];		//7：CameraSign
	int i_RealLocatPos;
	char c_CameraOtherSign[30];		// 8：OtherCameraName
	int i_CheckPosNo;		//程序内部序号 3--4==ipos++
	char c_CameraName[20];	//相机名称 //1：序列号
							//标识符	
	int TypeOFCamera;		//序号见上 //10：TypeOFCamera
	char c_AlgName[30];                //2：AlgName
	int i_FrameWidth;		//图像高度
	int i_FrameHeight;		//图像宽度
	int i_FrameChannel;		//图像通道数
	int i_ExpouseTime;		//曝光时长 //5：ExpouseTime
	int i_LineRateHZ;		//连续采集帧率 //6：LineRateHZ
	int i_SaveOKorNG;	//存图标识符，-1，NG存图；0，全部保存；1，OK存图
	int i_SaveLoop;		//存图循环标志服，0，全部存图；>0，张数内循环

	int i_imgWidth;		//ROI 宽度
	int i_imgHeight;	//ROI 高度
	int i_OffsetX;		//ROI X 偏移量
	int i_OffsetY;		//ROI Y 偏移量



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
	int u_iTriggerSelect;	//0:外触发Line1,1：软件触发
	bool b_InvertX;				//9：InvertInt
#endif
#ifdef DAHENG
	CGXDevicePointer                m_objDevicePtr;               ///< 设备句柄
	CGXStreamPointer                m_objStreamPtr;               ///< 流对象
#endif


	CAMERASTRUCT()
	{
		b_waitrespond = false;//
		strcpy(c_CameraName, "");//1序列号
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
