#pragma once
#include <Windows.h>
#include <process.h>
#include <iostream>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#include "comm.h"


typedef void (*MESSAGE_HANDLER) (void* context, DataToPC_typ);//声明MESSAGE_HANDLER头

class Socket_CPP
{
private:
public:
	Socket_CPP();
	~Socket_CPP();
	char* m_cip;//IP 10.86.50.210
	int m_iport;//port 5000
	SOCKADDR_IN server_addr; //服务器地址
	DataToPC_typ *m_Dmsg_ToPC; //PC接收
	DataFromPC_typ	*m_Dmsg_FromPC; //PC发送
	HANDLE h_THeartBeat; //心跳
	bool initialization(); //初始化
	bool connectServer(const char*, int); //连接服务器
	bool disconnect(); //断开
	bool StartWork(); //开始任务
	bool StopWork(); //停止
	bool InitWork(); //初始化任务
	bool ResetError(); //错误复位
	bool SetResult(int counter, unsigned int alarm[4]); //结果
	void set_message_handler(MESSAGE_HANDLER, void*); //消息头
	bool Communicate_PLC(DataFromPC_typ*, DataToPC_typ*);//PLC通信
};


