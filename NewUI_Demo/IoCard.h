
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IOCARD_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IOCONTROL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#define  IOCARD_EXPORTS
#ifdef IOCARD_EXPORTS
#define IOCONTROL_API __declspec(dllexport)
#else
#define IOCONTROL_API __declspec(dllimport)
#endif

// This class is exported from the IoCard.dll

#ifndef INCLODE_IOCARD_6356
#define  INCLODE_IOCARD_6356

#define		BOARDTYPE_3S500		0
#define		BOARDTYPE_LX45		1

typedef	struct _CTRLOUT_RESULT {
	int		nline;
	int		nIndex;
	BYTE	mOP;
	bool	mNeedDelay;
	int		mDelay;//0-127
}CTRLOUT_RESULT,*PCTRLOUT_RESULT;


typedef	struct _CTRLOUT_RESULT_INDEP {
	int		nline;
	int		nIndex;
	int		nResult;
}CTRLOUT_RESULT_INDEP,*PCTRLOUT_RESULT_INDEP;

typedef struct  _CTRL_TIMEREG_LX45
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
}CTRL_TIMEREG_LX45,*PCTRL_TIMEREG_LX45;

typedef struct  _CTRL_TIMEREG
{
	DWORD	m_timeCnt10us;
	DWORD	m_HCntAll1;
	DWORD	m_HCntAll2;
	int		m_HCntRem1;
	int		m_HCntRem2;
	int		m_Fcnt1;
	int		m_Fcnt2;
}CTRL_TIMEREG,*PCTRL_TIMEREG;

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


#define		EN_FPGATIMEINT		0x02
#define		EN_INPUTINT			0x04
#define		EN_C51INT			0x01

typedef struct _C51MSG_FROMDRV {
	UCHAR	cmd[4];
	DWORD	Cnt2us;
	DWORD	Code0Cnt;
	DWORD	Code1Cnt;
	DWORD	Code2Cnt;
	DWORD	Code3Cnt;
	
}C51MSG_FROMDRV,*PC51MSG_FROMDRV;

typedef struct  _PCI6356_OUTPORT_REG_TM{
	DWORD	m_Delay;
	DWORD	m_Keep;
}PCI6356_OUTPORT_REG_TM,*PPCI6356_OUTPORT_REG_TM;

typedef struct  _PCI6356_OUTPORT_REG_SQ{
	WORD	m_MidDelay;//1/1024间隔，相对延迟
	UCHAR	m_IncDelay;//延迟输入（产品）个数
	UCHAR	m_NO;//无效
	DWORD	m_Keep;
}PCI6356_OUTPORT_REG_SQ,*PPCI6356_OUTPORT_REG_SQ;

typedef struct  _PCI6356_OUTPORT_REG{
	union {
		UCHAR	TMreg[8];
		PCI6356_OUTPORT_REG_TM	m_TmParam;
		PCI6356_OUTPORT_REG_SQ	m_SqParam;
	}TMCtrl;//时间控制参数
	UCHAR	m_OutChnType;//控制类型
	UCHAR	m_OutChnReLine;//关联队列
}PCI6356_OUTPORT_REG,*PPCI6356_OUTPORT_REG;

typedef struct  _PCI6356_LINE_INDEXSTATUS{
	UCHAR	mLine;//队列号
	UCHAR	mRelInp;//关联输入
	UCHAR	mRelInpCountIndex;//关联输入计数索引（当前输入）
	UCHAR	mRelOutNun;//关联输出控制索引（执行输出）
//	DWORD	mIndexOpStatu[256];//索引控制状态：输出控制
//	UCHAR	mIndexKeepExStatu[256];//索引控制状态：扩展参数
	UCHAR	mOutIndexStatu[24];//输出通道对应计数索引
}PCI6356_LINE_INDEXSTATUS,*PPCI6356_LINE_INDEXSTATUS;

class IOCONTROL_API CioControl
{
	BOOL  ReadWorkMode2Param(const char* strPath);
	BOOL  ReadParameterFile_NewLX45(const char* strPath);
	BOOL  ReadParameterFile_OLDS500(const char* strPath);
	void  SetCodeFramelg_LX45();
public:
	CioControl();
	virtual ~CioControl();
	void* m_pData;
	BOOL	m_IfPcie32;
	int		m_BoardType;
	PULONG	p_PciEMem;	
	int		mWork_Mode;
	int		mCntVOutDelay;
	int		mKeepExBase_t;
	int		mKeepExBase_h;
	int		mRunType1;
	int		mRunType2;
	int		mCode1dsType;
	int		mCode2dsType;
	int		mCode1hMax;
	int		mCode2hMax;
	int		mCodehNum[4];//LX45四个编码器的每帧行数
	DWORD   mInCoeOut;//输入与输出级联设置
	DWORD   mEnInIntFlag;//输入与输入延迟中断使能
	DWORD   mInIntDelayms;//输入延迟中断延迟时间

	//设备控制
	BOOL PCI_Open(int mDev,void* hWnd=NULL);
	BOOL SetMsgWnd(void* hWnd=NULL);
	BOOL Close();
	BOOL PCI_IsOpen();
	BOOL PCI_StartWork();
	BOOL PCI_StopWork();
	BOOL PCI_SetOutStatus(int, bool);
	int  PCI_ReadInStatus(int);
	int  ReadBoardNo();//读板卡号
	//设备与软件版本信息
	int  ReadBoardInfo(char *sysVer);//读板卡服务的硬件系统版本信息
	int  GetSoftVer(char *SoftVer);//读板卡服务的软件版本信息:DLL/LIB
	int  GetHardVer(char *HardVer);//读板卡服务的固件版本信息:FPGA/C51/驱动
	
	BOOL  WriteBoardNo(int nbd);//写板卡号
	BOOL  WriteBoardVer(char *sysVer);//写板卡服务的系统版本信息,推荐解析方法


	//队列输出控制方式 : 统一控制，每个队列的只有一个输出控制参数，不独立,主要用于一个队列同时控制多个控制输出（执行效率较高）
	BOOL SetResult(int nChannel,int nIndex,int nResult,int delay=0);//每次控制一个输入产品索引
	BOOL SetResultEx(int num,PCTRLOUT_RESULT pRes);//每次控制N个输入产品索引
	//队列输出控制方式 : 独立控制，每个队列的各个输出控制参数独立控制，主要用于一个队列独立控制多个控制输出（执行效率较低，C51更多的控制参数接收与解析）
	BOOL SetOutCtrlByResult(int nChannel,int nIndex,int lineoutpout,int nResult);//每次只控制一个输入产品索引的一个输出
	BOOL SetOutCtrlByResultEx(int nChannel,int nIndex,int nResult);//每次控制一个输入产品索引的M个输出
	BOOL SetOutCtrlByResultEx2(int num,PCTRLOUT_RESULT_INDEP pCtrl);//每次控制N个输入产品索引的N*M个输出
	
	//读队列索引\输入\输出值
	int ReadInputIndex(int nChannel);//读队列索引值8位
	int ReadInputCount(int inputn);//读输入计数值32位

	int ReadOutputIndex(int nChannel,int nPos);//按读队列配置顺序读输出索引值8位
	int ReadOutputCount(int nChannel,int nPos);//按读队列配置顺序读输出计数值32位
	int ReadOutputCtrlCnt(int nChannel,int nPos);//根据队列位置读读输出索引值32位
	int ReadOutputBTCnt(int outputn,int nPos);//按读队列配置顺序读输出补踢计数器
	int ReadOutputCount(int outputn);//按物理端口号直接读输出计数值32位
	int ReadOutputCtrlCnt(int outputn);//按物理端口号直接读输出索引值32位
	int ReadOutputBTCnt(int outputn);//按物理端口号直接读输出补踢计数器
	//读延迟队列索引\输入\输出值
	int ReadInputdelayIndex(int nChannel);//读队列延迟输入索引值8位
	int ReadOutputdelayIndex(int nChannel,int nPos);//按读队列配置顺序读延迟输出索引值8位

	int ReadInPortdelayCount(int ninPort,int nDelay);//按物理端口号直接读延迟输入计数值8位,同时设置延迟
	int ReadOutputdelayCount(int noutPort,int nDelay);//按物理端口号直接读延迟输出计数值8位,同时设置延迟


	BOOL PCI_Init(const char* strPath, bool b_Pause = false);//输入参数配置初始化状态,并启动工作
//	BOOL WriteParameterFile(const char* strPath);

	BOOL IOWrite(int addr, BYTE mval);
	BOOL IORead(int addr, BYTE *pval);
	BOOL IOWrite32(int addr, DWORD mval);
	BOOL IORead32(int addr, DWORD *pval);
	BOOL RegWrite32(int addr, DWORD mval);
	BOOL RegRead32(int addr, DWORD *pval);

	//扩展函数
	//得到输入电平状态
	BOOL ReadInPortStatus(BYTE &inPortReg);
	//收到中断事件,读取输入及其延迟变化状态和发生时间
	BOOL ReadInChanged(UCHAR &instatus,UCHAR &indelaystatus,DWORD &time2uscnt);

	//启动运行,needReset=TRUE需要复位，内部执行ResetALL()
	BOOL StartWork(BOOL needReset=TRUE);
	//停止运行,阻断所有输入操作和队列操作
	BOOL StopWork(BOOL cutALL=TRUE);
	//重新复位初始化,复位C51控制参数、FPGA队列状态、所有计数器
	BOOL ResetALL();
	//重新复位所有计数器
	BOOL ResetCount();
	//暂停输入控制计数	
	BOOL PauseInCntWork(int nPortIn);
	//继续运行输入计数
	BOOL ContinueInCntWork(int nPortIn);
	//读入当前时间参数,旧函数
	BOOL GetAllTmParam(PCTRL_TIMEREG nowTmParam);
	//读入当前时间参数,兼容旧函数,不建议使用
	BOOL GetAllTmParam(PCTRL_TIMEREG_LX45 nowTmParam);
	//读入时间/编码器计数
	BOOL PCI_GetTm2us(DWORD &nowTm2us);
	BOOL PCI_GetTmCoder(int coder_n,DWORD &nowhAll,DWORD &nowf,DWORD &nowh);
	BOOL PCI_GetTmCoder(int coder_n,DWORD &nowhAll);
	BOOL PCI_GetTmCoder(int coder_n,DWORD &nowf,DWORD &nowh);

	//读入输入/编码器速率
	BOOL GetRate(int port,float &rate);

	//2017-12-15新增参数设置与计数控制接口函数，谨慎使用
	//设置输入参数:滤波参数
	BOOL InitInputCh(int chn,int Filter,float maxRate);//Filter:边缘滤波参数,maxRate:脉冲合并参数

	//设置输出参数:延迟参数与保持参数,注意各种模式不同,输入计数控制\时间基准控制\编码器控制
	//由于最后一个端口配置需要通知C51，因而修改参数不能影响最后一个端口排序，否则影响输出队列控制，只能执行配置文件初始化达到
	BOOL InitOutputCh(PFPGA_OUTPUT_PARAM paramCh);
	BOOL SetOutputType(int chn,BOOL ifAuto,BOOL ifKick);//是否自动/控制，如果是控制端口是否需要补剔
	BOOL SetOutputTm(int chn,int delayTM,int keepTM);
	BOOL SetOutputTm(int chn,int delayIN,int delayTM,int keepTM);
	BOOL SetOutputSqu(int chn,int HightM,int LowTM,int PluseNum,BOOL enSqu);

	//设置索引延迟计数器延迟输出时间
	BOOL SetDelayCntParam(int nDelay);
	//设置队列控制输出脉冲的最大宽度（含脉冲串）
	BOOL SetOutMaxPluse(int nDelay);

	//设置编码器参数,暂无用

	//选择时基控制参数

	//关闭编码器工作,停止时间计数

	//停止输入计数

	//停止输出控制,通过设置输出端口属性:必须谨慎使用,用户自己控制逻辑错误,根据当前的工作模式

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

	//软件看门狗:系统看门狗和用户软件看门狗
	BOOL	SetSoftAlerm(BOOL usersoft,BOOL sysdriver,int Alarmperiod);
	BOOL	ReSetSoftAlerm();//喂狗操作
	//
	BOOL	SetAlermCoeOut(int Acoeout,int Alevel);

	//级联信号输出端口
	BOOL	SetExCodeLine(BOOL enWork,int OutType,int Coder,int coein,int delayIn,int delayTm,int keep);
	//20180701,读取消息10004时的传输时间参数
	BOOL	GetC51Message(int msgNum,C51MSG_FROMDRV &mC51Msg);
};

#endif

IOCONTROL_API int fnIoControl(void);