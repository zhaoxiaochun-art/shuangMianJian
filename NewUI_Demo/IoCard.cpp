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
//	int			mCtrlType;//1 - �������߼�����̬=�Զ����ܿ����ͨ������ȫ����Ҫ�����0 - �����߼�����̬=�ܿ����ͨ�������������
//	int			mIndepType;//1 - ������������ƣ�0 - �����ͳһ���ƣ�
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

	//�ض�������Ʋ������ض���ֵ�趨
	BOOL		ReadOutPpreCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam );
	//�ض�������Ʋ������ض����߿������,����ʱ�ɼ�����˿ڿ��Ʋ������ֵ
	BOOL		ReadOutPinlCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam );

	//�ض����п��ƺ�������������FPGA�������п��Ʋ�������������״̬
	BOOL		ReadLineIndex(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam,DWORD  *sqIndexOpStatu,PUCHAR sqExKeep);
	//�ض����п��Ʋ�������FPGA�������п��Ʋ���������������
	BOOL		ReadLineCtrl(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam );
	//�ض���ʷ��Ϣ����
	BOOL		ReadMsgSQ(PUCHAR pmsg,int num);

};


//�ض�������Ʋ������ض���ֵ�趨
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

//�ض�������Ʋ������ض����߿������,����ʱ�ɼ�����˿ڿ��Ʋ������ֵ
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


//�ض����п��ƺ�������������FPGA�������п��Ʋ�������������״̬
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

//�ض����п��Ʋ�������FPGA�������п��Ʋ���������������
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

//�ض���ʷ��Ϣ����
BOOL		CPci6356_ioCtrl::ReadMsgSQ(PUCHAR pmsg,int num)
{
	//�����FPGA�ڲ�������Ϣ����һ�飬ʹ�ö�ʹ�ܣ�����FPGA��Ϣ���в���
	int i;
	PUCHAR ppp = pmsg;
	//�Ƶ���Ҫ���ͷ��
	for(i=0;i<(128-num);i++) {
		Pci6356_IOWrite(0x84,0x39);
	}
	//ȡ����Ҫ��Ϣ����ʱ��˳��
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



//�����������빤��
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

//ʹ��C51�������й��̣��ݲ�ʵ�� WORK >100
void CPci6356_ioCtrl::SetFPGA_C51Param_BYTE(int addr,BYTE cval)
{
	Pci6356_IOWrite(0x80,(BYTE)addr);
	Pci6356_IOWrite(0x81,(BYTE)(addr>>8)&0x3f);
	Pci6356_IOWrite(0x84,cval);
}
//ʹ��C51�������й��̣��ݲ�ʵ�� WORK >100
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
	if(m_LockComm) return FALSE;//�����������������������
	m_LockComm = TRUE;
	return TRUE;
}

BOOL CPci6356_ioCtrl::UnlockComm()
{
	if(!m_LockComm) return FALSE;//���δ���������ؽ�������
	m_LockComm = FALSE;
	return TRUE;
}

BOOL CPci6356_ioCtrl::IfLockComm()
{
	return m_LockComm;//����������־
}



CPci6356_ioCtrl::CPci6356_ioCtrl()
{
	m_LockComm = FALSE;
	mProc500usflag = FALSE;
	mProcC51flag = FALSE;
	mProcInputflag = FALSE;
	int i;
	for(i=0;i<24;i++) {
		OutPortCtrlStatus[i] = i;//�������״̬Ϊ��ֹ������Ϊ���߼����߼�=0�����Ϊ��ֹ��ͨ
	}
	for(i=0;i<8;i++) {
		InPortCtrlStatus[i] = 0;//���������Ϊ��ֹ����
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

//��DLL��ά�������е�256������������˿���״̬
//���ڶ��ܿ����ͨ�������������
//ÿ�ο���һ�����
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
		ddd |= 0x03000000;//����ִ��
		pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= ddd;//�����������ִ�У������ÿ��ƶ˿�״̬Ϊ��Ч
	}
	else {
		ddd = ddd << i;
		ddd = ~ddd;
		ddd = (ddd & 0x00ffffff );
		pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl &= ddd;
		pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x01000000;//�ǿ����Զ�ִ������
	}
	SENDTOC51CMD paramCmd;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x01;//�߷�����
	paramCmd.Command[1] = nChannel;//���к�
	paramCmd.Command[2] = nIndex;//����ֵ
	paramCmd.Command[3] = (BYTE)ddd;//
	paramCmd.Command[4] = (BYTE)(ddd>>8);//
	paramCmd.Command[5] = (BYTE)(ddd>>16);//
	paramCmd.Command[6] = (BYTE)(ddd>>24);//
	paramCmd.Command[7] = 0;
	paramCmd.lg = 8;
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendCmdToC51(&paramCmd);
}

//ÿ�ο��ƶ�����
BOOL CioControl::SetOutCtrlByResultEx(int nChannel,int nIndex,int nResult)//ÿ�ο���һ�������Ʒ������M�����
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	DWORD ddd,dd1,dd2;
	ddd = 1;
	int i,j,nn;
	nn = pIO->m_LineCtrlParam[nChannel].m_OutNum;
	
	//�յ���������
	//���ظ�������
	//pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x03000000;
	
	//20171026�޸ĺ󣬲����ظ�����������һ�β������
	pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl = 0x03000000;

	for(j=0;j<nn;j++) {
		dd1 = 1 << j;
		dd2 = nResult & dd1;
		i = pIO->m_LineCtrlParam[nChannel].m_OutChn[j];
		if(i>=24) return FALSE;
		if(dd2!=0) {
			ddd = 1 << i;
//			ddd |= 0x03000000;//����ִ��
			pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= ddd;//�����������ִ�У������ÿ��ƶ˿�״̬Ϊ��Ч
		}
/*		else {
			ddd = 1 << i;
			ddd = ~ddd;
			ddd = (ddd & 0x00ffffff );
			pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl &= ddd;
			pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x01000000;//�ǿ����Զ�ִ������
		}*/
	}

	ddd = pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl;
	int nextI;
	nextI  = nIndex + 10;
//	nIndex++;
	//if(nextI>255) nextI = 0;//������ʹ�ò���1ʱ,NEXT��λ����ȷ
	if(nextI>=256) nextI = nextI - 256;//������ʹ�ò���1ʱ,NEXT��λ����ȷ
	//����һ�ε�������λ,��֤������Ʋ�����λ,ѭ������ʱ�������,
	//����ͨ��nextI  = nIndex + nn;����nn�����˳����õĶ�������
	//��˱���˳����������,�ڶ��߳�ʱͬ������,������ֺ������������,���ܻ���ִ���
	pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nextI].out_Ctrl = 0;

	SENDTOC51CMD paramCmd;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x01;//�߷�����
	paramCmd.Command[1] = nChannel;//���к�
	paramCmd.Command[2] = nIndex;//����ֵ
	paramCmd.Command[3] = (BYTE)ddd;//
	paramCmd.Command[4] = (BYTE)(ddd>>8);//
	paramCmd.Command[5] = (BYTE)(ddd>>16);//
	paramCmd.Command[6] = (BYTE)(ddd>>24);//
	paramCmd.Command[7] = 0;
	paramCmd.lg = 8;
	return pIO->Pci6356_SendCmdToC51(&paramCmd);
}

//ÿ�ο��ƶ�����ж�������Ķ�����
BOOL CioControl::SetOutCtrlByResultEx2(int num,PCTRLOUT_RESULT_INDEP pCtrl)
{
	CPci6356_ioCtrl *pIO = (CPci6356_ioCtrl*)m_pData;
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//������ʼ������
	paramData.CLength = 1 + num * 4;//��������16�ֽ�
	paramData.CParam = 0xf1f1f1f1;//�������Ͷ��г�ʼ��
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
				ddd |= 0x03000000;//����ִ��
				pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= ddd;//�����������ִ�У������ÿ��ƶ˿�״̬Ϊ��Ч
			}
			else {
				ddd = 1 << i;
				ddd = ~ddd;
				ddd = (ddd & 0x00ffffff );
				pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl &= ddd;
				pIO->pLineCtrlStatus[nChannel].mOutCtrlStatus[nIndex].out_Ctrl |= 0x01000000;//�ǿ����Զ�ִ������
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
	paramCmd.Command[0] = 0x01;//�߷�����
	paramCmd.Command[1] = nChannel;//���к�
	paramCmd.Command[2] = nIndex;//����ֵ
	paramCmd.Command[3] = nResult;//�߷Ͽ��Ʋ���
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
	paramData.CType = 0x5555;//������ʼ������
	paramData.CLength = 1 + num * 4;//��������16�ֽ�
	paramData.CParam = 0xf1f1f1f1;//�������Ͷ��г�ʼ��
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
	//nChannel ��Ӧ��������������ȡ������˿ں�
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
	//�õ����ж�Ӧ������˿ں�
	inputn = ((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[nChannel].m_InChn;
	//nChannel ��Ӧ��������������ȡ������˿ں�
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
	//nChannel ��Ӧ��������������ȡ������˿ں�
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
	//nChannel ��Ӧ��������������ȡ������˿ں�
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
	//nChannel ��Ӧ��������������ȡ������˿ں�
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
	//nChannel ��Ӧ��������������ȡ������˿ں�
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
	if(m_IfPcie32){//�ݲ�ʵ��
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

//�������������ʼ��
BOOL CioControl::ReadParameterFile_NewLX45(const char* strPath)
{
	char str[16],str1[32],strbd[32];
	int i,j;
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	pCard->m_BoardType = 1;//���ð����� LX45
	//���û�������
	sprintf_s(str,"��������");
	//�������ӳٲ���,����ֵ�ӳ����ʱ��
	mCntVOutDelay = GetPrivateProfileIntA(str,"CntVOutDelay",20,strPath);
	((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(mCntVOutDelay);
	//�����չ���ò���,�ܿز���ʱ������������ߵ�ƽʱ��,_tΪʱ�����,_hΪ�м���
	mKeepExBase_t = GetPrivateProfileIntA(str,"KeepTmExUnit",5,strPath);
	mKeepExBase_h = GetPrivateProfileIntA(str,"KeepHcExUnit",5,strPath);
	((CPci6356_ioCtrl*)m_pData)->Pci6356_SetKeepExUnit(mKeepExBase_t,mKeepExBase_h);
	//��S500��ͬ�������ж���ѡ��ʱ���������ѡ��ʽ��

	//���ñ���������
	mCodehNum[0] = GetPrivateProfileIntA(str,"InCodeh0",0,strPath);
	mCodehNum[1] = GetPrivateProfileIntA(str,"InCodeh1",0,strPath);
	mCodehNum[2] = GetPrivateProfileIntA(str,"InCodeh2",0,strPath);
	mCodehNum[3] = GetPrivateProfileIntA(str,"InCodeh3",0,strPath);
	SetCodeFramelg_LX45();
	//���ñ������˲�ϵ��
	int CodeFilter = GetPrivateProfileIntA(str,"CodeFilter",85,strPath);//'b01010101=0x55=85,�����˲�
	IOWrite(0x68,(BYTE)CodeFilter);	//���ü�������˿�
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
	pCard->m_BoardType = 0;//���ð����� S500
	//���û�������
	sprintf_s(str,"��������");
	//�������ӳٲ���,����ֵ�ӳ����ʱ��
		//�������ӳٲ���
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
		if(pCard->m_BoardType != 1) {//????������?20190425
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
	sprintf_s(str,"����ģ��");
	//GetPrivateProfileIntA(str,"BoardType",0,strPath);
	mWork_Mode = GetPrivateProfileIntA(str,"WorkMode",0,strPath);
	((CPci6356_ioCtrl*)m_pData)->WorkMode = mWork_Mode;

	memset(strbd,0,32);
	GetPrivateProfileStringA(str,"BoardType","RunType1",strbd,30,strPath);
	QString sss;
	sss = strbd;
	if(sss=="PCI6356_LX45")	{//LX45���ӵĲ���
		ReadParameterFile_NewLX45(strPath);
	}
	else if(sss!="PCI6356_S500")	{
		return FALSE;
	}
	else {
		ReadParameterFile_OLDS500(strPath);//ͬʱ�����˶��еĿ��������ʽ
	}
	pCard->mEnInIntFlag = mEnInIntFlag;
	pCard->mInIntDelayms = mInIntDelayms;
	if(m_IfPcie32) RegWrite32(0x210,mInIntDelayms);
	else IOWrite(0x67,(UCHAR)(mInIntDelayms));
	if((mWork_Mode==0)||(mWork_Mode==101)) {//����2usʱ��������ͱ���������
		for (i=0;i<4;i++)
		{
			sprintf_s(str,"����%d",i);
			pCard->m_LineCtrlParam[i].m_LineNo = i;
			pCard->m_LineCtrlParam[i].m_Used = GetPrivateProfileIntA(str,"nUsed",0,strPath);//���ÿ���
			pCard->m_LineCtrlParam[i].m_InChn = GetPrivateProfileIntA(str,"nInPort",0,strPath);//��Ӧ����˿�
			pCard->m_LineCtrlParam[i].m_InFilterType = GetPrivateProfileIntA(str,"nInFilter",0,strPath);//��Ӧ����˿�
			pCard->m_LineCtrlParam[i].m_InMaxRate = GetPrivateProfileIntA(str,"nMaxRate",10,strPath);//��Ӧ����˿�
//			pCard->m_LineCtrlParam[i].m_OutCtrlType = GetPrivateProfileIntA(str,"nOutCtrlType",0,strPath);//����������������ʽ������/ͳһ����ȫ������
			pCard->m_LineCtrlParam[i].m_OutNum = GetPrivateProfileIntA(str,"nOutPortNum",1,strPath);//������ƶ˿ڸ���
			pCard->m_LineCtrlParam[i].m_TmType = GetPrivateProfileIntA(str,"nRunType",0,strPath);//�����ƽ���ʽ��LX45-0x18ִ�У���Ӱ��S500
			pCard->m_LineCtrlParam[i].m_LastOutPort = GetPrivateProfileIntA(str,"nLastOutPort",-1,strPath);//���һ�����ƶ˿�
			pCard->m_LineCtrlParam[i].m_LineCntDelay = GetPrivateProfileIntA(str,"nLineCntDelay",20,strPath);//�����ӳ�
			int cchn;

			for (j=0;j<pCard->m_LineCtrlParam[i].m_OutNum;j++)
			{
				sprintf_s(str1,"nOutPort%d",j);
				pCard->m_LineCtrlParam[i].m_OutChn[j] = GetPrivateProfileIntA(str,str1,0,strPath);//����˿�
				cchn = pCard->m_LineCtrlParam[i].m_OutChn[j];
				pCard->m_OutportParam[cchn].m_outChn = cchn;
				sprintf_s(str1,"nDelay%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnDelay[j] = GetPrivateProfileIntA(str,str1,0,strPath);//�ӳ�
				sprintf_s(str1,"nDelayIn%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j] = GetPrivateProfileIntA(str,str1,0,strPath);//�ӳ������Ʒ����

				sprintf_s(str1,"nDelayTm%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnDelayTM[j] = GetPrivateProfileIntA(str,str1,0,strPath);//�ӳٳ��ȣ���Ʒ��λ��0-1023��
				sprintf_s(str1,"nKeep%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnKeep[j] = GetPrivateProfileIntA(str,str1,0,strPath);//�������ʱ�䳤��
				sprintf_s(str1,"nType%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnType[j] = GetPrivateProfileIntA(str,str1,0,strPath);//����
				pCard->m_OutportParam[cchn].m_OutChnType = pCard->m_LineCtrlParam[i].m_OutChnType[j];
				pCard->m_OutportParam[cchn].m_NoUsed = 1;
				sprintf_s(str1,"nOutCntDelay%d",j);
				pCard->m_LineCtrlParam[i].m_OutChnCntDelay[j] = GetPrivateProfileIntA(str,str1,20,strPath);//�����ӳ�

				pCard->m_OutportParam[cchn].m_OutChnType = pCard->m_LineCtrlParam[i].m_OutChnType[j];
				pCard->m_OutportParam[cchn].m_NoUsed = 0;//ʹ�ø����

				if(pCard->m_LineCtrlParam[i].m_TmType==0) {
					pCard->m_LineCtrlParam[i].m_OutChnDelay[j]*=500;
					pCard->m_LineCtrlParam[i].m_OutChnKeep[j]*=500;
				}

				if(pCard->m_LineCtrlParam[i].m_TmType==9) {
					pCard->m_LineCtrlParam[i].m_OutChnKeep[j]*=500;
					//���͵�C51ʱ,����һ����
					pCard->m_LineCtrlParam[i].m_OutChnDelay[j] = pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j];
				}

				if( (pCard->m_LineCtrlParam[i].m_OutChnType[j] & 0x80)==0x80) {//�����Ҫ����
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
				/*LX45����˿�0-3��Ҫ����Ǿ������崮
				*/
			}
		}
	}
	else if( mWork_Mode==2 ) {//���������ź�(��Ʒ)��������2us��������
		ReadWorkMode2Param(strPath);
	}
	if (b_Pause)
	{
		for (i = 0; i < 4; i++)
		{
			pCard->m_LineCtrlParam[i].m_Used = 0;
		}
	}
	((CPci6356_ioCtrl*)m_pData)->InitLine();//��ͨ��m_BoardType=LX45�ڲ����ְ忨��ͬ
	//ContinueInCntWork(-1);
	return TRUE;
//	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_ReadParameterFile(strPath);
}

BOOL CioControl::ReadWorkMode2Param(const char* strPath)
{
	char str[16],str1[32];
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	sprintf_s(str,"��������");
	int i,j;
	//�������ӳٲ���
	mCntVOutDelay = GetPrivateProfileIntA(str,"CntVOutDelay",20,strPath);
	((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(mCntVOutDelay);
	for (i=0;i<4;i++)
	{
		sprintf_s(str,"����%d",i);
		pCard->m_LineCtrlParam[i].m_LineNo = i;
		pCard->m_LineCtrlParam[i].m_Used = GetPrivateProfileIntA(str,"nUsed",0,strPath);//���ÿ���
		pCard->m_LineCtrlParam[i].m_InChn = GetPrivateProfileIntA(str,"nInPort",0,strPath);//��Ӧ����˿�
		pCard->m_LineCtrlParam[i].m_InFilterType = GetPrivateProfileIntA(str,"nInFilter",0,strPath);//��Ӧ����˿�
		pCard->m_LineCtrlParam[i].m_InMaxRate = GetPrivateProfileIntA(str,"nMaxRate",10,strPath);//��Ӧ����˿�
//		pCard->m_LineCtrlParam[i].m_OutCtrlType = GPci6356_DL_etPrivateProfileInt(str,"nOutCtrlType",0,strPath);//����������������ʽ������/ͳһ
		pCard->m_LineCtrlParam[i].m_OutNum = GetPrivateProfileIntA(str,"nOutPortNum",1,strPath);//��������ƶ˿�
		pCard->m_LineCtrlParam[i].m_TmType = GetPrivateProfileIntA(str,"nRunType",0,strPath);//�����ƽ���ʽ
		pCard->m_LineCtrlParam[i].m_LastOutPort = GetPrivateProfileIntA(str,"nLastOutPort",-1,strPath);//���һ�����ƶ˿� 
		pCard->m_LineCtrlParam[i].m_LineCntDelay = GetPrivateProfileIntA(str,"nLineCntDelay",20,strPath);//�����ӳ�
		int cchn;

		for (j=0;j<pCard->m_LineCtrlParam[i].m_OutNum;j++)
		{
			sprintf_s(str1,"nOutPort%d",j);
			pCard->m_LineCtrlParam[i].m_OutChn[j] = GetPrivateProfileIntA(str,str1,0,strPath);//����˿�
			cchn = pCard->m_LineCtrlParam[i].m_OutChn[j];
			sprintf_s(str1,"nDelayIn%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j] = GetPrivateProfileIntA(str,str1,0,strPath);//�ӳ������Ʒ����
			pCard->m_LineCtrlParam[i].m_OutChnDelay[j] = pCard->m_LineCtrlParam[i].m_OutChnDelayIN[j];
			sprintf_s(str1,"nDelayTm%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnDelayTM[j] = GetPrivateProfileIntA(str,str1,0,strPath);//�ӳٳ��ȣ���Ʒ��λ��0-1023��
			sprintf_s(str1,"nKeep%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnKeep[j] = GetPrivateProfileIntA(str,str1,0,strPath);//����
			sprintf_s(str1,"nType%d",j);
			pCard->m_LineCtrlParam[i].m_OutChnType[j] = GetPrivateProfileIntA(str,str1,0,strPath);//����
			pCard->m_LineCtrlParam[i].m_OutChnKeep[j]*=500;
			sprintf_s(str1,"nOutCntDelay%d",j);

			pCard->m_OutportParam[cchn].m_OutChnType = pCard->m_LineCtrlParam[i].m_OutChnType[j];
			pCard->m_OutportParam[cchn].m_NoUsed = 0;
			
			pCard->m_LineCtrlParam[i].m_OutChnCntDelay[j] = GetPrivateProfileIntA(str,str1,20,strPath);//�����ӳ�
			if( (pCard->m_LineCtrlParam[i].m_OutChnType[j] & 0x80)==0x80) {//�����Ҫ����
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
			/*LX45����˿�0-3��Ҫ����Ǿ������崮
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
		sprintf_s(str,"����%d",i);
		sprintf_s(str1,"%d		//����%d���ÿ���",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_Used,i);
		WritePrivateProfileString(str,"nUsed",str1,strPath);//���ÿ���
		sprintf_s(str1,"%d		//����˿�",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_InChn);
		WritePrivateProfileString(str,"nInPort",str1,strPath);//����˿�
		sprintf_s(str1,"%d		//����˿ڸ���",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutNum);
		WritePrivateProfileString(str,"nOutPortNum",str1,strPath);//����ڸ���
		sprintf_s(str1,"%d		//�����ƽ���ʽ",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_TmType);
		WritePrivateProfileString(str,"nRunType",str1,strPath);//�����ƽ���ʽ
		for (int j=0;j<((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutNum;j++)
		{
			sprintf_s(str1,"nOutPort%d",j);
			sprintf_s(str2,"%d		//����%d,��%d��������ö˿�",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChn[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
			sprintf_s(str1,"nDelay%d",j);
			sprintf_s(str2,"%d		//����%d,��%d������ӳ�",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChnDelayTM[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
			sprintf_s(str1,"nKeep%d",j);
			sprintf_s(str2,"%d		//����%d,��%d���������",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChnKeep[j],i,j);
			WritePrivateProfileString(str,str1,str2,strPath);
			sprintf_s(str1,"nType%d",j);
			sprintf_s(str2,"%d		//����%d,��%d���������",((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[i].m_OutChnType[j],i,j);
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
	//��λ������
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ResetAllCount();
	return TRUE;
}

BOOL CioControl::ResetALL()
{
	//��λ������
	((CPci6356_ioCtrl*)m_pData)->Pci6356_ResetAllCount();
	//��C51����ʼ������(ע�⣬���ǳ�ʼ����������)
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
		//�趨�漰���п��Ƶ�����˿ڿ���ΪPC�ɿأ�ȫ���趨Ϊ��Ч��ƽ
		
		//֪ͨC51ֹͣ����������ƣ�����ʼ��C51�����ͼ�����
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
			IOWrite(0x1b,(BYTE)(nPortIn));//ѡ��˿�
			IOWrite(0x1c,0);//��ֹ�������
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
			IOWrite(0x1b,(BYTE)(nPortIn));//ѡ��˿�
			IOWrite(0x1c,1);//�����������
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
				//�������б仯����
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
				if( (C51Message[0]==(char)0xcc)&&(C51Message[1]==(char)0xcc) ) //C51��ʼ�����
				{
					//��C51��ʼ�����
					pdlg->C51InitFin = TRUE;
					pdlg->initC51Flag = TRUE;
					if(pdlg->OP_Start==TRUE) {
						pdlg->OP_Start=FALSE;
						//�������ִֻ��C51��ʼ������������
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
						//ָ��һ���洢�ϴ���Ϣ�Ķ���,�������������͵�ǰC51���������
						//	MsgLineNum = 0;
						//pMsgLine = new C51MSG_FROMDRV[128];
						memcpy(&(pdlg->pMsgLine[pdlg->MsgLineNum]),C51Message,24);
						PostMessage((HWND)pdlg->m_hWnd,10004,*(int*)(C51Message),(int)pdlg->MsgLineNum);
						pdlg->MsgLineNum++;
						if(pdlg->MsgLineNum>=128) pdlg->MsgLineNum = 0;
						}
//					PostMessage(pdlg->m_hWnd,10002,*(int*)(C51Message),*(int*)(C51Message+4));
/*					if( C51Message[0]==(char)0xc1 ) {//����仯
						nli = (int)C51Message[1];
						ind = (int)C51Message[2];
						//�������״̬������ʼ��
						pdlg->pLineCtrlStatus[nli].mOutCtrlStatus[ind].out_Ctrl = 0;
						//pdlg->pLineCtrlStatus[C51Message[1]].mOutCtrlStatus[C51Message[2]].out_Ctrl = 0;	//����SeResult/ex�޸�״̬
						//pLineCtrlStatus[C51Message[1]].mOutCtrlStatus[C51Message[2]].in_Status = 0;	
					}*/
				}
/*				else if( C51Message[0]==(char)0xc0 ) //����仯
				{
					if (pdlg->m_hWnd)
						PostMessage(pdlg->m_hWnd,20001,C51Message[1],C51Message[2]);//��ǰ���к�������������
				}
				else if( C51Message[0]==(char)0xc1 ) //����仯
				{
					if (pdlg->m_hWnd)
						PostMessage(pdlg->m_hWnd,20002,C51Message[1],C51Message[2]);//��ǰ���к������������
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
	//�ر��������
// 	FPGA_OUTPUT_PARAM	iniddd;
// 	memset(&iniddd,0,sizeof(iniddd));
// 	for(i=0;i<24;i++) {
// 		iniddd.m_NoUsed = 1;
// 		iniddd.m_outChn = i;
// 		Pci6356_InitOutputCh(&iniddd);
// 	}
	//2018 ��ʼ�� �����������ΪPC���ƣ������ǹر����������
	BYTE aaa;
	if(m_If32) {
		for(i=0;i<24;i++) {
			Pci6356_REGWrite32(0x0600+i*4,0);
		}
	}
	else {
		for(i=0;i<24;i++) {
			aaa = (BYTE)(i&0x1f);
			Pci6356_IOWrite(0x05,aaa);//ѡ��ͨ��
			Pci6356_IOWrite(0x13,0);//PC����
		}
	}

	//��ʼ�����в���
	for(i=0;i<4;i++) {
		Pci6356_InitFpgaLine(&m_LineCtrlParam[i]);
		if(m_LineCtrlParam[i].m_Used==1) {//���������Ч���øö�����Чλ����=1
			type |= 1<<i;
		}
	}

	
	//�ó�ʼ��δ���
	C51InitFin = FALSE;
	//��ʼ������״̬���ر����ж���
	//Pci6356_InitWork(type,3,0);//�������ô������жϣ���3���������������ļ���EnInputINT����
	Pci6356_InitWork(type,7,mEnInIntFlag);//�������ô������жϣ���3���������������ļ���EnInputINT����,�ڶ��������������ж�,�������Ź�
	//��ͣ�����������
	EnAllInCntWork(FALSE);
	//��λ
	Pci6356_ResetAllCount();
	ResetEvent(m_InputEvent);
	ResetEvent(m_C51Event);
	ResetEvent(m_500usEvent);


	//��ʼ��C51ͨ��,ֻ����C51�жϣ��ر�����ʹ��
	//���յ�C51��ʼ����ɺ󣬹ر����м������ٴ�����ʹ��
	Pci6356_InitC51Comm();

	Start();

	//��C51��ʼ������
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//������ʼ������
	paramData.CLength = 48;//��������48�ֽ�
	paramData.CParam = 0xf0f0f0f0;//�������Ͷ��г�ʼ��

	int m,n,occl,addst;
	if(WorkMode>=100) {
		//�����C51���ƶ������������ģʽ������������д��FPGA����������
		SetFPGA_C51Param_BYTE(0,WorkMode);
		m = 0;
		n = 0;
		occl = 0;
		for(i=0;i<4;i++) {//ȱ�������˿ں�
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
				SetFPGA_C51Param_BYTE(8+i,7);//�ɲ���
				SetFPGA_C51Param_BYTE(16+i,23);//�ɲ���
			}
		}
		SetFPGA_C51Param_BYTE(2,m);
		SetFPGA_C51Param_BYTE(3,n);

		paramData.CType = 0x5555;//������ʼ������
		paramData.CLength = 8;//��������48�ֽ�
		paramData.CParam = 0xf0f0f0f0;//�������Ͷ��г�ʼ��
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
			initlinedata[40+i] = m_LineCtrlParam[i].m_TmType;//ʱ���߼�
//			initlinedata[44+i] = m_LineCtrlParam[i].m_OutCtrlType;//����/ͳһ
			if( (initlinedata[40+i]>4) || (initlinedata[40+i]<0) ) 
				initlinedata[40+i] = 0;//ʹ��2us����
			//�õ����һ��ִ������˿�
			//////////////////////////////////////
			//2017-10-25,ʹ�������ļ��õ����˿ڣ�Ϊ��������İ汾��û��lastoutport������Զ��õ�������ʹ�������ļ��趨
			if(m_LineCtrlParam[i].m_LastOutPort==-1) {
				//��������汾
				lastlg = m_LineCtrlParam[i].m_OutChnDelay[0];// + m_LineCtrlParam[0].m_OutChnKeep[0];2017-08-25�޸Ĵ���m_LineCtrlParam[0]Ϊm_LineCtrlParam[i]
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
				//ʹ��������
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
			initlinedata[i*3] = 0;//����
			initlinedata[i*3+1] = 7;//�����һ���ݴ�
			initlinedata[i*3+2] = 23;//�����һ���ݴ�
			initlinedata[12+i] = 0;//û�����
			initlinedata[40+i] = 0;//ʹ��2us����
		}
	}
	memcpy(paramData.DataBuff,initlinedata,48);//�������Ͷ��г�ʼ��
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
	Sleep(10);//ȷ����һ����Ϣ���������
	Pci6356_SendCmdToC51(&paramCmd);
	if(mProcC51flag==TRUE) {
		//���C51��Ϣ�����߳�������������Ϣ�����߳�
		initC51Flag = FALSE;
		return TRUE;
	}
	else {//û��C51��Ϣ�����̣߳��ڴ˴���
		Sleep(100);//C51��λ�ӳ�
		if(WaitForSingleObject(m_C51Event,1000)==WAIT_OBJECT_0){
			ResetEvent(m_C51Event);
			char C51Message[8];
			int remainmsg = 0;
			while(Pci6356_ReadFromC51(C51Message,remainmsg)) {
				if( (C51Message[0]==(char)0xcc)&&(C51Message[1]==(char)0xcc) ) //C51��ʼ�����
				{
					//��C51��ʼ�����
					C51InitFin = TRUE;
					initC51Flag = TRUE;
					if(OP_Start==TRUE) {
						OP_Start=FALSE;
						//�������ִֻ��C51��ʼ������������
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

//���忨��
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

int  CioControl::GetHardVer(char *HardVer)//���忨����Ĺ̼��汾��Ϣ
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
int  CioControl::GetSoftVer(char *SoftVer)//���忨���������汾��Ϣ
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

//���忨�����ϵͳ�汾��Ϣ,�1000
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
	IOWrite(0x1d,2);//��ַ2Ϊ����
	IOWrite(0x1e,0);
	IORead(0x07,&ddd);
	IOWrite(0x1d,3);//��ַ2Ϊ����
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

//д�忨��
BOOL  CioControl::WriteBoardNo(int nbd)
{
	SENDTOC51CMD paramCmd;
	paramCmd.lg = 2;
	paramCmd.CType = 0xaaaa;
	paramCmd.Command[0] = 0x33;
	paramCmd.Command[1] = (BYTE)nbd;
	paramCmd.Command[2] = (BYTE)nbd;
	Sleep(10);//ȷ����һ����Ϣ���������
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendCmdToC51(&paramCmd);
	
}

//д�忨�����ϵͳ�汾��Ϣ
BOOL  CioControl::WriteBoardVer(char *sysVer)
{
	//����0xfd
	int i,lg;
	lg = strlen(sysVer);
	if(lg>1000) lg = 1000;
	SENDTOC51DATA paramData;
	paramData.CType = 0x5555;//��������
	paramData.CLength = lg+2;//���ݳ���
	paramData.CParam = 0xfdfdfdfd;//�汾д������
	paramData.DataBuff[0] = (BYTE)lg;
	paramData.DataBuff[1] = (BYTE)(lg>>8);
	for(i=0;i<lg;i++) {
		paramData.DataBuff[i+2] = sysVer[i];
	}
	Sleep(10);//ȷ����һ����Ϣ���������
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SendDataToC51(&paramData);
}

//////////////////////////////////////////////////////////////
//2017-12-15��������������������ƽӿں���������ʹ��

BOOL CioControl::InitInputCh(int chn,int Filter,float maxRate)//Filter:��Ե�˲�����,maxRate:����ϲ�����
{

	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_InitInputCh(chn,Filter,maxRate);

}

//�����������:�ӳٲ����뱣�ֲ���,ע�����ģʽ��ͬ,�����������\ʱ���׼����\����������
BOOL CioControl::InitOutputCh(PFPGA_OUTPUT_PARAM paramCh)
{
	FPGA_OUTPUT_PARAM *poldParam;
	FPGA_INPUT_LINE *pLin;
	int chn;
	chn = paramCh->m_outChn;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	//�������д��󣬷���
	if(poldParam->m_RelLine!=paramCh->m_RelLine) return FALSE;
	//���ʹ��2us��������Ҫ��ʱ�����*500�γɺ������ֵ��˵������д��
	memcpy(poldParam,paramCh,sizeof(FPGA_OUTPUT_PARAM));
	//�õ���������ʱ���������
	int lin = poldParam->m_RelLine;
	pLin = &(((CPci6356_ioCtrl*)m_pData)->m_LineCtrlParam[lin]);
	if(pLin->m_TmType==0) {
		poldParam->m_OutChnDelay *= 500;
		poldParam->m_OutChnKeep *= 500;
	}
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_InitOutputCh(poldParam);
}

//���޸�ԭ���ò������ԣ�ֱ�ӿ��ư忨�������������һ���˿�ʹ�ܣ���ҪC51���
BOOL CioControl::SetOutputType(int chn,BOOL ifAuto,BOOL ifKick)//�Ƿ��Զ�/���ƣ�����ǿ��ƶ˿��Ƿ���Ҫ����
{
	FPGA_OUTPUT_PARAM dParam;
	FPGA_OUTPUT_PARAM *poldParam;
	poldParam = &(((CPci6356_ioCtrl*)m_pData)->m_OutportParam[chn]);
	memcpy(&dParam,poldParam,sizeof(FPGA_OUTPUT_PARAM));
	DWORD ddd = dParam.m_OutChnType;
	if(ifAuto) {
		ddd |= 0x01;//ȷ�϶��й���
		ddd &= 0x0fffd;//xxx01�����ܿض˿�
	}
	else {
		ddd |= 0x03;//ȷ�϶��й������ܿض˿�
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
		//ѡ��˿ں�
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

//���ú�ˮģʽ��ʱ��/�����������ӳٿ��ƣ�
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

//��������ģʽ����������ӳٿ��ƣ�
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

//���޸�ԭ���ò������ԣ�ֱ�ӿ��ư忨
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
		//ѡ��˿ں�
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

//���������ӳټ������ӳ����ʱ��
BOOL CioControl::SetDelayCntParam(int nDelay)
{
	return ((CPci6356_ioCtrl*)m_pData)->Pci6356_SetDelayCntParam(nDelay);	
}

/*
OutType: bit0=0/1 -> InPluse(��������)/exLine(�����ӳ����)
OutType: bit1=0/1 -> exLine������Ƶ��������������/2us��ʱ��
���A/B/Z�ļ���������ѡ��0-3 Coder
���C�ļ�������˿ڣ�0-7 coein
������������ӳ�������delayIn
�������������ʱ���ӳټ���delayTm����OutType��bit1ѡ��ʱ��Դ��
������屣�ֿ��keep����OutType��bit1ѡ��ʱ��Դ��
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
		if((OutType&0x02)==0x02) {//������
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
	//��ʼ����
	dval = 0x04 | (OutType&0x03);
	RegWrite32(0x230,dval);
	
	return TRUE;
}

//������Ź�:ϵͳ���Ź����û�������Ź�
//Alarmperiod �� ��5��ı��������
BOOL	CioControl::SetSoftAlerm(BOOL usersoft,BOOL sysdriver,int Alarmperiod)
{
	DWORD dval=0;
	if(usersoft) dval |= 0x02;
	if(sysdriver) dval |= 0x01;
	if(m_IfPcie32) {
		IOWrite32(0x084,1);//��λϵͳ�澯
		IOWrite32(0x088,1);//��λ����澯
		IOWrite32(0x010,dval);
		if(Alarmperiod>=5)
			RegWrite32(0x110,Alarmperiod/5);//����ת��Ϊ5��
		return TRUE;
	}
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	//20180325
	if(pCard->m_BoardType == 1){//������ LX45 ������ALARM
		IOWrite(0x03a,1);//��λϵͳ�澯
		IOWrite(0x03b,1);//��λ����澯
		IOWrite(0x040,(BYTE)(dval));//��λ����澯
		if(Alarmperiod>=5)
			IOWrite(0x41,(BYTE)(Alarmperiod/5));
		return TRUE;
	}
	return FALSE;
}

BOOL	CioControl::ReSetSoftAlerm()//ι������
{
	if(m_IfPcie32) {
		IOWrite32(0x084,1);//��λϵͳ�澯
		return IOWrite32(0x088,1);//��λ����澯
	}
	//20180325
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	if(pCard->m_BoardType == 1){//������ LX45 ������ALARM
		IOWrite(0x03a,1);//��λϵͳ�澯
		return IOWrite(0x03b,1);//��λ����澯
	}
	else return FALSE;
}

BOOL	CioControl::SetAlermCoeOut(int Acoeout,int Alevel)
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	BYTE aaa;
	if(pCard->m_BoardType == 1){//������ LX45 ������ALARM
		aaa = (BYTE)Acoeout;
		IOWrite(0x042,aaa);//���ù�������ͨ��
		aaa = (BYTE)(Alevel&0x01);
		return IOWrite(0x043,aaa);//���������ƽ
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
	if(pCard->m_BoardType == 1){//������ LX45 ������ALARM
		aaa = (BYTE)(MaxWith>>8);
		IOWrite(0x045,aaa&0x1f);//���ø�5λ
		aaa = (BYTE)(MaxWith&0xff);
		return IOWrite(0x044,aaa);//���õ�8λ
	}
	else return FALSE;

}


//////////////////////�ض�������Ʋ���
//�ض�������Ʋ������ض���ֵ�趨
BOOL		CioControl::ReadOutPpreCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam )
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadOutPpreCtrl(port,OutCtrlParam));
}

//�ض�������Ʋ������ض����߿������,����ʱ�ɼ�����˿ڿ��Ʋ������ֵ
BOOL		CioControl::ReadOutPinlCtrl(int port,PCI6356_OUTPORT_REG &OutCtrlParam )
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadOutPinlCtrl(port,OutCtrlParam));
}


//�ض����п��ƺ�������������FPGA�������п��Ʋ�������������״̬
BOOL		CioControl::ReadLineIndex(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam,DWORD  *sqIndexOpStatu,PUCHAR sqExKeep)
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadLineIndex(nline,inLineParam,sqIndexOpStatu,sqExKeep));
}


//�ض����п��Ʋ�������FPGA�������п��Ʋ���������������
BOOL		CioControl::ReadLineCtrl(int nline,PCI6356_LINE_INDEXSTATUS &inLineParam )
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadLineCtrl(nline,inLineParam));
}


//�ض���ʷ��Ϣ����
BOOL		CioControl::ReadMsgSQ(PUCHAR pmsg,int num)
{
	CPci6356_ioCtrl *pCard = ((CPci6356_ioCtrl*)m_pData);
	return (pCard->ReadMsgSQ(pmsg,num));
}

