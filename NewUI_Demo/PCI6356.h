#ifndef _AD_PCI6356_API_SYS_INCLUDE_DLL
#define _AD_PCI6356_API_SYS_INCLUDE_DLL


#define		DWORD	ULONG
#define		WORD	unsigned _int16


#define		MAX_RELAX_OUT	16
typedef struct  _PCI6356_TIMEREG
{
	DWORD	m_timeCnt10us;
	DWORD	m_HCntAll1;
	DWORD	m_HCntAll2;
	int		m_HCntRem1;
	int		m_HCntRem2;
	int		m_Fcnt1;
	int		m_Fcnt2;
}PCI6356_TIMEREG,*PPCI6356_TIMEREG;

typedef struct  _PCI6356_TIMEREG_LX45
{
	DWORD	m_timeCnt10us;
	
	WORD	m_HCntRem0;
	WORD	m_Fcnt0;
	DWORD	m_HCntAll0;

	WORD	m_HCntRem1;
	WORD	m_Fcnt1;
	DWORD	m_HCntAll1;

	WORD	m_HCntRem2;
	WORD	m_Fcnt2;
	DWORD	m_HCntAll2;

	WORD	m_HCntRem3;
	WORD	m_Fcnt3;
	DWORD	m_HCntAll3;
}PCI6356_TIMEREG_LX45,*PPCI6356_TIMEREG_LX45;




/*
typedef	struct _FPGA_OUTPUT_PARAM 
{
	int	m_outChn;
	int	m_NoUsed;//�Ƿ�ʹ��=1��ֹ,=0ʹ��
	int m_OutChnType;//������Ʒ�ʽ,����bit7=1���Ϊ���������������Ϊ������������������в���FPGA_OUTPUT_PARAM_SQ
	int m_RelLine;//�����������,=-1����ʹ��
	int m_OutChnDelay;//���ͨ����Ӧ�����ӳ�ʱ�䣨�й���������Ч��
	int m_OutChnKeep;//���ͨ���������ʱ�䣨�й���������Ч��
	int m_OutChnDelayIN;//���ͨ����Ӧ�����ӳ�ʱ�䣨�й���������Ч��
	int m_OutChnDelayTM;//���ͨ����Ӧ�����ӳ�ʱ�䣨�й���������Ч��
	int m_HighTm;//1-255����ߵ�ƽ����ʱ�䣬�����
	int m_LowTm;//1-255����͵�ƽ����ʱ�䣬�����
	int m_ReCycleNum;//1-255���������������
}FPGA_OUTPUT_PARAM,*PFPGA_OUTPUT_PARAM;
*/
typedef	struct _FPGA_INPUT_LINE 
{
	int	m_Used;//�Ƿ�ʹ��
	int	m_LineNo;//FPGA�������
	int	m_TmType;//ʱ��ʹ�÷�ʽ
	int m_InChn;//���봥��ͨ��
	int m_InMaxRate;//�����������
	int m_LineCntDelay;//��������ӳ����ʱ��:	����
	int m_InFilterType;//�����˲�����
	int m_OutNum;//�������ͨ����
	int m_LastOutPort;//���һ������˿�
	int m_OutChn[MAX_RELAX_OUT];//��Ӧ��������ͨ���ţ������˳������
	int m_OutChnDelay[MAX_RELAX_OUT];//ÿ�����ͨ����Ӧ�����ӳ�ʱ��
	int m_OutChnDelayIN[MAX_RELAX_OUT];//ÿ�����ͨ����Ӧ�ӳ��������
	int m_OutChnDelayTM[MAX_RELAX_OUT];//ÿ����������ض�Ӧ�����������ӳ�ʱ��(mode0ʱ��2us����,mode2���ڵ�λ��0-1023)
	int m_OutChnKeep[MAX_RELAX_OUT];//ÿ�����ͨ���������ʱ��
	int m_OutChnType[MAX_RELAX_OUT];//������Ʒ�ʽ
	int m_OutChnCntDelay[MAX_RELAX_OUT];//��������ӳ����ʱ��:	����
	int m_OutHighTm[MAX_RELAX_OUT];//1-255����ߵ�ƽ����ʱ�䣬�����
	int m_OutLowTm[MAX_RELAX_OUT];//1-255����͵�ƽ����ʱ�䣬�����
	int m_OutReCycleNum[MAX_RELAX_OUT];//1-255���������������
}FPGA_INPUT_LINE,*PFPGA_INPUT_LINE;

typedef	struct _SENDTOC51DATA
{
	WORD	CType;//��������
	WORD	CLength;//���ݳ���0=1024
	DWORD	CParam;//˵���������Զ���
	UCHAR	DataBuff[1024];//����
}SENDTOC51DATA,*PSENDTOC51DATA;

typedef	struct _SENDTOC51CMD
{
	WORD	CType;//��������
	int		lg;
	BYTE	Command[14];//�������
}SENDTOC51CMD,*PSENDTOC51CMD;
class	CPci6356_Card
{

	BOOL	Pci6356_InitEvent();
	BOOL	Pci6356_Get32Flag();
	BOOL	Pci6356_GetMemPoint(PULONG *pmem);
	BOOL	Pci6356_InitOutputCh_LX45(PFPGA_OUTPUT_PARAM paramCh);
	BOOL	Pci6356_InitOutputCh_PCIE(PFPGA_OUTPUT_PARAM paramCh);
	BOOL	Pci6356_InitOutputCh_S500(PFPGA_OUTPUT_PARAM paramCh);
public:
	CPci6356_Card();
	~CPci6356_Card();
	int			mDev;
	int			m_BoardType;
	BOOL		m_If32;
	BOOL		m_EnC51CodeCnt;
	void*		m_hWnd;
	PULONG		p_PcieMem;

	HANDLE	mDevHandle;
	HANDLE	mInputEvent;
	HANDLE	mC51Event;
	HANDLE	m500UsEvent;
	
	int		WorkMode;
	
	FPGA_INPUT_LINE	m_LineCtrlParam[4];
	FPGA_OUTPUT_PARAM	m_OutportParam[24];

	BOOL		Pci6356_Open(int iDev, HWND iWnd);
	BOOL		Pci6356_Close();
	BOOL		Pci6356_IsOpen();
	BOOL		Pci6356_GetSysVer(char *pver);
	BOOL		Pci6356_GetLibVer(char *pver);
	BOOL		Pci6356_GetHardVer(char *pver);

	BOOL		Pci6356_EnGetC51MsgCodeCnt(BOOL EnCodeCnt);
	
	BOOL		Pci6356_SerVBOcxInit(void* mOcxHandle);
	BOOL		Pci6356_ReadTimeReg(PPCI6356_TIMEREG mallval);
	BOOL		Pci6356_ReadTimeReg_LX45(PPCI6356_TIMEREG_LX45 pout);
	
	BOOL		Pci6356_IOWrite(int addr,BYTE mval);
	BOOL		Pci6356_IORead(int addr,BYTE *pval);
	BOOL		Pci6356_IOWrite32(int addr,DWORD mval);
	BOOL		Pci6356_IORead32(int addr,DWORD *pval);
	BOOL		Pci6356_REGWrite32(int addr,DWORD mval);
	BOOL		Pci6356_REGRead32(int addr,DWORD *pval);
	
	//ͬʱʹ��CPLD��C51����
	BOOL		Pci6356_ReadTmParam(int type,PVOID tmParam);
	BOOL		Pci6356_ReadTmParam_LX45(int type,PVOID tmParam);

	BOOL		Pci6356_ReadInputIOParam(int chn,PUCHAR regParam);
	
	BOOL		Pci6356_ReadOutputIOParam(int chn,PUCHAR regParam);
	
	BOOL		Pci6356_ResetC51();
	
	BOOL		Pci6356_ReadInIOStatus(BYTE &inStatus);
	BOOL		Pci6356_SetOutIOStatus(int chn,BOOL status);
	
	BOOL		Pci6356_ReadInChanged(UCHAR &instatus);
	BOOL		Pci6356_ReadInChanged(UCHAR &instatus,UCHAR &indelaystatus,DWORD &time2uscnt);
	
	BOOL		Pci6356_ResetAllCount();
	
	BOOL		Pci6356_StartWork();
	BOOL		Pci6356_StopWork();
	BOOL		Pci6356_InitWork(int nEnLine, int nEnIntType,int nEnInputInt);
	BOOL		Pci6356_SetKeepExUnit(int tmdelay,int hcdelay);
	
	BOOL		Pci6356_InitFpgaLine(PFPGA_INPUT_LINE pLineparam);
	BOOL		Pci6356_InitInputCh(int chn,int FilterType,float maxRate);
	BOOL		Pci6356_InitOutputCh(PFPGA_OUTPUT_PARAM paramCh);
	BOOL		Pci6356_SendCmdToC51(PSENDTOC51CMD paramCh);
	BOOL		Pci6356_SendDataToC51(PSENDTOC51DATA paramCh);
	
	BOOL		Pci6356_ReadFromC51(char *pData,int &remain);
	BOOL		Pci6356_ReadC51DataReg(char *pData);
	
	
	BOOL		Pci6356_ReadInPCnt32_t(int chn ,DWORD &cntvalue);
	
	//�����ʱ�������ֵ����Pci6356_ReadOutputIOParam��ͬ
	BOOL		Pci6356_ReadOutPCnt32_t(int chn ,DWORD &cntvalue);
	//ʵ���������ֵ
	BOOL		Pci6356_ReadOutPCnt32_o(int chn ,DWORD &cntvalue);
	//���߹�����ʽ��,�Զ��������������
	BOOL		Pci6356_ReadOutPCnt32_a(int chn ,DWORD &cntvalue);

	BOOL		Pci6356_GetRateVal(int port,float &rate);


	//������ֵ�ӳ����
	//�ӳ�ʱ������
	BOOL		Pci6356_SetDelayCntParam(int delayms);
	BYTE		Pci6356_ReadInPDelay(int chn,int delay);
	BYTE		Pci6356_ReadOutPDelay(int chn,int delay);

	//���ñ�������������
	//type = 0 : ����
	//type = 1 : ˫��
	BOOL		Pci6356_SetCoderType(int type);
	BOOL		Pci6356_SetCodeParam(int code1ds,int code1hmax,int code2ds,int code2hmax);
	
	//����C51ͨ�ſ���
	BOOL		Pci6356_InitC51Comm();

	//����������п��Ʒ�ʽ
	BOOL		Pci6356_SetBaseType(int group1,int group2);


	
};

#endif