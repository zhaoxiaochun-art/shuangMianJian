// IoCard.cpp : Defines the entry point for the DLL application.
//

//#include "pch.h"
#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "IoCard.h"
#include "QString"
#include "Pci6356.h"
#pragma comment(lib,"Pci6356Lib.lib")

char	DLLversion[] = "[IoCard.DLL 2018-01-25,Ver2.1.4]";

typedef struct _OUTCTRL_STATUS {
//	int			in_Status;
	DWORD		out_Ctrl;
}OUTCTRL_STATUS,*POUTCTRL_STATUS;


typedef struct _LINE_CTRL_STATUS {
	int			mLine;
//	int			mCtrlType;//1 - 负控制逻辑，初态=自动与受控输出通道隐含全部需要输出；0 - 正常逻辑，初态=受控输出通道隐含不输出；
//	int			mIndepType;//1 - 多输出独立控制，0 - 多输出统一控制；
	OUTCTRL_STATUS	mOutCtrlStatus[266];
}LINE_CTRL_STATUS,*PLINE_CTRL_STATUS;

class	CPci6356_ioCtrl : public CPci6356_Card
{

public:
	CPci6356_ioCtrl();
	~CPci6356_ioCtrl();
	BOOL	mProcInputflag;
	HANDLE	m_InputEvent;
	BOOL	mProcC51flag;
	HANDLE	m_C51Event;
	BOOL	mProc500usflag;
	HANDLE	m_500usEvent;

//	int		m_BoardType;
	
//	PLINE_CTRL_STATUS	pLineCtrlStatus;
	LINE_CTRL_STATUS	pLineCtrlStatus[4];
	PC51MSG_FROMDRV	pMsgLine;
	int		MsgLineNum;
	int		m_500uscnt;
	
	BOOL	C51InitFin;
	DWORD	mEnInIntFlag;
	BOOL	initC51Flag;
	BOOL	OP_Start;
	DWORD	mInIntDelayms;

	UCHAR	OutPortCtrlStatus[24];
	UCHAR	InPortCtrlStatus[8];
//	PFPGA_INPUT_LINE	pLineParam;
//	FPGA_OUTPUT_PARAM	pOutPortParam;
	
	static UINT MonitInputThread(LPVOID pParam);
	static UINT MonitC51Thread(LPVOID pParam);
	static UINT Monit500usThread(LPVOID pParam);
	
	BOOL	Start();
	BOOL	InitLine();
	BOOL	InitC51Status();
//	BOOL	StopLineOut();
//	BOOL	StartLineOut();
	BOOL	LockComm();
	BOOL	UnlockComm();
	BOOL	IfLockComm();
	BOOL	m_LockComm;

	BOOL	EnAllInCntWork(BOOL enInCnt=TRUE);

	void	SetFPGA_C51Param_BYTE(int addr,BYTE cval);
	void	SetFPGA_C51Param_DWORD(int addr,DWORD cval);

	//回读输出控制参数，回读初值设定
	BOOL		ReadOutPpreCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam );
	//回读输出控制参数，回读在线控制输出,工作时采集输出端口控制参数输出值
	BOOL		ReadOutPinlCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam );

	//回读队列控制和索引参数，从FPGA读出队列控制参数和索引队列状态
	BOOL		ReadLineIndex(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam,DWORD  *sqIndexOpStatu,PUCHAR sqExKeep);
	//回读队列控制参数，从FPGA读出队列控制参数（不读索引）
	BOOL		ReadLineCtrl(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam );
	//回读历史消息队列
	BOOL		ReadMsgSQ(PUCHAR pmsg,int num);

};


//回读输出控制参数，回读初值设定
BOOL		CPci6356_ioCtrl::ReadOutPpreCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam )
{
	//
	int i;
	BYTE addr,data;
	addr = port * 8;
	for(i=0;i<7;i++) {
		Pci6356_IOWrite(0x80,addr+i);
		Pci6356_IORead(0x80,&data);
		OutCtrlParam.TMCtrl.TMreg[i] = data;
	}
	OutCtrlParam.TMCtrl.TMreg[7] = 0;
	Pci6356_IOWrite(0x84,(BYTE)(port));
	Pci6356_IORead(0x84,&data);
	OutCtrlParam.m_OutChnType = data;
	Pci6356_IORead(0x85,&data);
	OutCtrlParam.m_OutChnReLine = data;
	return TRUE; 
}

//回读输出控制参数，回读在线控制输出,工作时采集输出端口控制参数输出值
BOOL		CPci6356_ioCtrl::ReadOutPinlCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam )
{
		int i;
	BYTE addr,data;
	addr = port * 8;
	for(i=0;i<7;i++) {
		Pci6356_IOWrite(0x81,addr+i);
		Pci6356_IORead(0x81,&data);
		OutCtrlParam.TMCtrl.TMreg[i] = data;
	}
	OutCtrlParam.TMCtrl.TMreg[7] = 0;
	Pci6356_IOWrite(0x84,(BYTE)(port));
	Pci6356_IORead(0x84,&data);
	OutCtrlParam.m_OutChnType = data;
	Pci6356_IORead(0x85,&data);
	OutCtrlParam.m_OutChnReLine = data;
	return TRUE; 
}


//回读队列控制和索引参数，从FPGA读出队列控制参数和索引队列状态
BOOL		CPci6356_ioCtrl::ReadLineIndex(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam,DWORD  *sqIndexOpStatu,PUCHAR sqExKeep)
{
	int i;
	BYTE addr,data;
	inLineParam.mLine = nline & 0x03;
	inLineParam.mRelInp = m_LineCtrlParam[nline].m_InChn;
	Pci6356_IOWrite(0x4,(BYTE)(inLineParam.mRelInp));
	Pci6356_IORead(0x10,&data);
	inLineParam.mRelInpCountIndex = data;
	inLineParam.mRelOutNun = m_LineCtrlParam[nline].m_OutNum;
	for(i=0;i<inLineParam.mRelOutNun;i++) {
		Pci6356_IOWrite(0x84,(BYTE)(m_LineCtrlParam[nline].m_OutChn[i]));
		Pci6356_IORead(0x86,&data);
		inLineParam.mOutIndexStatu[i] = data;
	}
	PUCHAR ppp = (PUCHAR)sqIndexOpStatu;
	WORD waddr = nline * 1024;
	for(i=0;i<1024;i++) {
		addr = waddr & 0x0ff;
		Pci6356_IOWrite(0x82,addr);
		addr = (waddr>>8) & 0x0f;
		Pci6356_IOWrite(0x83,addr);
		Pci6356_IORead(0x82,ppp);
		waddr++;
		ppp++;
	}
	ppp = (PUCHAR)sqExKeep;
	waddr = nline * 1024;
	for(i=0;i<256;i++) {
		addr = waddr & 0x0ff;
		Pci6356_IOWrite(0x82,addr);
		addr = (waddr>>8) & 0x0f;
		Pci6356_IOWrite(0x83,addr);
		Pci6356_IORead(0x82,ppp);
		waddr+=4;
		ppp++;
	}
	return TRUE; 
}

//回读队列控制参数，从FPGA读出队列控制参数（不读索引）
BOOL		CPci6356_ioCtrl::ReadLineCtrl(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam )
{
	int i;
	BYTE addr,data;
	inLineParam.mLine = nline & 0x03;
	inLineParam.mRelInp = m_LineCtrlParam[nline].m_InChn;
	Pci6356_IOWrite(0x4,(BYTE)(inLineParam.mRelInp));
	Pci6356_IORead(0x10,&data);
	inLineParam.mRelInpCountIndex = data;
	inLineParam.mRelOutNun = m_LineCtrlParam[nline].m_OutNum;
	for(i=0;i<inLineParam.mRelOutNun;i++) {
		Pci6356_IOWrite(0x84,(BYTE)(m_LineCtrlParam[nline].m_OutChn[i]));
		Pci6356_IORead(0x86,&data);
		inLineParam.mOutIndexStatu[i] = data;
	}
	return TRUE; 
}

//回读历史消息队列
BOOL		CPci6356_ioCtrl::ReadMsgSQ(PUCHAR pmsg,int num)
{
	//必须把FPGA内部所有消息重走一遍，使用读使能，否则FPGA消息队列不对
	int i;
	PUCHAR ppp = pmsg;
	//移到需要输出头部
	for(i=0;i<(128-num);i++) {
		Pci6356_IOWrite(0x84,0x39);
	}
	//取出所要消息，按时间顺序
	for(i=0;i<num;i++) {
		Pci6356_IORead(0x30,ppp++);
		Pci6356_IORead(0x31,ppp++);
		Pci6356_IORead(0x32,ppp++);
		Pci6356_IORead(0x33,ppp++);
		Pci6356_IORead(0x34,ppp++);
		Pci6356_IORead(0x35,ppp++);
		Pci6356_IORead(0x36,ppp++);
		Pci6356_IORead(0x37,ppp++);
		Pci6356_IOWrite(0x84,0x39);
	}
	return TRUE; 
}



//启动所有输入工作
BOOL CPci6356_ioCtrl::EnAllInCntWork(BOOL enInCnt)
{
	int i;
	BYTE ddd;
	ddd = 0;
	if(enInCnt) ddd = 1;
	if(m_If32) {
		DWORD dval;
		dval = ddd;
		for(i=0;i<8;i++) {
			Pci6356_REGWrite32(0x460+i*4,dval);
		}
		return TRUE;
	}
	for(i=0;i<=7;i++) {
		Pci6356_IOWrite(0x1b,(BYTE)(i));
		Pci6356_IOWrite(0x1c,ddd);
	}
	return FALSE;
}

//使用C51控制所有过程，暂不实现 WORK >100
void CPci6356_ioCtrl::SetFPGA_C51Param_BYTE(int addr,BYTE cval)
{
	Pci6356_IOWrite(0x80,(BYTE)addr);
	Pci6356_IOWrite(0x81,(BYTE)(addr>>8)&0x3f);
	Pci6356_IOWrite(0x84,cval);
}
//使用C51控制所有过程，暂不实现 WORK >100
void CPci6356_ioCtrl::SetFPGA_C51Param_DWORD(int addr,DWORD cval)
{
	DWORD	ddd;
	ddd = (DWORD)addr;
	Pci6356_IOWrite(0x80,(BYTE)ddd);
	Pci6356_IOWrite(0x81,(BYTE)(ddd>>8)&0x3f);
	Pci6356_IOWrite(0x84,(BYTE)(cval & 0x0ff));
	ddd++;
	Pci6356_IOWrite(0x80,(BYTE)ddd);
	Pci6356_IOWrite(0x81,(BYTE)(ddd>>8)&0x3f);
	Pci6356_IOWrite(0x84,(BYTE)((cval>>8) & 0x0ff));
	ddd++;
	Pci6356_IOWrite(0x80,(BYTE)ddd);
	Pci6356_IOWrite(0x81,(BYTE)(ddd>>8)&0x3f);
	Pci6356_IOWrite(0x84,(BYTE)((cval>>16) & 0x0ff));
	ddd++;
	Pci6356_IOWrite(0x80,(BYTE)ddd);
	Pci6356_IOWrite(0x81,(BYTE)(ddd>>8)&0x3f);
	Pci6356_IOWrite(0x84,(BYTE)((cval>>24) & 0x0ff));
	ddd++;
}



BOOL CPci6356_ioCtrl::LockComm()
{
	if(m_LockComm) return FALSE;//如果已锁定，返回锁定错误
	m_LockComm = TRUE;
	return TRUE;
}

BOOL CPci6356_ioCtrl::UnlockComm()
{
	if(!m_LockComm) return FALSE;//如果未锁定，返回解锁错误
	m_LockComm = FALSE;
	return TRUE;
}

BOOL CPci6356_ioCtrl::IfLockComm()
{
	return m_LockComm;//返回锁定标志
}



CPci6356_ioCtrl::CPci6356_ioCtrl()
{
	m_LockComm = FALSE;
	mProc500usflag = FALSE;
	mProcC51flag = FALSE;
	mProcInputflag = FALSE;
	int i;
	for(i=0;i<24;i++) {
		OutPortCtrlStatus[i] = i;//所有输出状态为禁止，隐含为负逻辑，逻辑=0，输出为禁止导通
	}
	for(i=0;i<8;i++) {
		InPortCtrlStatus[i] = 0;//输入计数器为禁止工作
	}
	MsgLineNum = 0;
	pMsgLine = new C51MSG_FROMDRV[128];
	//	pLineCtrlStatus = new LINE_CTRL_STATUS[4];
}

CPci6356_ioCtrl::~CPci6356_ioCtrl() 
{
//	if(pLineCtrlStatus!=NULL) delete pLineCtrlStatus;
	if(pMsgLine!=NULL) delete[] pMsgLine;
}


/*
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
*/

// This is an example of an exported variable
IOCONTROL_API int nIoCard=0;

// This is an example of an exported function.
IOCONTROL_API int fnIoControl(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see IoCard.h for the class definition
CioControl::CioControl()
{
	m_pData = (void*)(new CPci6356_ioCtrl);
	
}

CioControl::~CioControl()
{
	if(m_pData!=NULL)	delete m_pData;
}

BOOL CioControl::PCI_Open(int mDev,void* hWnd)
{
	BOOL ret;
	ret = ((CPci6356_ioCtrl*)m_pData)->Pci6356_Open(mDev,nullptr);
	p_PciEMem =  ((CPci6356_ioCtrl*)m_pData)->p_PcieMem;
	m_IfPcie32 = ((CPci6356_ioCtrl*)m_pData)->m_If32;
	if(ret) {
		StopWork(TRUE);
		((CPci6356_ioCtrl*)m_pData)->Start();
		if(hWnd!=NULL)
			((CPci6356_ioCtrl*)m_pData)->m_hWnd = hWnd;
	}
//	DWORD ddd = p_PciEMem[0x40];
	return ret;
}

BOOL CioControl::SetMsgWnd(void* hWnd)
{
	((CPci6356_ioCtrl*)m_pData)->m_hWnd = hWnd;
	return TRUE;
}

BOOL	CioControl::GetC51Message(int msgNum,C51MSG_FROMDRV &mC51Msg)
{
	if( (msgNum>=0) && (msgNum<128) )
	{
		memcpy(&mC51Msg,&((CPci6356_ioCtrl*)m_pData)->pMsgLine[msgNum],sizeof(C51MSG_FROMDRV));
		return TRUE;
	}
	return FALSE;
}

BOOL CioControl::Close()
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_Close();
}

BOOL CioControl::PCI_IsOpen()
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_IsOpen();
	
}

BOOL CioControl::PCI_StartWork()
{
	return ContinueInCntWork(-1);
}

BOOL CioControl::PCI_StopWork()
{
	return PauseInCntWork(-1);
}

BOOL CioControl::PCI_SetOutStatus(int i, bool b)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SetOutIOStatus(i, b);
}

int CioControl::PCI_ReadInStatus(int pos)
{
	BYTE inPortReg = 0;
	int re = -1;
	if (((CPci6356_ioCtrl*)m_pData)->Pci6356_IsOpen())
	{
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadInIOStatus(inPortReg);
	}
		re = inPortReg >> pos;
	return re;
}

BOOL CioControl::ReadInPortStatus(BYTE &inPortReg)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadInIOStatus(inPortReg);
}

BOOL CioControl::GetAllTmParam(PCTRL_TIMEREG nowTmParam)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadTimeReg((PPCI6356_TIMEREG)nowTmParam);
}

BOOL CioControl::GetAllTmParam(PCTRL_TIMEREG_LX45 nowTmParam)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadTimeReg_LX45((PPCI6356_TIMEREG_LX45)nowTmParam);
}

BOOL CioControl::PCI_GetTm2us(DWORD &nowTm2us)
{
	DWORD	ret;
	if (!((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadTmParam_LX45(0,&ret)) return FALSE;
	nowTm2us = ret;
	return TRUE;
}

BOOL CioControl::PCI_GetTmCoder(int coder_n,DWORD &nowhAll,DWORD &nowf,DWORD &nowh)
{
	DWORD	ret[2];
	if ((coder_n<0)||(coder_n>3)) return FALSE;
	if (!((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadTmParam_LX45(coder_n+1,ret)) return FALSE;
// 	nowhAll = ret[0];
// 	nowh = ret[1]&0x0ffff;
// 	nowf = (ret[1]>>16)&0x0ffff;
 	nowhAll = ret[1];
 	nowh = ret[0]&0x0ffff;
 	nowf = (ret[0]>>16)&0x0ffff;
	return TRUE;
	
}

BOOL CioControl::PCI_GetTmCoder(int coder_n,DWORD &nowhAll)
{
	DWORD	ret[2];
	if ((coder_n<0)||(coder_n>3)) return FALSE;
	if (!((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadTmParam_LX45(coder_n+1,ret)) return FALSE;
	nowhAll = ret[1];
	return TRUE;
}

BOOL CioControl::PCI_GetTmCoder(int coder_n,DWORD &nowf,DWORD &nowh)
{
	DWORD	ret[2];
	if ((coder_n<0)||(coder_n>3)) return FALSE;
	if (!((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadTmParam_LX45(coder_n+1,ret)) return FALSE;
	nowh = ret[0]&0x0ffff;
	nowf = (ret[0]>>16)&0x0ffff;
	return TRUE;
}

//在DLL中维护各队列的256个索引的输出端控制状态
//用于多受控输出通道独立输出控制
//每次控制一个输出
BOOL CioControl::SetOutCtrlByResult(int nChannel,int nIndex,int lineoutpout,int nResult)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	DWORD ddd;
	ddd = 1;
	int i;
	i = pIO->m_LineCtrlParam[nChannel].m_OutChn[lineoutpout];
	if(i>=pIO->m_LineCtrlParam[nChannel].m_OutNum) return FALSE;
	if(nResult==1) {
		ddd = ddd << i;
		ddd |= 0x03000000;//控制执行
		pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= ddd;//控制输出允许执行，并设置控制端口状态为有效
	}
	else {
		ddd = ddd << i;
		ddd = ~ddd;
		ddd = (ddd & 0x00ffffff );
		pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl &= ddd;
		pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x01000000;//非控制自动执行允许
	}
	SENDTOC51CMD paramCmd;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x01;//踢废命令
	paramCmd.Command[1] = nChannel;//队列号
	paramCmd.Command[2] = nIndex;//索引值
	paramCmd.Command[3] = (BYTE)ddd;//
	paramCmd.Command[4] = (BYTE)(ddd>>8);//
	paramCmd.Command[5] = (BYTE)(ddd>>16);//
	paramCmd.Command[6] = (BYTE)(ddd>>24);//
	paramCmd.Command[7] = 0;
	paramCmd.lg = 8;
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendCmdToC51(&paramCmd);
}

//每次控制多个输出
BOOL CioControl::SetOutCtrlByResultEx(int nChannel,int nIndex,int nResult)//每次控制一个输入产品索引的M个输出
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	DWORD ddd,dd1,dd2;
	ddd = 1;
	int i,j,nn;
	nn = pIO->m_LineCtrlParam[nChannel].m_OutNum;
	
	//收到控制命令
	//可重复发命令
	//pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x03000000;
	
	//20171026修改后，不能重复发命令，按最后一次参数输出
	pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl = 0x03000000;

	for(j=0;j<nn;j++) {
		dd1 = 1 << j;
		dd2 = nResult & dd1;
		i = pIO->m_LineCtrlParam[nChannel].m_OutChn[j];
		if(i>=24) return FALSE;
		if(dd2!=0) {
			ddd = 1 << i;
//			ddd |= 0x03000000;//控制执行
			pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= ddd;//控制输出允许执行，并设置控制端口状态为有效
		}
/*		else {
			ddd = 1 << i;
			ddd = ~ddd;
			ddd = (ddd & 0x00ffffff );
			pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl &= ddd;
			pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x01000000;//非控制自动执行允许
		}*/
	}

	ddd = pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl;
	int nextI;
	nextI  = nIndex + 10;
//	nIndex++;
	//if(nextI>255) nextI = 0;//当不是使用步进1时,NEXT复位不正确
	if(nextI>=256) nextI = nextI - 256;//当不是使用步进1时,NEXT复位不正确
	//将下一次的索引复位,保证处理控制参数复位,循环调用时不会出错,
	//可以通过nextI  = nIndex + nn;调节nn来解决顺序调用的抖动问题
	//因此必须顺序处理索引号,在多线程时同步不好,如果出现后面的索引倒置,可能会出现错误
	pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nextI].out_Ctrl = 0;

	SENDTOC51CMD paramCmd;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x01;//踢废命令
	paramCmd.Command[1] = nChannel;//队列号
	paramCmd.Command[2] = nIndex;//索引值
	paramCmd.Command[3] = (BYTE)ddd;//
	paramCmd.Command[4] = (BYTE)(ddd>>8);//
	paramCmd.Command[5] = (BYTE)(ddd>>16);//
	paramCmd.Command[6] = (BYTE)(ddd>>24);//
	paramCmd.Command[7] = 0;
	paramCmd.lg = 8;
	return pIO->Pci6356_SendCmdToC51(&paramCmd);
}

//每次控制多个队列多个索引的多个输出
BOOL CioControl::SetOutCtrlByResultEx2(int num,PCTRLOUT_RESULT_INDEP pCtrl)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//参数初始化数据
	paramData.CLength = 1 + num * 4;//参数长度16字节
	paramData.CParam = 0xf1f1f1f1;//数据类型队列初始化
	paramData.DataBuff[0] = (BYTE)num;

	DWORD ddd,dd1,dd2;
	ddd = 1;
	int mm,i,j,nn,nChannel,nIndex;
	for(mm=0;mm<num;mm++) {
		nChannel = pCtrl[mm].nline;
		nIndex = pCtrl[mm].nIndex;
		nn = pIO->m_LineCtrlParam[nChannel].m_OutNum;
		for(j=0;j<nn;j++) {
			dd1 = 1 << j;
			dd2 = pCtrl[mm].nResult & dd1;
			i = pIO->m_LineCtrlParam[nChannel].m_OutChn[j];
			if(i>=nn) return FALSE;
			if(dd2!=0) {
				ddd = 1 << i;
				ddd |= 0x03000000;//控制执行
				pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= ddd;//控制输出允许执行，并设置控制端口状态为有效
			}
			else {
				ddd = 1 << i;
				ddd = ~ddd;
				ddd = (ddd & 0x00ffffff );
				pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl &= ddd;
				pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x01000000;//非控制自动执行允许
			}
		}
	}

	
	for(i=0;i<num;i++) {
		ddd = pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl;
		paramData.DataBuff[i*6+1] = (BYTE)pCtrl->nline;
		paramData.DataBuff[i*6+2] = (BYTE)pCtrl->nIndex;
		paramData.DataBuff[i*6+3] = (BYTE)(ddd&0x0ff);
		paramData.DataBuff[i*6+4] = (BYTE)((ddd>>8)&0x0ff);
		paramData.DataBuff[i*6+5] = (BYTE)((ddd>>16)&0x0ff);
		paramData.DataBuff[i*6+6] = (BYTE)((ddd>>24)&0x0ff);
	}
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendDataToC51(&paramData);

}

BOOL CioControl::SetResult(int nChannel,int nIndex,int nResult,int delay)
{
//	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SetResult(nChannel,nIndex,nResult);
	SENDTOC51CMD paramCmd;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x01;//踢废命令
	paramCmd.Command[1] = nChannel;//队列号
	paramCmd.Command[2] = nIndex;//索引值
	paramCmd.Command[3] = nResult;//踢废控制参数
	if(delay<=0)
		paramCmd.Command[4] = 0;
	else {
		if(delay>=255)
			paramCmd.Command[4] = (BYTE)0xff;
		else
			paramCmd.Command[4] = (BYTE)delay;
	}
	paramCmd.Command[5] = 0;
	paramCmd.lg = 6;
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendCmdToC51(&paramCmd);
}

BOOL CioControl::SetResultEx(int num,PCTRLOUT_RESULT pRes)
{
	//	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SetResult(nChannel,nIndex,nResult);
	int i;
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//参数初始化数据
	paramData.CLength = 1 + num * 4;//参数长度16字节
	paramData.CParam = 0xf1f1f1f1;//数据类型队列初始化
	paramData.DataBuff[0] = (BYTE)num;
	for(i=0;i<num;i++) {
		paramData.DataBuff[i*4+1] = (BYTE)pRes[i].nline;
		paramData.DataBuff[i*4+2] = (BYTE)pRes[i].nIndex;
		paramData.DataBuff[i*4+3] = (BYTE)pRes[i].mOP;
		if(pRes->mNeedDelay) {
			paramData.DataBuff[i*4+4] = 0;
		}
		else {
			if(pRes->mDelay>=255)
				paramData.DataBuff[i*4+4] = (BYTE)0xff;
			else
				paramData.DataBuff[i*4+4] = (BYTE)pRes[i].mDelay;
		}
	}
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendDataToC51(&paramData);
}


int CioControl::ReadInputdelayIndex(int nChannel)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	if((nChannel<0)&&(nChannel>3)) return -1;
	int inputn;
	inputn = pIO->m_LineCtrlParam[nChannel].m_InChn;
	if( (inputn>=0)&&(inputn<=7) )
		return ReadInPortdelayCount(inputn,pIO->m_LineCtrlParam[nChannel].m_LineCntDelay);
	return -1;
}

int CioControl::ReadOutputdelayIndex(int nChannel,int nPos)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	if((nChannel<0)&&(nChannel>3)) return -1;
	int outputn;
	int outputnum;
	//nChannel 对应索引，从索引中取得输入端口号
	outputnum = pIO->m_LineCtrlParam[nChannel].m_OutNum;
	if( (nPos<0)&&(nPos>=outputnum) ) return -1;
	outputn = pIO->m_LineCtrlParam[nChannel].m_OutChn[nPos];
	if( (outputn>=0)&&(outputn<=23) ) 
		return ReadOutputdelayCount(outputn,pIO->m_LineCtrlParam[nChannel].m_OutChnCntDelay[nPos]);
	return -1;
}

int CioControl::ReadInPortdelayCount(int ninPort,int nDelay)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	if((ninPort<0)&&(ninPort>7)) return -1;
	//BYTE ddd = pIO->Pci6356_ReadInPDelay(ninPort,pIO->m_LineCtrlParam[nChannel].m_LineCntDelay);
	BYTE ddd = pIO->Pci6356_ReadInPDelay(ninPort,nDelay);
	int ret = ddd;
	return ddd;
}

int CioControl::ReadOutputdelayCount(int noutPort,int nDelay)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	if((noutPort<0)&&(noutPort>23)) return -1;
	BYTE ddd = pIO->Pci6356_ReadOutPDelay(noutPort,nDelay);
	int ret = ddd;
	return ddd;
}

int CioControl::ReadInputCount(int ninPort)
{
	ULONG	inCount;
	if((ninPort<0)&&(ninPort>7)) return -1;
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadInPCnt32_t(ninPort,inCount);
	return (int)inCount;
}


int CioControl::ReadInputIndex(int nChannel)
{
	int	lineCount;
	UCHAR ddd[8];
	if((nChannel<0)&&(nChannel>3)) return -1;
	int inputn;
	//得到队列对应的输入端口号
	inputn = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_InChn;
	//nChannel 对应索引，从索引中取得输入端口号
	if( (inputn>=0) && (inputn<=7) ) {
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadInputIOParam(inputn,ddd);
		lineCount = ddd[0];
	}
	else {
		lineCount = -1;//ddd[nChannel];
	}

	return lineCount;
}

int CioControl::ReadOutputCount(int nChannel,int nPos)
{
	DWORD	outCount;
	if((nChannel<0)&&(nChannel>3)) return -1;
	int outputn;
	int outputnum;
	//nChannel 对应索引，从索引中取得输入端口号
	outputnum = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutNum;
	outputn = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutChn[nPos];
	if( (outputn<0)&&(outputn>=outputnum) ) return -1;
	//
	if( (outputn>=0) && (outputn<=23) ) {
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutPCnt32_o(outputn,outCount);
	}
	else {
		outCount = -1;//ddd[nChannel];
	}

	return outCount;
}

int CioControl::ReadOutputCtrlCnt(int nChannel,int nPos)
{
	DWORD	outCount;
	if((nChannel<0)&&(nChannel>3)) return -1;
	int outputn;
	int outputnum;
	//nChannel 对应索引，从索引中取得输入端口号
	outputnum = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutNum;
	if( (nPos<0)&&(nPos>=outputnum) ) return -1;
	outputn = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutChn[nPos];
	//
	if( (outputn>=0) && (outputn<=23) ) {
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutPCnt32_t(outputn,outCount);
	}
	else {
		outCount = -1;//ddd[nChannel];
	}
	return outCount;
}

int CioControl::ReadOutputBTCnt(int nChannel,int nPos)
{
	DWORD	outCount;
	if((nChannel<0)&&(nChannel>3)) return -1;
	int outputn;
	int outputnum;
	//nChannel 对应索引，从索引中取得输入端口号
	outputnum = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutNum;
	if( (nPos<0)&&(nPos>=outputnum) ) return -1;
	outputn = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutChn[nPos];
	//
	if( (outputn>=0) && (outputn<=23) ) {
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutPCnt32_a(outputn,outCount);
	}
	else {
		outCount = -1;//ddd[nChannel];
	}
	return outCount;
}



int CioControl::ReadOutputCount(int outputn)
{
	DWORD	outCount;
	if((outputn<0)&&(outputn>23)) return -1;
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutPCnt32_o(outputn,outCount);
	return outCount;
}

BOOL CioControl::GetRate(int port,float &rate)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_GetRateVal(port,rate);
}

int CioControl::ReadOutputCtrlCnt(int outputn)
{
	DWORD	outCount;
	if((outputn<0)&&(outputn>23)) return -1;
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutPCnt32_t(outputn,outCount);
	return outCount;
}

int CioControl::ReadOutputBTCnt(int outputn)
{
	DWORD	outCount;
	if((outputn<0)&&(outputn>23)) return -1;
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutPCnt32_a(outputn,outCount);
	return outCount;
}



int CioControl::ReadOutputIndex(int nChannel,int nPos)
{
	int	outIndex;
	UCHAR ddd[24];
	if((nChannel<0)&&(nChannel>3)) return -1;
	int outputn;
	int outputnum;
	//nChannel 对应索引，从索引中取得输入端口号
	outputnum = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutNum;
	if( (nPos<0)&&(nPos>=outputnum) ) return -1;
	outputn = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_OutChn[nPos];
	if( (outputn>=0) && (outputn<=23) ) {
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadOutputIOParam(outputn,ddd);
		outIndex = ddd[0];
	}
	else {
		outIndex = -1;//ddd[nChannel];
	}

	return outIndex;
}

//
void CioControl::SetCodeFramelg_LX45()
{
	DWORD ddd;
	if(m_IfPcie32){//暂不实现
		return;
	}
	ddd = mCodehNum[0];
	IOWrite(0x25,0);
	IOWrite(0x23,ddd&0x0ff);
	IOWrite(0x24,(ddd>>8)&0x0f);
	ddd = mCodehNum[1];
	IOWrite(0x25,1);
	IOWrite(0x23,ddd&0x0ff);
	IOWrite(0x24,(ddd>>8)&0x0f);
	ddd = mCodehNum[2];
	IOWrite(0x25,2);
	IOWrite(0x23,ddd&0x0ff);
	IOWrite(0x24,(ddd>>8)&0x0f);
	ddd = mCodehNum[3];
	IOWrite(0x25,3);
	IOWrite(0x23,ddd&0x0ff);
	IOWrite(0x24,(ddd>>8)&0x0f);
}

//输入参数，并初始化
BOOL CioControl::ReadParameterFile_NewLX45(const char* strPath)
{
	char str[16],str1[32],strbd[32];
	int i,j;
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	pCard->m_BoardType = 1;//设置板类型 LX45
	//配置基础参数
	sprintf_s(str,"基础参数");
	//计数器延迟参数,计数值延迟输出时间
	mCntVOutDelay = GetPrivateProfileIntA(str,"CntVOutDelay",20,strPath);
	((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(mCntVOutDelay);
	//输出扩展配置参数,受控操作时控制增加输出高电平时间,_t为时间计数,_h为行计数
	mKeepExBase_t = GetPrivateProfileIntA(str,"KeepTmExUnit",5,strPath);
	mKeepExBase_h = GetPrivateProfileIntA(str,"KeepHcExUnit",5,strPath);
	((CPci6356_ioCtrl*)m_pData)->Pci6356_SetKeepExUnit(mKeepExBase_t,mKeepExBase_h);
	//与S500不同，各队列独自选择定时器与编码器选择方式，

	//设置编码器参数
	mCodehNum[0] = GetPrivateProfileIntA(str,"InCodeh0",0,strPath);
	mCodehNum[1] = GetPrivateProfileIntA(str,"InCodeh1",0,strPath);
	mCodehNum[2] = GetPrivateProfileIntA(str,"InCodeh2",0,strPath);
	mCodehNum[3] = GetPrivateProfileIntA(str,"InCodeh3",0,strPath);
	SetCodeFramelg_LX45();
	//设置编码器滤波系数
	int CodeFilter = GetPrivateProfileIntA(str,"CodeFilter",85,strPath);//'b01010101=0x55=85,隐含滤波
	IOWrite(0x68,(BYTE)CodeFilter);	//设置级联输出端口
	mInCoeOut = GetPrivateProfileIntA(str,"InCoeOut",0,strPath);
	if(!m_IfPcie32)	IOWrite(0x10,BYTE(mInCoeOut&0x0ff));
	mEnInIntFlag = GetPrivateProfileIntA(str,"InEnINT",0,strPath);
	mInIntDelayms = GetPrivateProfileIntA(str,"InDelayINTms",10,strPath);
	return TRUE;	
}

BOOL CioControl::ReadParameterFile_OLDS500(const char* strPath)
{
	char str[16],str1[32],strbd[32];
	int i,j;
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	pCard->m_BoardType = 0;//设置板类型 S500
	//配置基础参数
	sprintf_s(str,"基础参数");
	//计数器延迟参数,计数值延迟输出时间
		//计数器延迟参数
		mCntVOutDelay = GetPrivateProfileIntA(str,"CntVOutDelay",20,strPath);
		((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(mCntVOutDelay);
		mKeepExBase_t = GetPrivateProfileIntA(str,"KeepTmExUnit",5,strPath);
		mKeepExBase_h = GetPrivateProfileIntA(str,"KeepHcExUnit",5,strPath);
		((CPci6356_ioCtrl*)m_pData)->Pci6356_SetKeepExUnit(mKeepExBase_t,mKeepExBase_h);

		mRunType1 = GetPrivateProfileIntA(str,"RunType1",0,strPath);
		mRunType2 = GetPrivateProfileIntA(str,"RunType2",0,strPath);
		((CPci6356_ioCtrl*)m_pData)->Pci6356_SetBaseType(mRunType1,mRunType2);
		mCode1dsType = GetPrivateProfileIntA(str,"Code1dsType",0,strPath);
		mCode2dsType = GetPrivateProfileIntA(str,"Code2dsType",0,strPath);
		mCode1hMax = GetPrivateProfileIntA(str,"Code1hMax",1000,strPath);
		mCode2hMax = GetPrivateProfileIntA(str,"Code2hMax",1000,strPath);
		if(pCard->m_BoardType != 1) {//????有问题?20190425
			((CPci6356_ioCtrl*)m_pData)->Pci6356_SetCodeParam(mCode1dsType,mCode1hMax,mCode2dsType,mCode2hMax);
		}


	mEnInIntFlag = GetPrivateProfileIntA(str,"InEnINT",0,strPath);
	mInIntDelayms = GetPrivateProfileIntA(str,"InDelayINTms",10,strPath);
	return TRUE;	
}


BOOL CioControl::PCI_Init(const char* strPath,bool b_Pause)
{
	char str[16],str1[32],strbd[32];
	int i,j;
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	StopWork(TRUE);
	sprintf_s(str,"工作模型");
	//GetPrivateProfileIntA(str,"BoardType",0,strPath);
	mWork_Mode = GetPrivateProfileIntA(str,"WorkMode",0,strPath);
	((CPci6356_ioCtrl*)m_pData)->WorkMode = mWork_Mode;

	memset(strbd,0,32);
	GetPrivateProfileStringA(str,"BoardType","RunType1",strbd,30,strPath);
	QString sss;
	sss = strbd;
	if(sss=="PCI6356_LX45")	{//LX45增加的参数
		ReadParameterFile_NewLX45(strPath);
	}
	else if(sss!="PCI6356_S500")	{
		return FALSE;
	}
	else {
		ReadParameterFile_OLDS500(strPath);//同时设置了队列的控制输出方式
	}
	pCard->mEnInIntFlag = mEnInIntFlag;
	pCard->mInIntDelayms = mInIntDelayms;
	if(m_IfPcie32) RegWrite32(0x210,mInIntDelayms);
	else IOWrite(0x67,(UCHAR)(mInIntDelayms));
	if((mWork_Mode==0)||(mWork_Mode==101)) {//基于2us时间计数器和编码器计数
		for (i=0;i<4;i++)
		{
			sprintf_s(str,"队列%d",i);
			pCard->m_LineCtrlParam[i].m_LineNo = i;
			pCard->m_LineCtrlParam[i].m_Used = GetPrivateProfileIntA(str,"nUsed",0,strPath);//启用开关
			pCard->m_LineCtrlParam[i].m_InChn = GetPrivateProfileIntA(str,"nInPort",0,strPath);//对应输入端口
			pCard->m_LineCtrlParam[i].m_InFilterType = GetPrivateProfileIntA(str,"nInFilter",0,strPath);//对应输入端口
			pCard->m_LineCtrlParam[i].m_InMaxRate = GetPrivateProfileIntA(str,"nMaxRate",10,strPath);//对应输入端口
//			pCard->m_LineCtrlParam[i].m_OutCtrlType = GetPrivateProfileIntA(str,"nOutCtrlType",0,strPath);//输出隐含控制输出方式：独立/统一，已全部独立
			pCard->m_LineCtrlParam[i].m_OutNum = GetPrivateProfileIntA(str,"nOutPortNum",1,strPath);//输出控制端口个数
			pCard->m_LineCtrlParam[i].m_TmType = GetPrivateProfileIntA(str,"nRunType",0,strPath);//队列推进方式，LX45-0x18执行，不影响S500
			pCard->m_LineCtrlParam[i].m_LastOutPort = GetPrivateProfileIntA(str,"nLastOutPort",-1,strPath);//最后一个控制端口
			pCard->m_LineCtrlParam[i].m_LineCntDelay = GetPrivateProfileIntA(str,"nLineCntDelay",20,strPath);//计数延迟
			int cchn;

			for (j=0;j<pCard->m_LineCtrlParam[i].m_OutNum;j++)
			{
				sprintf_s(str1,"nOutPort%d",j);
				pCard->m_LineCtrlParam[i].m_OutChn[j] = GetPrivateProfileIntA(str,str1,0,strPath);//输出端口
				cchn = pCard->m_LineCtrlParam[i].m_OutChn[j];
				pCard->m_OutportParam[cchn].m_outChn = cchn;
				sprintf_s(str1,"nDelay%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnDelay[j] = GetPrivateProfileIntA(str,str1,0,strPath);//延迟
				sprintf_s(str1,"nDelayIn%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j] = GetPrivateProfileIntA(str,str1,0,strPath);//延迟输入产品个数

				sprintf_s(str1,"nDelayTm%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnDelayTM[j] = GetPrivateProfileIntA(str,str1,0,strPath);//延迟长度（产品间位置0-1023）
				sprintf_s(str1,"nKeep%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnKeep[j] = GetPrivateProfileIntA(str,str1,0,strPath);//输出保持时间长度
				sprintf_s(str1,"nType%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnType[j] = GetPrivateProfileIntA(str,str1,0,strPath);//类型
				pCard->m_OutportParam[cchn].m_OutChnType = pCard->m_LineCtrlParam[i].m_OutChnType[j];
				pCard->m_OutportParam[cchn].m_NoUsed = 1;
				sprintf_s(str1,"nOutCntDelay%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnCntDelay[j] = GetPrivateProfileIntA(str,str1,20,strPath);//计数延迟

				pCard->m_OutportParam[cchn].m_OutChnType = pCard->m_LineCtrlParam[i].m_OutChnType[j];
				pCard->m_OutportParam[cchn].m_NoUsed = 0;//使用该输出

				if(pCard->m_LineCtrlParam[i].m_TmType==0) {
					pCard->m_LineCtrlParam[i].m_OutChnDelay[j]*=500;
					pCard->m_LineCtrlParam[i].m_OutChnKeep[j]*=500;
				}

				if(pCard->m_LineCtrlParam[i].m_TmType==9) {
					pCard->m_LineCtrlParam[i].m_OutChnKeep[j]*=500;
					//发送到C51时,保持一致性
					pCard->m_LineCtrlParam[i].m_OutChnDelay[j] = pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j];
				}

				if( (pCard->m_LineCtrlParam[i].m_OutChnType[j] & 0x80)==0x80) {//输出需要序列
					sprintf_s(str1,"nOutHigh%d",j);
					pCard->m_OutportParam[cchn].m_HighTm = GetPrivateProfileIntA(str,str1,0,strPath);
					pCard->m_LineCtrlParam[i].m_OutHighTm[j] = pCard->m_OutportParam[cchn].m_HighTm;
					sprintf_s(str1,"nOutLow%d",j);
					pCard->m_OutportParam[cchn].m_LowTm = GetPrivateProfileIntA(str,str1,0,strPath);
					pCard->m_LineCtrlParam[i].m_OutLowTm[j] = pCard->m_OutportParam[cchn].m_LowTm;
					sprintf_s(str1,"nOutReCycle%d",j);
					pCard->m_OutportParam[cchn].m_ReCycleNum = GetPrivateProfileIntA(str,str1,0,strPath);
					pCard->m_LineCtrlParam[i].m_OutReCycleNum[j] = pCard->m_OutportParam[cchn].m_ReCycleNum;
				}
				/*LX45输出端口0-3需要输出非均匀脉冲串
				*/
			}
		}
	}
	else if( mWork_Mode==2 ) {//基于输入信号(产品)计数器和2us计数控制
		ReadWorkMode2Param(strPath);
	}
	if (b_Pause)
	{
		for (i = 0; i < 4; i++)
		{
			pCard->m_LineCtrlParam[i].m_Used = 0;
		}
	}
	((CPci6356_ioCtrl*)m_pData)->InitLine();//在通过m_BoardType=LX45内部区分板卡不同
	//ContinueInCntWork(-1);
	return TRUE;
//	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadParameterFile(strPath);
}

BOOL CioControl::ReadWorkMode2Param(const char* strPath)
{
	char str[16],str1[32];
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	sprintf_s(str,"基础参数");
	int i,j;
	//计数器延迟参数
	mCntVOutDelay = GetPrivateProfileIntA(str,"CntVOutDelay",20,strPath);
	((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(mCntVOutDelay);
	for (i=0;i<4;i++)
	{
		sprintf_s(str,"队列%d",i);
		pCard->m_LineCtrlParam[i].m_LineNo = i;
		pCard->m_LineCtrlParam[i].m_Used = GetPrivateProfileIntA(str,"nUsed",0,strPath);//启用开关
		pCard->m_LineCtrlParam[i].m_InChn = GetPrivateProfileIntA(str,"nInPort",0,strPath);//对应输入端口
		pCard->m_LineCtrlParam[i].m_InFilterType = GetPrivateProfileIntA(str,"nInFilter",0,strPath);//对应输入端口
		pCard->m_LineCtrlParam[i].m_InMaxRate = GetPrivateProfileIntA(str,"nMaxRate",10,strPath);//对应输入端口
//		pCard->m_LineCtrlParam[i].m_OutCtrlType = GPci6356_DL_etPrivateProfileInt(str,"nOutCtrlType",0,strPath);//输出隐含控制输出方式：独立/统一
		pCard->m_LineCtrlParam[i].m_OutNum = GetPrivateProfileIntA(str,"nOutPortNum",1,strPath);//个输出控制端口
		pCard->m_LineCtrlParam[i].m_TmType = GetPrivateProfileIntA(str,"nRunType",0,strPath);//队列推进方式
		pCard->m_LineCtrlParam[i].m_LastOutPort = GetPrivateProfileIntA(str,"nLastOutPort",-1,strPath);//最后一个控制端口 
		pCard->m_LineCtrlParam[i].m_LineCntDelay = GetPrivateProfileIntA(str,"nLineCntDelay",20,strPath);//计数延迟
		int cchn;

		for (j=0;j<pCard->m_LineCtrlParam[i].m_OutNum;j++)
		{
			sprintf_s(str1,"nOutPort%d",j);
			pCard->m_LineCtrlParam[i].m_OutChn[j] = GetPrivateProfileIntA(str,str1,0,strPath);//输出端口
			cchn = pCard->m_LineCtrlParam[i].m_OutChn[j];
			sprintf_s(str1,"nDelayIn%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j] = GetPrivateProfileIntA(str,str1,0,strPath);//延迟输入产品个数
			pCard->m_LineCtrlParam[i].m_OutChnDelay[j] = pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j];
			sprintf_s(str1,"nDelayTm%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnDelayTM[j] = GetPrivateProfileIntA(str,str1,0,strPath);//延迟长度（产品间位置0-1023）
			sprintf_s(str1,"nKeep%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnKeep[j] = GetPrivateProfileIntA(str,str1,0,strPath);//持续
			sprintf_s(str1,"nType%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnType[j] = GetPrivateProfileIntA(str,str1,0,strPath);//类型
			pCard->m_LineCtrlParam[i].m_OutChnKeep[j]*=500;
			sprintf_s(str1,"nOutCntDelay%d",j);

			pCard->m_OutportParam[cchn].m_OutChnType = pCard->m_LineCtrlParam[i].m_OutChnType[j];
			pCard->m_OutportParam[cchn].m_NoUsed = 0;
			
			pCard->m_LineCtrlParam[i].m_OutChnCntDelay[j] = GetPrivateProfileIntA(str,str1,20,strPath);//计数延迟
			if( (pCard->m_LineCtrlParam[i].m_OutChnType[j] & 0x80)==0x80) {//输出需要序列
				sprintf_s(str1,"nOutHigh%d",j);
				pCard->m_OutportParam[cchn].m_HighTm = GetPrivateProfileIntA(str,str1,0,strPath);
				pCard->m_LineCtrlParam[i].m_OutHighTm[j] = pCard->m_OutportParam[cchn].m_HighTm;
				sprintf_s(str1,"nOutLow%d",j);
				pCard->m_OutportParam[cchn].m_LowTm = GetPrivateProfileIntA(str,str1,0,strPath);
				pCard->m_LineCtrlParam[i].m_OutLowTm[j] = pCard->m_OutportParam[cchn].m_LowTm;
				sprintf_s(str1,"nOutReCycle%d",j);
				pCard->m_OutportParam[cchn].m_ReCycleNum = GetPrivateProfileIntA(str,str1,0,strPath);
				pCard->m_LineCtrlParam[i].m_OutReCycleNum[j] = pCard->m_OutportParam[cchn].m_ReCycleNum;
			}
			/*LX45输出端口0-3需要输出非均匀脉冲串
			*/
		}
	}
	return TRUE;
}
/*
BOOL CioControl::WriteParameterFile(const char* strPath)
{
	char str[16],str1[64],str2[64];
	for (int i=0;i<4;i++)
	{
		sprintf_s(str,"队列%d",i);
		sprintf_s(str1,"%d		//队列%d启用开关",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_Used,i);
		WritePrivateProfileString(str,"nUsed",str1,strPath);//启用开关
		sprintf_s(str1,"%d		//输入端口",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_InChn);
		WritePrivateProfileString(str,"nInPort",str1,strPath);//输入端口
		sprintf_s(str1,"%d		//输出端口个数",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutNum);
		WritePrivateProfileString(str,"nOutPortNum",str1,strPath);//输出口个数
		sprintf_s(str1,"%d		//队列推进方式",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_TmType);
		WritePrivateProfileString(str,"nRunType",str1,strPath);//队列推进方式
		for (int j=0;j<((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutNum;j++)
		{
			sprintf_s(str1,"nOutPort%d",j);
			sprintf_s(str2,"%d		//队列%d,第%d个输出所用端口",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChn[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
			sprintf_s(str1,"nDelay%d",j);
			sprintf_s(str2,"%d		//队列%d,第%d个输出延迟",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChnDelayTM[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
			sprintf_s(str1,"nKeep%d",j);
			sprintf_s(str2,"%d		//队列%d,第%d个输出持续",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChnKeep[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
			sprintf_s(str1,"nType%d",j);
			sprintf_s(str2,"%d		//队列%d,第%d个输出类型",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChnType[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
		}
		//		Pci6356_InitFpgaLine(m_DevID,&m_LineCtrlParam[i]);
	}
	return TRUE;
//	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_WriteParameterFile(strPath);
}
*/


BOOL CioControl::ResetCount()
{
	//复位计数器
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ResetAllCount();
	return TRUE;
}

BOOL CioControl::ResetALL()
{
	//复位计数器
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ResetAllCount();
	//向C51发初始化命令(注意，不是初始化参数数据)
	((CPci6356_ioCtrl*)m_pData)->InitC51Status();
	return TRUE;
}


BOOL CioControl::StartWork(BOOL needReset)
{
	((CPci6356_ioCtrl*)m_pData)->OP_Start = TRUE;
	if(needReset)
		ResetALL();
	else {
		((CPci6356_ioCtrl*)m_pData)->Pci6356_ResetAllCount();
		((CPci6356_ioCtrl*)m_pData)->Start();
		((CPci6356_ioCtrl*)m_pData)->Pci6356_StartWork();
	}
	ContinueInCntWork(-1);
	return TRUE;
}

BOOL CioControl::StopWork(BOOL cutALL)
{
	((CPci6356_ioCtrl*)m_pData)->OP_Start=FALSE;
	if(cutALL) {
		//设定涉及队列控制的输出端口控制为PC可控，全部设定为无效电平
		
		//通知C51停止输入输出控制，并初始化C51索引和计数器
		((CPci6356_ioCtrl*)m_pData)->InitC51Status();
	}
	((CPci6356_ioCtrl*)m_pData)->Pci6356_StopWork();
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ResetAllCount();
	PauseInCntWork(-1);
	return TRUE;
}

BOOL CioControl::PauseInCntWork(int nPortIn)
{
	if((nPortIn>=0)&&(nPortIn<=7)) {
		if(m_IfPcie32) {
			RegWrite32(0x460+nPortIn*4,0);
		}
		else {
			IOWrite(0x1b,(BYTE)(nPortIn));//选择端口
			IOWrite(0x1c,0);//禁止输入计数
		}
		return TRUE;
	}
	else if(nPortIn==(-1)) {
		int i;
		for(i=0;i<=7;i++) {
			if(m_IfPcie32) RegWrite32(0x460+i*4,0);
			else {
				IOWrite(0x1b,(BYTE)(i));
				IOWrite(0x1c,0);
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CioControl::ContinueInCntWork(int nPortIn)
{
	if((nPortIn>=0)&&(nPortIn<=7)) {
		if(m_IfPcie32) RegWrite32(0x460+nPortIn*4,1);
		else {
			IOWrite(0x1b,(BYTE)(nPortIn));//选择端口
			IOWrite(0x1c,1);//允许输入计数
		}
		return TRUE;
	}
	else if(nPortIn==(-1)) {
		int i;
		for(i=0;i<=7;i++) {
			if(m_IfPcie32) RegWrite32(0x460+i*4,1);
			else {
				IOWrite(0x1b,(BYTE)(i));
				IOWrite(0x1c,1);
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CioControl::RegWrite32(int addr,DWORD mval)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_REGWrite32(addr,mval);
}

BOOL CioControl::RegRead32(int addr,DWORD *pval)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_REGRead32(addr,pval);
}

BOOL CioControl::IOWrite32(int addr,DWORD mval)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_IOWrite32(addr,mval);
}

BOOL CioControl::IORead32(int addr,DWORD *pval)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_IORead32(addr,pval);
}



BOOL CioControl::IOWrite(int addr,BYTE mval)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_IOWrite(addr,mval);
}

BOOL CioControl::IORead(int addr,BYTE *pval)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_IORead(addr,pval);
}

BOOL CioControl::ReadInChanged(UCHAR &instatus,UCHAR &indelaystatus,DWORD &time2uscnt)
{

	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadInChanged(instatus,indelaystatus,time2uscnt);
	
}

UINT CPci6356_ioCtrl::MonitInputThread(LPVOID pParam)
{
	CPci6356_ioCtrl *pdlg = (CPci6356_ioCtrl *)pParam;
	pdlg->mProcInputflag = TRUE;
	UCHAR	inIOChanged = 0;
	UCHAR	indelayIOChanged = 0;
	DWORD	inChangedtime = 0;
	UCHAR ddd = 0;
	while(pdlg->mProcInputflag) {
		if(WaitForSingleObject(pdlg->m_InputEvent,200)==WAIT_OBJECT_0){
			ResetEvent(pdlg->m_InputEvent);
			if (pdlg->m_hWnd) {
				PostMessage((HWND)pdlg->m_hWnd,10003,0,0);
			}
			else {
				//读出所有变化参数
				while(pdlg->Pci6356_ReadInChanged(inIOChanged,indelayIOChanged,inChangedtime)) ;
			}
		}
	}
	ResetEvent(pdlg->m_InputEvent);
	return 0;
}

UINT CPci6356_ioCtrl::MonitC51Thread(LPVOID pParam)
{
	CPci6356_ioCtrl *pdlg = (CPci6356_ioCtrl*)pParam;
	pdlg->mProcC51flag = TRUE;
	char	C51Message[8];
	int remainmsg = 0;
	int nli=0,ind=0;
	while(1)
	{
		pdlg->mProcC51flag = 1;
	while(pdlg->mProcC51flag) {
		if(WaitForSingleObject(pdlg->m_C51Event,200)==WAIT_OBJECT_0){
			ResetEvent(pdlg->m_C51Event);
		}
			//if(pdlg->Pci6356_ReadFromC51(C51Message,remainmsg);
			while(pdlg->Pci6356_ReadFromC51(C51Message,remainmsg)) {
				if( (C51Message[0]==(char)0xcc)&&(C51Message[1]==(char)0xcc) ) //C51初始化完成
				{
					//置C51初始化完成
					pdlg->C51InitFin = TRUE;
					pdlg->initC51Flag = TRUE;
					if(pdlg->OP_Start==TRUE) {
						pdlg->OP_Start=FALSE;
						//如果不是只执行C51初始化，启动工作
						pdlg->Pci6356_ResetAllCount();
						pdlg->Pci6356_StartWork();
						pdlg->EnAllInCntWork(TRUE);
					}
					else {
						pdlg->initC51Flag = FALSE;
					}
				}
				if (pdlg->m_hWnd) {
					if(pdlg->m_EnC51CodeCnt==FALSE)
						PostMessage((HWND)pdlg->m_hWnd,10002,*(int*)(C51Message),*(int*)(C51Message+4));
					else {
						//指向一个存储上传信息的队列,传出队列索引和当前C51命令操作符
						//	MsgLineNum = 0;
						//pMsgLine = new C51MSG_FROMDRV[128];
						memcpy(&(pdlg->pMsgLine[pdlg->MsgLineNum]),C51Message,24);
						PostMessage((HWND)pdlg->m_hWnd,10004,*(int*)(C51Message),(int)pdlg->MsgLineNum);
						pdlg->MsgLineNum++;
						if(pdlg->MsgLineNum>=128) pdlg->MsgLineNum = 0;
						}
//					PostMessage(pdlg->m_hWnd,10002,*(int*)(C51Message),*(int*)(C51Message+4));
/*					if( C51Message[0]==(char)0xc1 ) {//输出变化
						nli = (int)C51Message[1];
						ind = (int)C51Message[2];
						//将已输出状态参数初始化
						pdlg->pLineCtrlStatus[nli].mOutCtrlStatus[ind].out_Ctrl = 0;
						//pdlg->pLineCtrlStatus[C51Message[1]].mOutCtrlStatus[C51Message[2]].out_Ctrl = 0;	//再由SeResult/ex修改状态
						//pLineCtrlStatus[C51Message[1]].mOutCtrlStatus[C51Message[2]].in_Status = 0;	
					}*/
				}
/*				else if( C51Message[0]==(char)0xc0 ) //输入变化
				{
					if (pdlg->m_hWnd)
						PostMessage(pdlg->m_hWnd,20001,C51Message[1],C51Message[2]);//当前队列和输入索引计数
				}
				else if( C51Message[0]==(char)0xc1 ) //输出变化
				{
					if (pdlg->m_hWnd)
						PostMessage(pdlg->m_hWnd,20002,C51Message[1],C51Message[2]);//当前队列和输出索引计数
				}*/
			}
		//}
	}
	//AfxMessageBox("Exit C51");
}
	ResetEvent(pdlg->m_C51Event);
	return 0;
}

UINT CPci6356_ioCtrl::Monit500usThread(LPVOID pParam)
{
	CPci6356_ioCtrl *pdlg = (CPci6356_ioCtrl *)pParam;
	pdlg->mProc500usflag = TRUE;
	pdlg->m_500uscnt = 0;
	while(pdlg->mProc500usflag) {
		if(WaitForSingleObject(pdlg->m_500usEvent,200)==WAIT_OBJECT_0){
			ResetEvent(pdlg->m_500usEvent);
			pdlg->m_500uscnt++;
			if (pdlg->m_hWnd)PostMessage((HWND)pdlg->m_hWnd,10001,pdlg->m_500uscnt,0);
		}
	}
	ResetEvent(pdlg->m_500usEvent);
	return 0;
}


BOOL	CPci6356_ioCtrl::InitLine()
{
	int i;
	int type = 0;

	int j;
	for(i=0;i<4;i++) {
		pLineCtrlStatus[i].mLine = i;
//		pLineCtrlStatus[i].mCtrlType = m_LineCtrlParam[i].m_OutCtrlType;
//		if(pLineCtrlStatus[i].mCtrlType==0) {
			for(j=0;j<256;j++) {
				pLineCtrlStatus[i].mOutCtrlStatus[j].out_Ctrl = 0;
			}
//		}
//		else {
//			for(j=0;j<256;j++) {
//				pLineCtrlStatus[i].mOutCtrlStatus[j].out_Ctrl = 0x00ffffff;
//			}
//		}
	}
	//关闭所有输出
// 	FPGA_OUTPUT_PARAM	iniddd;
// 	memset(&iniddd,0,sizeof(iniddd));
// 	for(i=0;i<24;i++) {
// 		iniddd.m_NoUsed = 1;
// 		iniddd.m_outChn = i;
// 		Pci6356_InitOutputCh(&iniddd);
// 	}
	//2018 初始化 设置所有输出为PC控制（而不是关闭所有输出）
	BYTE aaa;
	if(m_If32) {
		for(i=0;i<24;i++) {
			Pci6356_REGWrite32(0x0600+i*4,0);
		}
	}
	else {
		for(i=0;i<24;i++) {
			aaa = (BYTE)(i&0x1f);
			Pci6356_IOWrite(0x05,aaa);//选择通道
			Pci6356_IOWrite(0x13,0);//PC控制
		}
	}

	//初始化队列参数
	for(i=0;i<4;i++) {
		Pci6356_InitFpgaLine(&m_LineCtrlParam[i]);
		if(m_LineCtrlParam[i].m_Used==1) {//如果队列有效，置该队列有效位比特=1
			type |= 1<<i;
		}
	}

	
	//置初始化未完成
	C51InitFin = FALSE;
	//初始化队列状态，关闭所有队列
	//Pci6356_InitWork(type,3,0);//根据配置打开输入中断，第3个参数，用配置文件中EnInputINT设置
	Pci6356_InitWork(type,7,mEnInIntFlag);//根据配置打开输入中断，第3个参数，用配置文件中EnInputINT设置,第二个参数打开所有中断,包括看门狗
	//暂停所有输入计数
	EnAllInCntWork(FALSE);
	//复位
	Pci6356_ResetAllCount();
	ResetEvent(m_InputEvent);
	ResetEvent(m_C51Event);
	ResetEvent(m_500usEvent);


	//初始化C51通信,只允许C51中断，关闭输入使能
	//接收到C51初始化完成后，关闭所有计数，再打开输入使能
	Pci6356_InitC51Comm();

	Start();

	//发C51初始化数据
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//参数初始化数据
	paramData.CLength = 48;//参数长度48字节
	paramData.CParam = 0xf0f0f0f0;//数据类型队列初始化

	int m,n,occl,addst;
	if(WorkMode>=100) {
		//如果是C51控制队列与输出工作模式，将参数数据写到FPGA参数交换区
		SetFPGA_C51Param_BYTE(0,WorkMode);
		m = 0;
		n = 0;
		occl = 0;
		for(i=0;i<4;i++) {//缺最后输出端口号
			if(m_LineCtrlParam[i].m_Used) {
				SetFPGA_C51Param_BYTE(4+i,1);
				SetFPGA_C51Param_BYTE(8+i,m_LineCtrlParam[i].m_InChn);
				m++;
				n+= m_LineCtrlParam[i].m_OutNum;
				SetFPGA_C51Param_BYTE(16+i,m_LineCtrlParam[i].m_OutNum);
				SetFPGA_C51Param_BYTE(20+i,m_LineCtrlParam[i].m_LastOutPort);
				for(j=0;j<m_LineCtrlParam[i].m_OutNum;j++) {
					SetFPGA_C51Param_BYTE(32+occl,m_LineCtrlParam[i].m_OutChn[j]);

					addst = 64 + occl * 16;
					SetFPGA_C51Param_BYTE(addst,i);
					SetFPGA_C51Param_BYTE(addst+1,m_LineCtrlParam[i].m_InChn);
					SetFPGA_C51Param_BYTE(addst+2,m_LineCtrlParam[i].m_OutChn[j]);
					SetFPGA_C51Param_BYTE(addst+3,(BYTE)m_LineCtrlParam[i].m_OutChnType[j]);
					SetFPGA_C51Param_DWORD(addst+4,m_LineCtrlParam[i].m_OutChnDelay[j]/512);
					SetFPGA_C51Param_DWORD(addst+8,m_LineCtrlParam[i].m_OutChnKeep[j]/512);
					occl++;
				}
			}
			else {
				SetFPGA_C51Param_BYTE(4+i,0);
				SetFPGA_C51Param_BYTE(8+i,7);//可不设
				SetFPGA_C51Param_BYTE(16+i,23);//可不设
			}
		}
		SetFPGA_C51Param_BYTE(2,m);
		SetFPGA_C51Param_BYTE(3,n);

		paramData.CType = 0x5555;//参数初始化数据
		paramData.CLength = 8;//参数长度48字节
		paramData.CParam = 0xf0f0f0f0;//数据类型队列初始化
		OP_Start=TRUE;
		Pci6356_SendDataToC51(&paramData);
		return TRUE;
	}

	char	initlinedata[64];
	int lastlg,ll1,ll2;
	ll2 = 0;

	//WorkMode = 0;
	for(i=0;i<4;i++) {
		if(m_LineCtrlParam[i].m_Used==1) {
			initlinedata[i*3] = 1;
			initlinedata[i*3+1] = m_LineCtrlParam[i].m_InChn;
			initlinedata[12+i] = (m_LineCtrlParam[i].m_OutNum-1);
			if(initlinedata[12+i]<0) initlinedata[12+i] = 0;
			initlinedata[40+i] = m_LineCtrlParam[i].m_TmType;//时间逻辑
//			initlinedata[44+i] = m_LineCtrlParam[i].m_OutCtrlType;//独立/统一
			if( (initlinedata[40+i]>4) || (initlinedata[40+i]<0) ) 
				initlinedata[40+i] = 0;//使用2us计数
			//得到最后一个执行输出端口
			//////////////////////////////////////
			//2017-10-25,使用配置文件得到最后端口，为兼容最早的版本，没有lastoutport配置项，自动得到，否则使用配置文件设定
			if(m_LineCtrlParam[i].m_LastOutPort==-1) {
				//兼容最早版本
				lastlg = m_LineCtrlParam[i].m_OutChnDelay[0];// + m_LineCtrlParam[0].m_OutChnKeep[0];2017-08-25修改错误m_LineCtrlParam[0]为m_LineCtrlParam[i]
				initlinedata[i*3+2] = m_LineCtrlParam[i].m_OutChn[0];
				for(j=1;j<m_LineCtrlParam[i].m_OutNum;j++) {
					ll1 = m_LineCtrlParam[i].m_OutChnDelay[j];// + m_LineCtrlParam[i].m_OutChnKeep[j];
					if(ll1>=lastlg) {
						initlinedata[i*3+2] = m_LineCtrlParam[i].m_OutChn[j];
						lastlg = ll1;
					}
				}
			}
			else {
				//使用配置项
				initlinedata[i*3+2] = m_LineCtrlParam[i].m_LastOutPort;
			}
			/////////////////////////////////////////////
			for(j=0;j<m_LineCtrlParam[i].m_OutNum;j++) {
				if(m_LineCtrlParam[i].m_OutChn[j]!=initlinedata[i*3+2]) {
					initlinedata[16+ll2] = m_LineCtrlParam[i].m_OutChn[j];
					ll2++;
				}
			}
		}
		else {
			initlinedata[i*3] = 0;//不用
			initlinedata[i*3+1] = 7;//用最后一个容错
			initlinedata[i*3+2] = 23;//用最后一个容错
			initlinedata[12+i] = 0;//没有输出
			initlinedata[40+i] = 0;//使用2us计数
		}
	}
	memcpy(paramData.DataBuff,initlinedata,48);//数据类型队列初始化
	OP_Start=TRUE;
	Pci6356_SendDataToC51(&paramData);

	return	TRUE;
}

BOOL CPci6356_ioCtrl::InitC51Status()
{
	SENDTOC51CMD paramCmd;
	paramCmd.CType = 0xaaaa;
	paramCmd.lg = 8;
	paramCmd.Command[0] = 0xcc;
	paramCmd.Command[1] = 0xcc;
	paramCmd.Command[2] = 0xcc;
	paramCmd.Command[3] = 0xcc;
	paramCmd.Command[4] = 0xcc;
	paramCmd.Command[5] = 0xcc;
	Sleep(10);//确保上一个消息发送已完成
	Pci6356_SendCmdToC51(&paramCmd);
	if(mProcC51flag==TRUE) {
		//如果C51消息处理线程已启动，由消息处理线程
		initC51Flag = FALSE;
		return TRUE;
	}
	else {//没有C51消息处理线程，在此处理
		Sleep(100);//C51复位延迟
		if(WaitForSingleObject(m_C51Event,1000)==WAIT_OBJECT_0){
			ResetEvent(m_C51Event);
			char C51Message[8];
			int remainmsg = 0;
			while(Pci6356_ReadFromC51(C51Message,remainmsg)) {
				if( (C51Message[0]==(char)0xcc)&&(C51Message[1]==(char)0xcc) ) //C51初始化完成
				{
					//置C51初始化完成
					C51InitFin = TRUE;
					initC51Flag = TRUE;
					if(OP_Start==TRUE) {
						OP_Start=FALSE;
						//如果不是只执行C51初始化，启动工作
						Pci6356_ResetAllCount();
						Pci6356_StartWork();
						EnAllInCntWork(TRUE);
					}
					else {
						initC51Flag = FALSE;
					}
				}
			}
		}
		else return FALSE;
	}
	return TRUE;
}

BOOL CPci6356_ioCtrl::Start()
{
	m_C51Event = mC51Event;
	m_500usEvent = m500UsEvent;
	m_InputEvent = mInputEvent;
	ResetEvent(m_InputEvent);
	ResetEvent(m_C51Event);
	ResetEvent(m_500usEvent);
	if(!mProcInputflag)
		//AfxBeginThread(MonitInputThread,this);
	if(!mProcC51flag)
		//AfxBeginThread(MonitC51Thread,this);
	if(!mProc500usflag)
		//AfxBeginThread(Monit500usThread,this);
	
	return TRUE;
}

//读板卡号
int  CioControl::ReadBoardNo()
{
	BYTE ddd;
	if(m_IfPcie32) {
		DWORD dval;
		IOWrite32(0x03c,0);
		IORead32(0x010,&dval);
		return dval;
	}
	else {
		IOWrite(0x1d,0);
		IOWrite(0x1e,0);
		IORead(0x07,&ddd);
		if(ddd>=200)	return -1;		
		else return (int)ddd;
	}
}

int  CioControl::GetHardVer(char *HardVer)//读板卡服务的固件版本信息
{
	char ccc[256];
	((CPci6356_ioCtrl*)m_pData)->Pci6356_GetHardVer(ccc);
	int l = strlen(ccc);
	ccc[l] = '+';l++;
	((CPci6356_ioCtrl*)m_pData)->Pci6356_GetSysVer(&ccc[l]);
	l = strlen(ccc);
	memcpy(HardVer,ccc,l);
	HardVer[l] = 0;
	return l;
}
int  CioControl::GetSoftVer(char *SoftVer)//读板卡服务的软件版本信息
{
	char ccc[256];
	int l,l1;
	memset(ccc,0,256);
	l1 = strlen(DLLversion);
	memcpy(ccc,DLLversion,l1);
	ccc[l1] = '+';l1++;
	((CPci6356_ioCtrl*)m_pData)->Pci6356_GetLibVer(&ccc[l1]);
	l = strlen(ccc);
	memcpy(SoftVer,ccc,l);
	SoftVer[l] = 0;
	return l;
}

//读板卡服务的系统版本信息,最长1000
int  CioControl::ReadBoardInfo(char *sysVer)
{
	BYTE ddd,dd1;
	int l;
	if(m_IfPcie32) {
		DWORD dval;
		IOWrite32(0x3c,2);
		IORead32(0x010,&dval);
		ddd = (BYTE)dval;
		IOWrite32(0x3c,3);
		IORead32(0x010,&dval);
		dd1 = (BYTE)dval;
		l = (int)dd1;
		l = l*256 + ddd; 
		if(l>1000) return 0;
		if(l<=0) return 0;
		for(int i=0;i<l;i++) {
			IOWrite32(0x03c,i+4);//
			IORead32(0x010,&dval);
			sysVer[i] = (char)dval;
			
		}
// 		RegRead32(0x2000,&dval);
// 		l = dval;
// 		for(int i=0;i<256;i++){
// 			RegRead32(0x2008+i*4,&dval);
// 			sysVer[i] = dval;	
// 		}
// 		sysVer[l] = 0;
		return l;
	}
	IOWrite(0x1d,2);//地址2为长度
	IOWrite(0x1e,0);
	IORead(0x07,&ddd);
	IOWrite(0x1d,3);//地址2为长度
	IORead(0x07,&dd1);
	l = (int)dd1;
	l = l*256 + ddd; 
	if(l>1000) return 0;
	if(l<=0) return 0;
	for(int i=0;i<l;i++) {
		IOWrite(0x1d,i+4);//
		IOWrite(0x1e,0);
		IORead(0x07,&dd1);
		sysVer[i] = dd1;
	}
	sysVer[l] = 0;
	return l;
}

//写板卡号
BOOL  CioControl::WriteBoardNo(int nbd)
{
	SENDTOC51CMD paramCmd;
	paramCmd.lg = 2;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x33;
	paramCmd.Command[1] = (BYTE)nbd;
	paramCmd.Command[2] = (BYTE)nbd;
	Sleep(10);//确保上一个消息发送已完成
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendCmdToC51(&paramCmd);
	
}

//写板卡服务的系统版本信息
BOOL  CioControl::WriteBoardVer(char *sysVer)
{
	//发送0xfd
	int i,lg;
	lg = strlen(sysVer);
	if(lg>1000) lg = 1000;
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//发送数据
	paramData.CLength = lg+2;//数据长度
	paramData.CParam = 0xfdfdfdfd;//版本写入数据
	paramData.DataBuff[0] = (BYTE)lg;
	paramData.DataBuff[1] = (BYTE)(lg>>8);
	for(i=0;i<lg;i++) {
		paramData.DataBuff[i+2] = sysVer[i];
	}
	Sleep(10);//确保上一个消息发送已完成
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendDataToC51(&paramData);
}

//////////////////////////////////////////////////////////////
//2017-12-15新增参数设置与计数控制接口函数，谨慎使用

BOOL CioControl::InitInputCh(int chn,int Filter,float maxRate)//Filter:边缘滤波参数,maxRate:脉冲合并参数
{

	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_InitInputCh(chn,Filter,maxRate);

}

//设置输出参数:延迟参数与保持参数,注意各种模式不同,输入计数控制\时间基准控制\编码器控制
BOOL CioControl::InitOutputCh(PFPGA_OUTPUT_PARAM paramCh)
{
	FPGA_OUTPUT_PARAM *poldParam;
	FPGA_INPUT_LINE *pLin;
	int chn;
	chn = paramCh->m_outChn;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	//关联队列错误，返回
	if(poldParam->m_RelLine!=paramCh->m_RelLine) return FALSE;
	//如果使用2us计数，需要将时间参数*500形成豪秒计数值，说明书中写明
	memcpy(poldParam,paramCh,sizeof(FPGA_OUTPUT_PARAM));
	//得到关联队列时间控制属性
	int lin = poldParam->m_RelLine;
	pLin = &(((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[lin]);
	if(pLin->m_TmType==0) {
		poldParam->m_OutChnDelay *= 500;
		poldParam->m_OutChnKeep *= 500;
	}
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_InitOutputCh(poldParam);
}

//不修改原配置参数属性，直接控制板卡，不能设置最后一个端口使能，需要C51配合
BOOL CioControl::SetOutputType(int chn,BOOL ifAuto,BOOL ifKick)//是否自动/控制，如果是控制端口是否需要补剔
{
	FPGA_OUTPUT_PARAM dParam;
	FPGA_OUTPUT_PARAM *poldParam;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	memcpy(&dParam,poldParam,sizeof(FPGA_OUTPUT_PARAM));
	DWORD ddd = dParam.m_OutChnType;
	if(ifAuto) {
		ddd |= 0x01;//确认队列工作
		ddd &= 0x0fffd;//xxx01，非受控端口
	}
	else {
		ddd |= 0x03;//确认队列工作，受控端口
		if(ifKick) {
			ddd |= 0x20;//bb0bbbbb
		}
		else {
			ddd |= 0x0ffdf;//bb0bbbbb
		}
	}

	if(m_IfPcie32) {
		DWORD dval,moff;
		dval = ddd;
		moff = chn*4; 
		RegWrite32(0x0600+moff,dval);
	}
	else {
		//选择端口号
		BYTE aaa;
		aaa = chn;
		IOWrite(0x05,aaa);
		aaa = (BYTE)ddd;
		IOWrite(0x13,aaa);
	}
	dParam.m_OutChnType = ddd;
	memcpy(poldParam,&dParam,sizeof(FPGA_OUTPUT_PARAM));
	return TRUE;
}

//设置衡水模式（时间/编码器计数延迟控制）
BOOL CioControl::SetOutputTm(int chn,int delayTM,int keepTM)
{
	if(mWork_Mode!=0) return FALSE;
	FPGA_OUTPUT_PARAM *poldParam;
	FPGA_INPUT_LINE *pLin;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	int lin = poldParam->m_RelLine;
	pLin = &(((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[lin]);
	if(pLin->m_TmType==0) {
		poldParam->m_OutChnDelay = delayTM * 500;
		poldParam->m_OutChnKeep = keepTM * 500;
	}
	else {
		poldParam->m_OutChnDelay = delayTM;
		poldParam->m_OutChnKeep = keepTM;
	}

	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_InitOutputCh(poldParam);
}

//设置无锡模式（输入计数延迟控制）
BOOL CioControl::SetOutputTm(int chn,int delayIN,int delayTM,int keepTM)
{
	if(mWork_Mode!=2) return FALSE;
	FPGA_OUTPUT_PARAM *poldParam;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	poldParam->m_OutChnDelayIN = delayIN;
	poldParam->m_OutChnDelayTM = delayTM;
	poldParam->m_OutChnKeep = keepTM * 500;
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_InitOutputCh(poldParam);
}

//不修改原配置参数属性，直接控制板卡
BOOL CioControl::SetOutputSqu(int chn,int HightM,int LowTM,int PluseNum,BOOL enSqu)
{
	FPGA_OUTPUT_PARAM dParam;
	FPGA_OUTPUT_PARAM *poldParam;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	memcpy(&dParam,poldParam,sizeof(FPGA_OUTPUT_PARAM));
	DWORD ddd = dParam.m_OutChnType;
	if(enSqu) ddd |= 0x80;
	else ddd &= 0xff7f;
	if(m_IfPcie32) {
		DWORD dval,moff;
		dval = ddd;
		moff = chn*4; 
		RegWrite32(0x0600+moff,dval);

		dval = HightM;
		RegWrite32(0x0680+moff,dval);
		dval = LowTM;
		RegWrite32(0x0700+moff,dval);
		dval = PluseNum;
		RegWrite32(0x0780+moff,dval);
	}
	else {
		//选择端口号
		BYTE aaa;
		aaa = chn;
		IOWrite(0x05,aaa);
		aaa = (BYTE)ddd;
		IOWrite(0x13,aaa);

		aaa = (BYTE)((HightM)&0x0ff); 
		IOWrite(0x60,aaa);
		aaa = (BYTE)((LowTM)&0x0ff); 
		IOWrite(0x61,aaa);
		aaa = (BYTE)((PluseNum)&0x0ff); 
		IOWrite(0x62,aaa);
		aaa = (BYTE)(((HightM)>>8)&0x0ff); 
		IOWrite(0x63,aaa);
		aaa = (BYTE)(((LowTM)>>8)&0x0ff); 
		IOWrite(0x64,aaa);
	}
	return TRUE;

}

//设置索引延迟计数器延迟输出时间
BOOL CioControl::SetDelayCntParam(int nDelay)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(nDelay);	
}

/*
OutType: bit0=0/1 -> InPluse(输入脉冲)/exLine(输入延迟输出)
OutType: bit1=0/1 -> exLine输出控制的脉冲参数编码器/2us定时器
输出A/B/Z的级联编码器选择：0-3 Coder
输出C的级联输入端口：0-7 coein
输出相对输入的延迟脉冲数delayIn
输出相对于输入的时间延迟计数delayTm（由OutType：bit1选择时钟源）
输出脉冲保持宽度keep（由OutType：bit1选择时钟源）
*/
BOOL	CioControl::SetExCodeLine(BOOL enWork,int OutType,int Coder,int coein,int delayIn,int delayTm,int keep)
{
	DWORD dval = 0;
	if(!enWork) {
		RegWrite32(0x230,0);
		return TRUE;
	}
	//
	RegWrite32(0x218,Coder&0x03);
	RegWrite32(0x21c,coein&0x07);
	if( (OutType&0x01)==0x01) {
		RegWrite32(0x220,delayIn);
		if((OutType&0x02)==0x02) {//编码器
			RegWrite32(0x224,delayTm);
			RegWrite32(0x228,keep);
		}
		else {
			dval = delayTm * 500;
			RegWrite32(0x224,dval);
			dval = keep * 500;
			RegWrite32(0x228,dval);
		}
	}
	//开始工作
	dval = 0x04 | (OutType&0x03);
	RegWrite32(0x230,dval);
	
	return TRUE;
}

//软件看门狗:系统看门狗和用户软件看门狗
//Alarmperiod 秒 （5秒的倍数间隔）
BOOL	CioControl::SetSoftAlerm(BOOL usersoft,BOOL sysdriver,int Alarmperiod)
{
	DWORD dval=0;
	if(usersoft) dval |= 0x02;
	if(sysdriver) dval |= 0x01;
	if(m_IfPcie32) {
		IOWrite32(0x084,1);//复位系统告警
		IOWrite32(0x088,1);//复位软件告警
		IOWrite32(0x010,dval);
		if(Alarmperiod>=5)
			RegWrite32(0x110,Alarmperiod/5);//将秒转换为5秒
		return TRUE;
	}
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	//20180325
	if(pCard->m_BoardType == 1){//板类型 LX45 可以有ALARM
		IOWrite(0x03a,1);//复位系统告警
		IOWrite(0x03b,1);//复位软件告警
		IOWrite(0x040,(BYTE)(dval));//复位软件告警
		if(Alarmperiod>=5)
			IOWrite(0x41,(BYTE)(Alarmperiod/5));
		return TRUE;
	}
	return FALSE;
}

BOOL	CioControl::ReSetSoftAlerm()//喂狗操作
{
	if(m_IfPcie32) {
		IOWrite32(0x084,1);//复位系统告警
		return IOWrite32(0x088,1);//复位软件告警
	}
	//20180325
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	if(pCard->m_BoardType == 1){//板类型 LX45 可以有ALARM
		IOWrite(0x03a,1);//复位系统告警
		return IOWrite(0x03b,1);//复位软件告警
	}
	else return FALSE;
}

BOOL	CioControl::SetAlermCoeOut(int Acoeout,int Alevel)
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	BYTE aaa;
	if(pCard->m_BoardType == 1){//板类型 LX45 可以有ALARM
		aaa = (BYTE)Acoeout;
		IOWrite(0x042,aaa);//设置关联警告通道
		aaa = (BYTE)(Alevel&0x01);
		return IOWrite(0x043,aaa);//设置输出电平
	}
	else return FALSE;

}
//20150325
BOOL	CioControl::SetOutMaxPluse(int MaxWith)
{
	if(m_IfPcie32) {
		return RegWrite32(0x248,MaxWith&0x1fff);//
	}
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	BYTE aaa;
	if(pCard->m_BoardType == 1){//板类型 LX45 可以有ALARM
		aaa = (BYTE)(MaxWith>>8);
		IOWrite(0x045,aaa&0x1f);//设置高5位
		aaa = (BYTE)(MaxWith&0xff);
		return IOWrite(0x044,aaa);//设置低8位
	}
	else return FALSE;

}


//////////////////////回读输出控制参数
//回读输出控制参数，回读初值设定
BOOL		CioControl::ReadOutPpreCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam )
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadOutPpreCtrl(port,OutCtrlParam));
}

//回读输出控制参数，回读在线控制输出,工作时采集输出端口控制参数输出值
BOOL		CioControl::ReadOutPinlCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam )
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadOutPinlCtrl(port,OutCtrlParam));
}


//回读队列控制和索引参数，从FPGA读出队列控制参数和索引队列状态
BOOL		CioControl::ReadLineIndex(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam,DWORD  *sqIndexOpStatu,PUCHAR sqExKeep)
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadLineIndex(nline,inLineParam,sqIndexOpStatu,sqExKeep));
}


//回读队列控制参数，从FPGA读出队列控制参数（不读索引）
BOOL		CioControl::ReadLineCtrl(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam )
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadLineCtrl(nline,inLineParam));
}


//回读历史消息队列
BOOL		CioControl::ReadMsgSQ(PUCHAR pmsg,int num)
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadMsgSQ(pmsg,num));
}

