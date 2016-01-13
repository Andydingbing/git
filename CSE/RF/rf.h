#ifndef _RF_H_
#define _RF_H_

#include "visa_driver.h"
#include "ad9361_inttypes.h"

//#define RF rf::Instance() 
const uint32_t tx_dma_mem_size  = 1024 * 1024;		//每次传1M Sample = 4Mb

const uint32_t R_BlockSamples	= 128 * 1024 * 8;	//每块128k*8 samples(4Mb)
const uint32_t R_Blocks			= 512 / 8 / 4;		//64块(总共256Mb)

enum TRX {
	Tx = 0,											//发射/接收板卡
	Rx = 1,
};
enum GPIO {
	GPA = 0,										//Tx IO扩展器 GPIOA&B;
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
	int32_t Fpga_Reset();						//FPGA复位
	int32_t	Get_Tx_Rx_Board(TRX &trx);			//发射/接收板卡
	int32_t PCIE_Open(const int &slot);

	/****************************Tx*************************/
	int32_t Turn_TxCh_On();						//打开Tx通道
	int32_t Tx_DMA_Memory_Alloc();				//申请Tx DMA内存(4MB) 
	int32_t Tx_DMA_Memory_Release();			//释放Tx DMA内存
	int32_t DMA_Start();						//DMA开始
	int32_t Fpga_DMA_Read(uint32_t *pbuf,uint32_t length,float *time);

	int32_t SetArb_Load(char * filepath);		
	int32_t SetArb_Segments(uint16_t seg);
	int32_t SetArb_Param(uint32_t addsamp,uint32_t cycl,uint32_t repmo);
	int32_t SetArb_Trigger(bool reTrigger, bool autoStart, uint32_t src, uint32_t trigDelay);
	int32_t SetArb_MultiSegMode(uint32_t msts, uint32_t msrm);
	int32_t SetArb_FreqOffset(uint32_t freqMHz);
	int32_t Arb(char *   FILEPATH,									//文件路径
				unsigned Additional_Samples=0,						//Additional Samples = 0
				unsigned Cycles=1,									//Cycles = 1
				unsigned repMode=RepMode_Continous,					//播放方式:Continuous

				unsigned ReTrigger=0,								//ReTrigger = False
				unsigned Trigger_Delay=0,							//Trigger Delay = 0
				unsigned AutoStart=1,								//AutoStart = True
				unsigned triSrc=TriSrc_Manual,						//触发源：Manual

				unsigned multiSegTrigSrc=MultiSegTrigSrc_Manual,	//多段Trigger：Manual
				unsigned multiSegRepMode=MultiSegRepMode_Auto,		//多段重复方式：Auto
				unsigned FreqOffset=0);								//频偏 = 0
	int32_t Arb_FPGA_CW(bool fpga_cw,double freq=1000000);
	int32_t Arb_Start();											//Arb开始播放
	int32_t Arb_Stop();												//Arb停止播放

	int32_t Set_Tx_Rf_Att(uint32_t att);							//设置43503衰减
	int32_t Set_Tx_IO_Ext(GPIO gpio,uint32_t gp_value);				//设置发射IO扩展器
	int32_t Set_Tx_Baseband_Att(double att);						//设置基带att
	int32_t Get_Tx_Baseband_Att(double &att);
	bool	Get_Tx_Memory_Status();									//获取发射内存状态
public:
	/****************************Rx*************************/
	int32_t Rx_CAP_Memory_Alloc();									//申请Rx DMA内存
	int32_t Rx_CAP_Memory_Release();								//释放Rx DMA内存
	int32_t Rx_Capture(int16_t *			I,							//I路值
					   int16_t *			Q,							//Q路值
					   double		 Threshold_dBm	 = 0,				//阈值	
					   uint32_t		 samples		 = 1024,			//捕获Sample数
					   char *		 FILEPATH		 = "c://1.txt",		//保存路径
				       bool			 SaveToFile	     = true,			//是否保存到文件(路径为FILEPATH)

					   IQCaptureSrcs Trig_Src	     = IQCapsrcFreeRun,	//触发源,默认为中频功率
					   bool			 B_Posedge	     = false,			//上升沿/下降沿(默认false为上升沿)
					   uint32_t		 Trig_Offset	 = 0,				//触发偏移	
					   uint32_t		 Cap_Offset	     = 0,				//捕获偏移(16的整数倍)
					   uint32_t		 Trig_Timeout    = 6553600,			//超时时间(us)/超时Samples(默认为前者)
				       uint32_t		 Trig_Gap	     = 1);				//Trigger最小间隔(单位:Sample)

	int32_t IQ_Capture_Start();										//IQ捕获开始
	int32_t IQ_Capture_Abort();										//IQ捕获终止	
	int32_t Get_IQ_To_Buf(int16_t *I,int16_t *Q,uint32_t samples);	//保存IQ数据到Buf
	int32_t	Get_IQ_To_File(char * filepath,uint32_t samples);		//保存IQ数据到文件

	int32_t Set_Rx_Freq(uint64_t Freq);								//设置接收频率
	int32_t Rx_Level_Select(uint32_t level);						//射频链路衰减器、LNA开关
	int32_t Set_Rx_Rf_Att(uint32_t att);							//射频链路衰减值
	int32_t Get_Rx_Power(double &pow,int &iq);						//获取ADC值
	int32_t Setup_DDC();											//DDC
	int32_t Set_Rx_Rf_Lo(uint32_t LoFreq);							//射频链路接收本振
	bool	Get_Rx_Memory_Status();									//获取接收内存状态
public:
	int32_t Tx_Flash_Write(float *data,int length);
	int32_t Tx_Flash_Read(float *data,int length);
private:
	int  slot;
private:
	core tx_dma_memory;												//DMA内存
	bool tx_memory_status;
	core rx_cap_memory[R_Blocks];									//IQ Capture内存
	bool rx_memory_status;
private:
	enum TriSrc	{													//DMA触发源
		TriSrc_Manual,
		TriSrc_External_Trigger_A,
		TriSrc_External_Trigger_B,
		TriSrc_External_Trigger_C,
		TriSrc_External_Trigger_D
	};
	enum RepMode {													//DMA播放方式
		RepMode_Single_Slot,
		RepMode_Continous
	};
	enum MultiSegTrigSrc {											//DMA多段触发源(暂不支持多段播放)
		MultiSegTrigSrc_Manual
	};
	enum MultiSegRepMode {											//DMA多段播放方式(暂不支持多段播放)
		MultiSegRepMode_Auto,
		MultiSegRepMode_Continous,
		MultiSegRepMode_Continous_Seamless
	};
private:
	enum IQCaptureSrcs {											//接收捕获源,Trigger为中频功率,全部捕获为FreeRun	
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
	unsigned long aSamples[R_BlockSamples];							//IQ数据临时存放
public:
	static char *get_last_error();
private:
	static void set_last_error(const char *format, ...);
private:
	static char m_last_error[512];
};

int32_t str_to_freq(char *str_freq,uint64_t &freq);					//字符串(K/M/G)转换成频率,单位不分大小写
int32_t str_to_freq(CString str_freq,uint64_t &freq);

#endif