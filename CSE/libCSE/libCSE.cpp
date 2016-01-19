#include "StdAfx.h"

#include "libCSE.h"

CSE::CSE()
{
	
}

CSE::~CSE()
{

}

int32_t CSE::RF_Boot()
{
	return 0;
}

int32_t CSE::RF_GetRFUNumber(uint32_t RFUNumber)
{
	return 0;
}

int32_t CSE::RF_GetRFNumber()
{
	return 0;
}

int32_t CSE::RF_GetRFUSerialNumber(uint32_t RFUIndex,uint64_t &SerialNumber)
{
	return 0;
}

int32_t CSE::RF_GetRFUVersion(uint64_t &version)
{
	return 0;
}

//RF控制函数列表
int32_t CSE::RF_SetTxState(uint32_t RFIndex,bool State)
{
	return 0;
}

int32_t CSE::RFU_SetTxPower(uint32_t RFIndex,float Power)
{
	return 0;
}

int32_t CSE::RFU_SetTxFrequency(uint32_t RFIndex,double Freq)
{
	return 0;
}

int32_t CSE::RFU_SetTxSampleRate(uint32_t RFIndex,SAMPLERATE SampleRate)
{
	return 0;
}

int32_t CSE::RFU_SetTxDelay(uint32_t RFIndex,double Delay_ns)
{
	return 0;
}

int32_t CSE::RFU_SetTxSource(uint32_t RFIndex,SOURCE Source)
{
	return 0;
}

int32_t CSE::RFU_LoadARBSource(uint32_t RFIndex,char *filename)
{
	return 0;
}

int32_t CSE::RFU_SetRxLevel(uint32_t RFIndex,int32_t Level)
{
	return 0;
}

int32_t CSE::RFU_SetRxFrequency(uint32_t RFIndex,double Freq)
{
	return 0;
}

int32_t CSE::RFU_SetRxSampleRate(uint32_t RFIndex,SAMPLERATE SampleRate)
{
	return 0;
}

int32_t CSE::RFU_SetRxDelay(uint32_t RFIndex,double Delay_ns)
{
	return 0;
}

int32_t CSE::RFU_SetConnector(uint32_t RFIndex,CONNECTOR Connector)
{
	return 0;
}

	//时钟与触发源函数列表
int32_t CSE::RFU_SetClockSource(uint32_t RFIndex,RFU_CLOCKSOURCE ClockSource)
{
	return 0;
}

int32_t CSE::RFU_SetTriggerSource(uint32_t RFIndex,RFU_TRIGGERSOURCE TriggerSource)
{
	return 0;
}

	//测量设置函数列表
int32_t CSE::RFU_SetTriggerMode(uint32_t RFIndex,TRIGGERMODE TriggerMode)
{
	return 0;
}

int32_t CSE::RFU_SetTriggerLevel(uint32_t RFIndex,float TriggerLevel)
{
	return 0;
}

int32_t CSE::RFU_SetTriggerOffset(uint32_t RFIndex,uint32_t Offset)
{
	return 0;
}

int32_t CSE::RFU_SetCaptureLength(uint32_t RFIndex,uint32_t MLength)
{
	return 0;
}

int32_t CSE::RFU_SetIQDataMap(uint32_t RFIndex,uint16_t *pData)
{
	return 0;
}

	//功率测量函数列表
int32_t CSE::RFU_InitPowerMeasure(uint32_t RFIndex)
{
	return 0;
}

int32_t CSE::RFU_AbortPowerMeasure(uint32_t RFIndex)
{
	return 0;
}

int32_t CSE::RFU_GetMeasProcess(uint32_t RFIndex,PROCESS &Process)
{
	return 0;
}

int32_t CSE::RFU_GetMeasResult(uint32_t RFIndex,float &Power,float &Crest)
{
	return 0;
}

	//自检测量函数列表
int32_t CSE::RFU_InitIQCapture(uint32_t RFIndex)
{
	return 0;
}

int32_t CSE::RFU_AbortIQCapture(uint32_t RFIndex)
{
	return 0;
}

int32_t CSE::RFU_GetIQCaptureProcess(uint32_t RFIndex,PROCESS &Process)
{
	return 0;
}

	//系统控制函数列表
int32_t CSE::RFU_WarningInfo(uint32_t RFIndex,uint32_t &State)
{
	return 0;
}