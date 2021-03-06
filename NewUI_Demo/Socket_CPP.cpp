#include "Socket_CPP.h"
#include <QMessageBox>
#include <QFile>
#include "spdlog/spdlog.h"
static SOCKET s_client;
bool g_bStop = false;

extern QString AppPath;
namespace spd = spdlog;
//日志工具
extern std::shared_ptr<spd::logger> SYS_logger;//系统
extern std::shared_ptr<spd::logger> ALM_logger;//报警
extern std::shared_ptr<spd::logger> OPS_logger;//操作
MESSAGE_HANDLER m_message_handler;
void* m_pexe;
unsigned __stdcall  BeatSignal(LPVOID lpParam)
{
	DataToPC_typ *_ToPC = new DataToPC_typ();
	char *_ctoPC = new char[sizeof(DataToPC_typ)];
	memset(_ctoPC, 0, sizeof(DataToPC_typ));
	DataFromPC_typ	*_FromPC = new DataFromPC_typ();;
	char *_cfromPC = new char[sizeof(DataFromPC_typ)];
	memset(_cfromPC, 0, sizeof(DataFromPC_typ));
	while (!g_bStop)
	{
		memset(_cfromPC, 0, sizeof(DataFromPC_typ));
		memset(_FromPC, 0, sizeof(DataFromPC_typ));
		_FromPC->Telegram_typ = 0;////报文类型，0:心跳报文,1:命令报文,2:参数报文,3:结果报文,100:应答
		memcpy(_cfromPC, _FromPC, sizeof(DataFromPC_typ));
		if (send(s_client, _cfromPC, sizeof(DataFromPC_typ), 0))
		{
			while (sizeof(DataToPC_typ) == recv(s_client, _ctoPC, sizeof(DataToPC_typ), 0))
				memcpy(_ToPC, _ctoPC, sizeof(DataToPC_typ));
			
			if (nullptr != m_message_handler)
			{
				ALM_logger->error("{},{},{}",_ToPC->Telegram_typ, _ToPC->Status.HomeOK, _ToPC->Status.AlarmStatus);
				m_message_handler(m_pexe, *_ToPC);
			}
		}

		memset(_ToPC, 0, sizeof(DataToPC_typ));
		memset(_ctoPC, 0, sizeof(DataToPC_typ));
		Sleep(500);
	}
	return 0;
}

bool Socket_CPP::Communicate_PLC(DataFromPC_typ* m_Dmsg_FromPC, DataToPC_typ* m_Dmsg_ToPC)
{
	char* m_Cmsg_ToPC = new char[sizeof(DataToPC_typ)];//注意区分C和D，创建一新char
	memset(m_Cmsg_ToPC, 0, sizeof(DataToPC_typ));//将新char所指向的前size字节的内存单元用一个0替换，初始化内存。下同
	char* msg_cfromPC = new char[sizeof(DataFromPC_typ)];
	memset(msg_cfromPC, 0, sizeof(DataFromPC_typ));
	memcpy(msg_cfromPC, m_Dmsg_FromPC, sizeof(DataFromPC_typ));//将m_Dmsg_FromPC值赋值/覆盖给msg_cfromPC
	if (0 < send(s_client, msg_cfromPC, sizeof(DataFromPC_typ), 0))//发送msg_cfromPC，第4个参数一般置零
	{//如果send函数copy数据成功，就返回实际copy的字节数，如果send在copy数据时出现错误，那么send就返回SOCKET_ERROR （-1）
		while (0 > recv(s_client, m_Cmsg_ToPC, sizeof(DataToPC_typ), 0))//接收到m_Cmsg_ToPC
		{
		}
		if (m_Dmsg_ToPC != nullptr)
		{
			memcpy(m_Dmsg_ToPC, m_Cmsg_ToPC, sizeof(DataToPC_typ));//将接收到m_Cmsg_ToPC的值赋值/覆盖给m_Dmsg_ToPC
		}
		delete m_Cmsg_ToPC;
		delete msg_cfromPC;
		return true;
	}
	delete m_Cmsg_ToPC;
	delete msg_cfromPC;
	return false;
}

Socket_CPP::Socket_CPP()
{
	s_client = 0;
	m_Dmsg_FromPC = nullptr;
	m_Dmsg_ToPC = nullptr;
	m_message_handler = nullptr;
	m_pexe = nullptr;
	m_cip = nullptr;
	m_iport = 0;
}


Socket_CPP::~Socket_CPP()
{
	if (nullptr != m_Dmsg_FromPC)
	{
		delete m_Dmsg_FromPC;
		m_Dmsg_FromPC = nullptr;
	}
	if (nullptr != m_Dmsg_ToPC)
	{
		delete m_Dmsg_ToPC;
		m_Dmsg_ToPC = nullptr;
	}
	if (nullptr != m_cip)
	{
		delete m_cip;
		m_cip = nullptr;
	}
}

bool Socket_CPP::initialization()//连接初始化
{
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		return false;
	}
	else {
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		WSACleanup();
		return false;
	}
	else {
	}
	s_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Dmsg_FromPC == nullptr)
	{
		m_Dmsg_FromPC = new DataFromPC_typ;
	}
	if (m_Dmsg_ToPC == nullptr)
	{
		m_Dmsg_ToPC = new DataToPC_typ;
	}
	return true;
}
unsigned __stdcall  ConnectSever(LPVOID lpParam)
{
	SOCKADDR_IN server_addr; 
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(((Socket_CPP*)lpParam)->m_cip);
	server_addr.sin_port = htons(((Socket_CPP*)lpParam)->m_iport);
	int iTimeOut = 100;
	int ul = 1;
	int ret = ioctlsocket(s_client, FIONBIO, (unsigned long *)&ul);
	setsockopt(s_client, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
	setsockopt(s_client, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
	//int re = connect(s_client, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
	int reVal;			//返回值
	while (true)
	{
		//连接服务器
		reVal = connect(s_client, (struct sockaddr*)&server_addr, sizeof(server_addr));
		Sleep(10);
		//处理连接错误p
		if (SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();
			if (WSAEWOULDBLOCK == nErrCode || WSAEINVAL == nErrCode)    //连接还没有完成
			{
				continue;
			}
			else if (WSAEISCONN == nErrCode)//连接已经完成
			{
				DataToPC_typ *_ToPC = new DataToPC_typ();
				_ToPC->Telegram_typ = 102;
				HANDLE h_THeartBeat = (HANDLE)_beginthreadex(NULL, 0, BeatSignal, nullptr, 0, nullptr);
				ALM_logger->error("102nErrCode:{},{},{}", _ToPC->Telegram_typ, _ToPC->Status.HomeOK, _ToPC->Status.AlarmStatus);
				m_message_handler(m_pexe, *_ToPC);
				delete _ToPC;
				return true;
			}
			else//其它原因，连接失败
			{
				DataToPC_typ *_ToPC = new DataToPC_typ();
				_ToPC->Telegram_typ = 103;
				ALM_logger->error("103nErrCode:{},{},{}", _ToPC->Telegram_typ, _ToPC->Status.HomeOK, _ToPC->Status.AlarmStatus);
				m_message_handler(m_pexe, *_ToPC);
				delete _ToPC;
				return false;
			}
		}

		if (reVal == 0)//连接成功
			break;
	}





// 	if (re == SOCKET_ERROR)
// 	{
// 		DataToPC_typ *_ToPC = new DataToPC_typ();
// 		_ToPC->Telegram_typ = 103;
// 		m_message_handler(m_pexe, *_ToPC);
// 		delete _ToPC;
// 		return false;
// 	}
// 	else
// 	{
// 		DataToPC_typ *_ToPC = new DataToPC_typ();
// 		_ToPC->Telegram_typ = 102;
// 		HANDLE h_THeartBeat = (HANDLE)_beginthreadex(NULL, 0, BeatSignal, nullptr, 0, nullptr);
// 		m_message_handler(m_pexe, *_ToPC);
// 		delete _ToPC;
// 		return true;
// 	}
	return false;
	
}

bool Socket_CPP::connectServer(const char* ip, int port)
{
	if (m_cip == nullptr)
	{
		m_cip = new char[strlen(ip)+1];
	}
	strcpy(m_cip, ip);
	m_iport = port;
	h_THeartBeat = (HANDLE)_beginthreadex(NULL, 0, ConnectSever, this, 0, nullptr);
	return true;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);
	int iTimeOut = 100;
	int ul = 0;
	int ret = ioctlsocket(s_client, FIONBIO, (unsigned long *)&ul);
	setsockopt(s_client, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
	setsockopt(s_client, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
	int re = connect(s_client, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
	if (re == SOCKET_ERROR)
	{
		return false;
	}
	else
	{
		h_THeartBeat = (HANDLE)_beginthreadex(NULL, 0, BeatSignal, nullptr, 0, nullptr);
		return true;
	}
	return false;
}

bool Socket_CPP::disconnect()
{
	closesocket(s_client);
	WSACleanup();
	return false;
}

bool Socket_CPP::StartWork()
{
	memset(m_Dmsg_FromPC, 0, sizeof(DataFromPC_typ));
	memset(m_Dmsg_ToPC, 0, sizeof(DataToPC_typ));
	m_Dmsg_FromPC->Telegram_typ = 1;
	m_Dmsg_FromPC->Machine_Cmd.cmdStart = TRUE;
	if (Communicate_PLC(m_Dmsg_FromPC, m_Dmsg_ToPC))
	{
		return true;
	}

	return false;
}

bool Socket_CPP::StopWork()
{
	memset(m_Dmsg_FromPC, 0, sizeof(DataFromPC_typ));
	memset(m_Dmsg_ToPC, 0, sizeof(DataToPC_typ));
	m_Dmsg_FromPC->Telegram_typ = 1;
	m_Dmsg_FromPC->Machine_Cmd.cmdStop = TRUE;
	if (Communicate_PLC(m_Dmsg_FromPC, m_Dmsg_ToPC))
	{
		return true;
	}
	return false;
}


bool Socket_CPP::InitWork()
{
	memset(m_Dmsg_FromPC, 0, sizeof(DataFromPC_typ));
	memset(m_Dmsg_ToPC, 0, sizeof(DataToPC_typ));
	if (Communicate_PLC(m_Dmsg_FromPC, m_Dmsg_ToPC))
	{
	}
	if (m_Dmsg_ToPC->Status.HomeOK==1)//有可能开始就在初始位置，此时不用归原点
	{
		m_Dmsg_FromPC->Telegram_typ = 1;
		m_Dmsg_FromPC->Machine_Cmd.cmdHome = TRUE;
		if (Communicate_PLC(m_Dmsg_FromPC, m_Dmsg_ToPC))
		{
		}
	}


	DataFromPC_typ typ;

	QString plcParamPath = AppPath + "/plcParam/plc.txt";
	QFile f(plcParamPath);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return false;
	}
	char* ch = new char[sizeof(DataFromPC_typ) + 1];
	memset(ch, 0, sizeof(DataFromPC_typ) + 1);
	//txtInput.readAll();
	f.read(ch, sizeof(DataFromPC_typ));

	memcpy(&typ, ch, sizeof(DataFromPC_typ));//赋值回来

	f.close();

	typ.Telegram_typ = 2;
	Communicate_PLC(&typ, nullptr);//系统

	typ.Telegram_typ = 4;
	if(Communicate_PLC(&typ, nullptr))//运行
	{
		return true;
	}
	return false;
}

bool Socket_CPP::ResetError()
{
	memset(m_Dmsg_FromPC, 0, sizeof(DataFromPC_typ));
	memset(m_Dmsg_ToPC, 0, sizeof(DataToPC_typ));
	m_Dmsg_FromPC->Telegram_typ = 1;
	m_Dmsg_FromPC->Machine_Cmd.cmdErrorAck = TRUE;
	if (Communicate_PLC(m_Dmsg_FromPC, m_Dmsg_ToPC))
	{
		return true;
	}
	return false;
}


bool Socket_CPP::SetResult(int counter, unsigned int alarm[4])
{
	memset(m_Dmsg_FromPC, 0, sizeof(DataFromPC_typ));
	memset(m_Dmsg_ToPC, 0, sizeof(DataToPC_typ));
	m_Dmsg_FromPC->Telegram_typ = 3;
	m_Dmsg_FromPC->PhotoResult.Alarm[0] = alarm[0]; //0b00000101;
	m_Dmsg_FromPC->PhotoResult.Alarm[1] = alarm[1]; //0b00001001;
	m_Dmsg_FromPC->PhotoResult.Alarm[2] = alarm[2]; //0b00010001;
	m_Dmsg_FromPC->PhotoResult.Alarm[3] = alarm[3]; //0b00100001;
// 	for (int i=0;i<30;i++)
// 	{
// 		SET_BIT(msg_FromPC->PhotoResult.Alarm[i/8], i-i/8*8);
// 	}
	if (Communicate_PLC(m_Dmsg_FromPC, m_Dmsg_ToPC))
	{
		return true;
	}
	return false;
}


void Socket_CPP::set_message_handler(MESSAGE_HANDLER handler, void* pexe)
{
	m_message_handler = handler;
	m_pexe = pexe;
}