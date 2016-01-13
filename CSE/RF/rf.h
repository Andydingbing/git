#ifndef _RF_H_
#define _RF_H_

#include "visa_driver.h"
#include "ad9361_inttypes.h"

//#define RF rf::Instance() 
const uint32_t tx_dma_mem_size  = 1024 * 1024;		//ÿ�δ�1M Sample = 4Mb

const uint32_t R_BlockSamples	= 128 * 1024 * 8;	//ÿ��128k*8 samples(4Mb)
const uint32_t R_Blocks			= 512 / 8 / 4;		//64��(�ܹ�256Mb)

enum TRX {
	Tx = 0,											//����/���հ忨
	Rx = 1,
};
enum GPIO {
	GPA = 0,										//Tx IO��չ�� GPIOA&B;
	GPB = 1,
};

class rf
{
public:
	rf();
	~rf(){};
	static rf &Instance();
public:
	int32_t Get_FPGA_Version(uint32_t &ver);
	const char *Get_Driver_Version();
public:
	int32_t Fpga_Reset();						//FPGA��λ
	int32_t	Get_Tx_Rx_Board(TRX &trx);			//����/���հ忨
	int32_t PCIE_Open(const int &slot);

	/****************************Tx*************************/
	int32_t Turn_TxCh_On();						//��Txͨ��
	int32_t Tx_DMA_Memory_Alloc();				//����Tx DMA�ڴ�(4MB) 
	int32_t Tx_DMA_Memory_Release();			//�ͷ�Tx DMA�ڴ�
	int32_t DMA_Start();						//DMA��ʼ
	int32_t Fpga_DMA_Read(uint32_t *pbuf,uint32_t length,float *time);

	int32_t SetArb_Load(char * filepath);		
	int32_t SetArb_Segments(uint16_t seg);
	int32_t SetArb_Param(uint32_t addsamp,uint32_t cycl,uint32_t repmo);
	int32_t SetArb_Trigger(bool reTrigger, bool autoStart, uint32_t src, uint32_t trigDelay);
	int32_t SetArb_MultiSegMode(uint32_t msts, uint32_t msrm);
	int32_t SetArb_FreqOffset(uint32_t freqMHz);
	int32_t Arb(char *   FILEPATH,									//�ļ�·��
				unsigned Additional_Samples=0,						//Additional Samples = 0
				unsigned Cycles=1,									//Cycles = 1
				unsigned repMode=RepMode_Continous,					//���ŷ�ʽ:Continuous

				unsigned ReTrigger=0,								//ReTrigger = False
				unsigned Trigger_Delay=0,							//Trigger Delay = 0
				unsigned AutoStart=1,								//AutoStart = True
				unsigned triSrc=TriSrc_Manual,						//����Դ��Manual

				unsigned multiSegTrigSrc=MultiSegTrigSrc_Manual,	//���Trigger��Manual
				unsigned multiSegRepMode=MultiSegRepMode_Auto,		//����ظ���ʽ��Auto
				unsigned FreqOffset=0);								//Ƶƫ = 0
	int32_t Arb_FPGA_CW(bool fpga_cw,double freq=1000000);
	int32_t Arb_Start();											//Arb��ʼ����
	int32_t Arb_Stop();												//Arbֹͣ����

	int32_t Set_Tx_Rf_Att(uint32_t att);							//����43503˥��
	int32_t Set_Tx_IO_Ext(GPIO gpio,uint32_t gp_value);				//���÷���IO��չ��
	int32_t Set_Tx_Baseband_Att(double att);						//���û���att
	int32_t Get_Tx_Baseband_Att(double &att);
	bool	Get_Tx_Memory_Status();									//��ȡ�����ڴ�״̬
public:
	/****************************Rx*************************/
	int32_t Rx_CAP_Memory_Alloc();									//����Rx DMA�ڴ�
	int32_t Rx_CAP_Memory_Release();								//�ͷ�Rx DMA�ڴ�
	int32_t Rx_Capture(int16_t *			I,							//I·ֵ
					   int16_t *			Q,							//Q·ֵ
					   double		 Threshold_dBm	 = 0,				//��ֵ	
					   uint32_t		 samples		 = 1024,			//����Sample��
					   char *		 FILEPATH		 = "c://1.txt",		//����·��
				       bool			 SaveToFile	     = true,			//�Ƿ񱣴浽�ļ�(·��ΪFILEPATH)

					   IQCaptureSrcs Trig_Src	     = IQCapsrcFreeRun,	//����Դ,Ĭ��Ϊ��Ƶ����
					   bool			 B_Posedge	     = false,			//������/�½���(Ĭ��falseΪ������)
					   uint32_t		 Trig_Offset	 = 0,				//����ƫ��	
					   uint32_t		 Cap_Offset	     = 0,				//����ƫ��(16��������)
					   uint32_t		 Trig_Timeout    = 6553600,			//��ʱʱ��(us)/��ʱSamples(Ĭ��Ϊǰ��)
				       uint32_t		 Trig_Gap	     = 1);				//Trigger��С���(��λ:Sample)

	int32_t IQ_Capture_Start();										//IQ����ʼ
	int32_t IQ_Capture_Abort();										//IQ������ֹ	
	int32_t Get_IQ_To_Buf(int16_t *I,int16_t *Q,uint32_t samples);	//����IQ���ݵ�Buf
	int32_t	Get_IQ_To_File(char * filepath,uint32_t samples);		//����IQ���ݵ��ļ�

	int32_t Set_Rx_Freq(uint64_t Freq);								//���ý���Ƶ��
	int32_t Rx_Level_Select(uint32_t level);						//��Ƶ��·˥������LNA����
	int32_t Set_Rx_Rf_Att(uint32_t att);							//��Ƶ��·˥��ֵ
	int32_t Get_Rx_Power(double &pow,int &iq);						//��ȡADCֵ
	int32_t Setup_DDC();											//DDC
	int32_t Set_Rx_Rf_Lo(uint32_t LoFreq);							//��Ƶ��·���ձ���
	bool	Get_Rx_Memory_Status();									//��ȡ�����ڴ�״̬
public:
	int32_t Tx_Flash_Write(float *data,int length);
	int32_t Tx_Flash_Read(float *data,int length);
private:
	int  slot;
private:
	core tx_dma_memory;												//DMA�ڴ�
	bool tx_memory_status;
	core rx_cap_memory[R_Blocks];									//IQ Capture�ڴ�
	bool rx_memory_status;
private:
	enum TriSrc	{													//DMA����Դ
		TriSrc_Manual,
		TriSrc_External_Trigger_A,
		TriSrc_External_Trigger_B,
		TriSrc_External_Trigger_C,
		TriSrc_External_Trigger_D
	};
	enum RepMode {													//DMA���ŷ�ʽ
		RepMode_Single_Slot,
		RepMode_Continous
	};
	enum MultiSegTrigSrc {											//DMA��δ���Դ(�ݲ�֧�ֶ�β���)
		MultiSegTrigSrc_Manual
	};
	enum MultiSegRepMode {											//DMA��β��ŷ�ʽ(�ݲ�֧�ֶ�β���)
		MultiSegRepMode_Auto,
		MultiSegRepMode_Continous,
		MultiSegRepMode_Continous_Seamless
	};
private:
	enum IQCaptureSrcs {											//���ղ���Դ,TriggerΪ��Ƶ����,ȫ������ΪFreeRun	
		IQCapsrcFreeRun = 0x00,
		IQCapsrcRstMkr  = 0x09,
		IQCapsrcMkr1	= 0x0a,
		IQCapsrcMkr2	= 0x0b,
		IQCapsrcMkr3	= 0x0c,
		IQCapsrcMkr4	= 0x0d,
		IQCapsrcUsrMkr	= 0x0e,
		IQCapsrcETrig	= 0x11,
		IQCapsrcETrig1	= 0x12,
		IQCapsrcETrig2	= 0x13,
		IQCapsrcETrig3	= 0x14,
		IQCapsrcETrig4	= 0x15,
		IQCapsrcIFPwr	= 0x21
	};
private:
	unsigned long aSamples[R_BlockSamples];							//IQ������ʱ���
public:
	static char *get_last_error();
private:
	static void set_last_error(const char *format, ...);
private:
	static char m_last_error[512];
};

int32_t str_to_freq(char *str_freq,uint64_t &freq);					//�ַ���(K/M/G)ת����Ƶ��,��λ���ִ�Сд
int32_t str_to_freq(CString str_freq,uint64_t &freq);

#endif