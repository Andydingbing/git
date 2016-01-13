#include "StdAfx.h"

#include "libCSE.h"

CSE::CSE()
{
	
}

CSE::~CSE
{

}

int32_t CSE::RF_Boot()
{

}

int32_t CSE::RF_GetRFUNumber(uint32_t RFUNumber)
{

}

int32_t CSE::RF_GetRFNumber()
{

}

int32_t CSE::RF_GetRFUSerialNumber(uint32_t RFUIndex,uint64_t &SerialNumber)
{

}

int32_t CSE::RF_GetRFUVersion(uint64_t &version)
{

}

//RF控制函数列表
int32_t CSE::RF_SetTxState(uint32_t RFIndex,bool State)
{

}

int32_t CSE::RFU_SetTxPower(uint32_t RFIndex,float Power)
{

}

int32_t CSE::RFU_SetTxFrequency(uint32_t RFIndex,double Freq)
{

}

int32_t CSE::RFU_SetTxSampleRate(uint32_t RFIndex,SAMPLERATE SampleRate)
{

}

int32_t CSE::RFU_SetTxDelay(uint32_t RFIndex,double Delay_ns)
{

}

int32_t CSE::RFU_SetTxSource(uint32_t RFIndex,SOURCE Source)
{

}

int32_t CSE::RFU_LoadARBSource(uint32_t RFIndex,char *filename)
{

}

int32_t CSE::RFU_SetRxLevel(uint32_t RFIndex,int32_t Level)
{

}

int32_t CSE::RFU_SetRxFrequency(uint32_t RFIndex,double Freq)
{

}

int32_t CSE::RFU_SetRxSampleRate(uint32_t RFIndex,SAMPLERATE SampleRate)
{

}

int32_t CSE::RFU_SetRxDelay(uint32_t RFIndex,double Delay_ns)
{

}

int32_t CSE::RFU_SetConnector(uint32_t RFIndex,CONNECTOR Connector)
{

}

	//时钟与触发源函数列表
int32_t CSE::RFU_SetClockSource(uint32_t RFIndex,RFU_CLOCKSOURCE ClockSource)
{

}

int32_t CSE::RFU_SetTriggerSource(uint32_t RFIndex,RFU_TRIGGERSOURCE TriggerSource)
{

}

	//测量设置函数列表
int32_t CSE::RFU_SetTriggerMode(uint32_t RFIndex,TRIGGERMODE TriggerMode)
{

}

int32_t CSE::RFU_SetTriggerLevel(uint32_t RFIndex,float TriggerLevel)
{

}

int32_t CSE::RFU_SetTriggerOffset(uint32_t RFIndex,uint32_t Offset)
{

}

int32_t CSE::RFU_SetCaptureLength(uint32_t RFIndex,uint32_t MLength)
{

}

int32_t CSE::RFU_SetIQDataMap(uint32_t RFIndex,uint16_t *pData)
{

}

	//功率测量函数列表
int32_t CSE::RFU_InitPowerMeasure(uint32_t RFIndex)
{

}

int32_t CSE::RFU_AbortPowerMeasure(uint32_t RFIndex)
{

}

int32_t CSE::RFU_GetMeasProcess(uint32_t RFIndex,PROCESS &Process)
{

}

int32_t CSE::RFU_GetMeasResult(uint32_t RFIndex,float &Power,float &Crest)
{

}

	//自检测量函数列表
int32_t CSE::RFU_InitIQCapture(uint32_t RFIndex)
{

}

int32_t CSE::RFU_AbortIQCapture(uint32_t RFIndex)
{

}

int32_t CSE::RFU_GetIQCaptureProcess(uint32_t RFIndex,PROCESS &Process)
{

}

	//系统控制函数列表
int32_t CSE::RFU_WarningInfo(uint32_t RFIndex,uint32_t &State)
{

}