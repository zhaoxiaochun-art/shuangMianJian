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
	int	m_NoUsed;//是否使用=1禁止,=0使用
	int m_OutChnType;//输出控制方式,增加bit7=1输出为序列输出：如果输出为序列输出定义该输出序列参数FPGA_OUTPUT_PARAM_SQ
	int m_RelLine;//关联队列序号,=-1独立使用
	int m_OutChnDelay;//输出通道对应输入延迟时间（有关联队列有效）
	int m_OutChnKeep;//输出通道输出保持时间（有关联队列有效）
	int m_OutChnDelayIN;//输出通道对应输入延迟时间（有关联队列有效）
	int m_OutChnDelayTM;//输出通道对应输入延迟时间（有关联队列有效）
	int m_HighTm;//1-255脉冲高电平持续时间，先输出
	int m_LowTm;//1-255脉冲低电平持续时间，后输出
	int m_ReCycleNum;//1-255脉冲连续输出个数
}FPGA_OUTPUT_PARAM,*PFPGA_OUTPUT_PARAM;
*/
typedef	struct _FPGA_INPUT_LINE 
{
	int	m_Used;//是否使用
	int	m_LineNo;//FPGA序列序号
	int	m_TmType;//时基使用方式
	int m_InChn;//输入触发通道
	int m_InMaxRate;//输入最大速率
	int m_LineCntDelay;//输入计数延迟输出时间:	毫秒
	int m_InFilterType;//输入滤波参数
	int m_OutNum;//控制输出通道数
	int m_LastOutPort;//最后一个输出端口
	int m_OutChn[MAX_RELAX_OUT];//对应输控制输出通道号，按输出顺序排列
	int m_OutChnDelay[MAX_RELAX_OUT];//每个输出通道对应输入延迟时间
	int m_OutChnDelayIN[MAX_RELAX_OUT];//每个输出通道对应延迟输入个数
	int m_OutChnDelayTM[MAX_RELAX_OUT];//每个输出上升沿对应输入上升沿延迟时间(mode0时间2us计数,mode2周期的位置0-1023)
	int m_OutChnKeep[MAX_RELAX_OUT];//每个输出通道输出保持时间
	int m_OutChnType[MAX_RELAX_OUT];//输出控制方式
	int m_OutChnCntDelay[MAX_RELAX_OUT];//输出计数延迟输出时间:	毫秒
	int m_OutHighTm[MAX_RELAX_OUT];//1-255脉冲高电平持续时间，先输出
	int m_OutLowTm[MAX_RELAX_OUT];//1-255脉冲低电平持续时间，后输出
	int m_OutReCycleNum[MAX_RELAX_OUT];//1-255脉冲连续输出个数
}FPGA_INPUT_LINE,*PFPGA_INPUT_LINE;

typedef	struct _SENDTOC51DATA
{
	WORD	CType;//数据类型
	WORD	CLength;//数据长度0=1024
	DWORD	CParam;//说明参数，自定义
	UCHAR	DataBuff[1024];//数据
}SENDTOC51DATA,*PSENDTOC51DATA;

typedef	struct _SENDTOC51CMD
{
	WORD	CType;//命令类型
	int		lg;
	BYTE	Command[14];//命令及参数
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
	
	//同时使用CPLD和C51设置
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
	
	//输出定时到达计数值，与Pci6356_ReadOutputIOParam相同
	BOOL		Pci6356_ReadOutPCnt32_t(int chn ,DWORD &cntvalue);
	//实际输出计数值
	BOOL		Pci6356_ReadOutPCnt32_o(int chn ,DWORD &cntvalue);
	//补踢工作方式下,自动补踢输出计数器
	BOOL		Pci6356_ReadOutPCnt32_a(int chn ,DWORD &cntvalue);

	BOOL		Pci6356_GetRateVal(int port,float &rate);


	//计数器值延迟输出
	//延迟时间设置
	BOOL		Pci6356_SetDelayCntParam(int delayms);
	BYTE		Pci6356_ReadInPDelay(int chn,int delay);
	BYTE		Pci6356_ReadOutPDelay(int chn,int delay);

	//设置编码器输入类型
	//type = 0 : 单端
	//type = 1 : 双端
	BOOL		Pci6356_SetCoderType(int type);
	BOOL		Pci6356_SetCodeParam(int code1ds,int code1hmax,int code2ds,int code2hmax);
	
	//启动C51通信控制
	BOOL		Pci6356_InitC51Comm();

	//设置两组队列控制方式
	BOOL		Pci6356_SetBaseType(int group1,int group2);


	
};

#endif