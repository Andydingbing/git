#ifndef _LIBCSE_H_
#define _LIBCSE_H_

#include "stdint.h"
#include "rf.h"

//#define lib_CES
#ifdef lib_CSE
#else
#define lib_CSE __declspec(dllexport)
#endif

#define __cplusplus
#if defined (__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

class lib_CSE CSE
{
public:
	CSE();
	virtual ~CSE();
public:
	rf RF_1;
	rf RF_2;
	rf RF_3;
	rf RF_4;
public:
	enum SAMPLERATE
	{
		BW40,BW80,BW100,BW160
	};
	enum SOURCE
	{
		ARB,FPGA,CW
	};
	enum CONNECTOR
	{
		IO,OUTPUT,LOOP
	};
	enum RFU_CLOCKSOURCE
	{
		RFU_CLOCK_INTERNAL,
		RFU_CLOCK_EXTERNAL,
	};
	enum RFU_TRIGGERSOURCE
	{
		RFU_TRIGGER_INTERNAL,
		RFU_TRIGGER_EXTERNAL
	};
	enum TRIGGERMODE
	{
		IF,FREERUN,MARKER
	};
	enum PROCESS
	{
		IDLE_Driver,
		WFTrigger_Driver,
		Timeout_Driver,
		RUNNING_Driver,
		DONE_Driver
	};
	//启动函数列表
public:
	int32_t RF_Boot();
	int32_t RF_GetRFUNumber(uint32_t RFUNumber);
	int32_t RF_GetRFNumber();
	int32_t RF_GetRFUSerialNumber(uint32_t RFUIndex,uint64_t &SerialNumber);
	int32_t RF_GetRFUVersion(uint64_t &version);
	//RF控制函数列表
public:
	int32_t RF_SetTxState(uint32_t RFIndex,bool State);
	int32_t RFU_SetTxPower(uint32_t RFIndex,float Power);
	int32_t RFU_SetTxFrequency(uint32_t RFIndex,double Freq);
	int32_t RFU_SetTxSampleRate(uint32_t RFIndex,SAMPLERATE SampleRate);
	int32_t RFU_SetTxDelay(uint32_t RFIndex,double Delay_ns);
	int32_t RFU_SetTxSource(uint32_t RFIndex,SOURCE Source);
	int32_t RFU_LoadARBSource(uint32_t RFIndex,char *filename);
public:
	int32_t RFU_SetRxLevel(uint32_t RFIndex,int32_t Level);
	int32_t RFU_SetRxFrequency(uint32_t RFIndex,double Freq);
	int32_t RFU_SetRxSampleRate(uint32_t RFIndex,SAMPLERATE SampleRate);
	int32_t RFU_SetRxDelay(uint32_t RFIndex,double Delay_ns);
	int32_t RFU_SetConnector(uint32_t RFIndex,CONNECTOR Connector);
	//时钟与触发源函数列表
public:
	int32_t RFU_SetClockSource(uint32_t RFIndex,RFU_CLOCKSOURCE ClockSource = RFU_CLOCK_INTERNAL);
	int32_t RFU_SetTriggerSource(uint32_t RFIndex,RFU_TRIGGERSOURCE TriggerSource = RFU_TRIGGER_INTERNAL);
	//测量设置函数列表
public:
	int32_t RFU_SetTriggerMode(uint32_t RFIndex,TRIGGERMODE TriggerMode = IF);
	int32_t RFU_SetTriggerLevel(uint32_t RFIndex,float TriggerLevel = 0);
	int32_t RFU_SetTriggerOffset(uint32_t RFIndex,uint32_t Offset);
	int32_t RFU_SetCaptureLength(uint32_t RFIndex,uint32_t MLength);
	int32_t RFU_SetIQDataMap(uint32_t RFIndex,uint16_t *pData);
	//功率测量函数列表
public:
	int32_t RFU_InitPowerMeasure(uint32_t RFIndex);
	int32_t RFU_AbortPowerMeasure(uint32_t RFIndex);
	int32_t RFU_GetMeasProcess(uint32_t RFIndex,PROCESS &Process);
	int32_t RFU_GetMeasResult(uint32_t RFIndex,float &Power,float &Crest);
	//自检测量函数列表
public:
	int32_t RFU_InitIQCapture(uint32_t RFIndex);
	int32_t RFU_AbortIQCapture(uint32_t RFIndex);
	int32_t RFU_GetIQCaptureProcess(uint32_t RFIndex,PROCESS &Process);
	//系统控制函数列表
public:
	int32_t RFU_WarningInfo(uint32_t RFIndex,uint32_t &State);
};

#if defined (__cplusplus) || defined(__cplusplus__)
}
#endif

#endif