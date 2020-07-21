
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
	WORD	m_MidDelay;//1/1024���������ӳ�
	UCHAR	m_IncDelay;//�ӳ����루��Ʒ������
	UCHAR	m_NO;//��Ч
	DWORD	m_Keep;
}PCI6356_OUTPORT_REG_SQ,*PPCI6356_OUTPORT_REG_SQ;

typedef struct  _PCI6356_OUTPORT_REG{
	union {
		UCHAR	TMreg[8];
		PCI6356_OUTPORT_REG_TM	m_TmParam;
		PCI6356_OUTPORT_REG_SQ	m_SqParam;
	}TMCtrl;//ʱ����Ʋ���
	UCHAR	m_OutChnType;//��������
	UCHAR	m_OutChnReLine;//��������
}PCI6356_OUTPORT_REG,*PPCI6356_OUTPORT_REG;

typedef struct  _PCI6356_LINE_INDEXSTATUS{
	UCHAR	mLine;//���к�
	UCHAR	mRelInp;//��������
	UCHAR	mRelInpCountIndex;//�������������������ǰ���룩
	UCHAR	mRelOutNun;//�����������������ִ�������
//	DWORD	mIndexOpStatu[256];//��������״̬���������
//	UCHAR	mIndexKeepExStatu[256];//��������״̬����չ����
	UCHAR	mOutIndexStatu[24];//���ͨ����Ӧ��������
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
	int		mCodehNum[4];//LX45�ĸ���������ÿ֡����
	DWORD   mInCoeOut;//�����������������
	DWORD   mEnInIntFlag;//�����������ӳ��ж�ʹ��
	DWORD   mInIntDelayms;//�����ӳ��ж��ӳ�ʱ��

	//�豸����
	BOOL PCI_Open(int mDev,void* hWnd=NULL);
	BOOL SetMsgWnd(void* hWnd=NULL);
	BOOL Close();
	BOOL PCI_IsOpen();
	BOOL PCI_StartWork();
	BOOL PCI_StopWork();
	BOOL PCI_SetOutStatus(int, bool);
	int  PCI_ReadInStatus(int);
	int  ReadBoardNo();//���忨��
	//�豸������汾��Ϣ
	int  ReadBoardInfo(char *sysVer);//���忨�����Ӳ��ϵͳ�汾��Ϣ
	int  GetSoftVer(char *SoftVer);//���忨���������汾��Ϣ:DLL/LIB
	int  GetHardVer(char *HardVer);//���忨����Ĺ̼��汾��Ϣ:FPGA/C51/����
	
	BOOL  WriteBoardNo(int nbd);//д�忨��
	BOOL  WriteBoardVer(char *sysVer);//д�忨�����ϵͳ�汾��Ϣ,�Ƽ���������


	//����������Ʒ�ʽ : ͳһ���ƣ�ÿ�����е�ֻ��һ��������Ʋ�����������,��Ҫ����һ������ͬʱ���ƶ�����������ִ��Ч�ʽϸߣ�
	BOOL SetResult(int nChannel,int nIndex,int nResult,int delay=0);//ÿ�ο���һ�������Ʒ����
	BOOL SetResultEx(int num,PCTRLOUT_RESULT pRes);//ÿ�ο���N�������Ʒ����
	//����������Ʒ�ʽ : �������ƣ�ÿ�����еĸ���������Ʋ����������ƣ���Ҫ����һ�����ж������ƶ�����������ִ��Ч�ʽϵͣ�C51����Ŀ��Ʋ��������������
	BOOL SetOutCtrlByResult(int nChannel,int nIndex,int lineoutpout,int nResult);//ÿ��ֻ����һ�������Ʒ������һ�����
	BOOL SetOutCtrlByResultEx(int nChannel,int nIndex,int nResult);//ÿ�ο���һ�������Ʒ������M�����
	BOOL SetOutCtrlByResultEx2(int num,PCTRLOUT_RESULT_INDEP pCtrl);//ÿ�ο���N�������Ʒ������N*M�����
	
	//����������\����\���ֵ
	int ReadInputIndex(int nChannel);//����������ֵ8λ
	int ReadInputCount(int inputn);//���������ֵ32λ

	int ReadOutputIndex(int nChannel,int nPos);//������������˳����������ֵ8λ
	int ReadOutputCount(int nChannel,int nPos);//������������˳����������ֵ32λ
	int ReadOutputCtrlCnt(int nChannel,int nPos);//���ݶ���λ�ö����������ֵ32λ
	int ReadOutputBTCnt(int outputn,int nPos);//������������˳���������߼�����
	int ReadOutputCount(int outputn);//������˿ں�ֱ�Ӷ��������ֵ32λ
	int ReadOutputCtrlCnt(int outputn);//������˿ں�ֱ�Ӷ��������ֵ32λ
	int ReadOutputBTCnt(int outputn);//������˿ں�ֱ�Ӷ�������߼�����
	//���ӳٶ�������\����\���ֵ
	int ReadInputdelayIndex(int nChannel);//�������ӳ���������ֵ8λ
	int ReadOutputdelayIndex(int nChannel,int nPos);//������������˳����ӳ��������ֵ8λ

	int ReadInPortdelayCount(int ninPort,int nDelay);//������˿ں�ֱ�Ӷ��ӳ��������ֵ8λ,ͬʱ�����ӳ�
	int ReadOutputdelayCount(int noutPort,int nDelay);//������˿ں�ֱ�Ӷ��ӳ��������ֵ8λ,ͬʱ�����ӳ�


	BOOL PCI_Init(const char* strPath, bool b_Pause = false);//����������ó�ʼ��״̬,����������
//	BOOL WriteParameterFile(const char* strPath);

	BOOL IOWrite(int addr, BYTE mval);
	BOOL IORead(int addr, BYTE *pval);
	BOOL IOWrite32(int addr, DWORD mval);
	BOOL IORead32(int addr, DWORD *pval);
	BOOL RegWrite32(int addr, DWORD mval);
	BOOL RegRead32(int addr, DWORD *pval);

	//��չ����
	//�õ������ƽ״̬
	BOOL ReadInPortStatus(BYTE &inPortReg);
	//�յ��ж��¼�,��ȡ���뼰���ӳٱ仯״̬�ͷ���ʱ��
	BOOL ReadInChanged(UCHAR &instatus,UCHAR &indelaystatus,DWORD &time2uscnt);

	//��������,needReset=TRUE��Ҫ��λ���ڲ�ִ��ResetALL()
	BOOL StartWork(BOOL needReset=TRUE);
	//ֹͣ����,���������������Ͷ��в���
	BOOL StopWork(BOOL cutALL=TRUE);
	//���¸�λ��ʼ��,��λC51���Ʋ�����FPGA����״̬�����м�����
	BOOL ResetALL();
	//���¸�λ���м�����
	BOOL ResetCount();
	//��ͣ������Ƽ���	
	BOOL PauseInCntWork(int nPortIn);
	//���������������
	BOOL ContinueInCntWork(int nPortIn);
	//���뵱ǰʱ�����,�ɺ���
	BOOL GetAllTmParam(PCTRL_TIMEREG nowTmParam);
	//���뵱ǰʱ�����,���ݾɺ���,������ʹ��
	BOOL GetAllTmParam(PCTRL_TIMEREG_LX45 nowTmParam);
	//����ʱ��/����������
	BOOL PCI_GetTm2us(DWORD &nowTm2us);
	BOOL PCI_GetTmCoder(int coder_n,DWORD &nowhAll,DWORD &nowf,DWORD &nowh);
	BOOL PCI_GetTmCoder(int coder_n,DWORD &nowhAll);
	BOOL PCI_GetTmCoder(int coder_n,DWORD &nowf,DWORD &nowh);

	//��������/����������
	BOOL GetRate(int port,float &rate);

	//2017-12-15��������������������ƽӿں���������ʹ��
	//�����������:�˲�����
	BOOL InitInputCh(int chn,int Filter,float maxRate);//Filter:��Ե�˲�����,maxRate:����ϲ�����

	//�����������:�ӳٲ����뱣�ֲ���,ע�����ģʽ��ͬ,�����������\ʱ���׼����\����������
	//�������һ���˿�������Ҫ֪ͨC51������޸Ĳ�������Ӱ�����һ���˿����򣬷���Ӱ��������п��ƣ�ֻ��ִ�������ļ���ʼ���ﵽ
	BOOL InitOutputCh(PFPGA_OUTPUT_PARAM paramCh);
	BOOL SetOutputType(int chn,BOOL ifAuto,BOOL ifKick);//�Ƿ��Զ�/���ƣ�����ǿ��ƶ˿��Ƿ���Ҫ����
	BOOL SetOutputTm(int chn,int delayTM,int keepTM);
	BOOL SetOutputTm(int chn,int delayIN,int delayTM,int keepTM);
	BOOL SetOutputSqu(int chn,int HightM,int LowTM,int PluseNum,BOOL enSqu);

	//���������ӳټ������ӳ����ʱ��
	BOOL SetDelayCntParam(int nDelay);
	//���ö��п���������������ȣ������崮��
	BOOL SetOutMaxPluse(int nDelay);

	//���ñ���������,������

	//ѡ��ʱ�����Ʋ���

	//�رձ���������,ֹͣʱ�����

	//ֹͣ�������

	//ֹͣ�������,ͨ����������˿�����:�������ʹ��,�û��Լ������߼�����,���ݵ�ǰ�Ĺ���ģʽ

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

	//������Ź�:ϵͳ���Ź����û�������Ź�
	BOOL	SetSoftAlerm(BOOL usersoft,BOOL sysdriver,int Alarmperiod);
	BOOL	ReSetSoftAlerm();//ι������
	//
	BOOL	SetAlermCoeOut(int Acoeout,int Alevel);

	//�����ź�����˿�
	BOOL	SetExCodeLine(BOOL enWork,int OutType,int Coder,int coein,int delayIn,int delayTm,int keep);
	//20180701,��ȡ��Ϣ10004ʱ�Ĵ���ʱ�����
	BOOL	GetC51Message(int msgNum,C51MSG_FROMDRV &mC51Msg);
};

#endif

IOCONTROL_API int fnIoControl(void);