#ifndef _MULTISG_API_H_
#define _MULTISG_API_H_

#include "ad9361_inttypes.h"
#include "rf.h"
#include "ad9361_api.h"

#define API_MULTICHSG
#define __cplusplus 

#ifdef  API_MULTICHSG
#else
#define API_MULTICHSG  __declspec(dllexport)
#endif

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

class API_MULTICHSG multichSG
{
public:
	ad9361 AD9361;
	rf     RF;
public:
	multichSG();
	~multichSG();
public:
	int32_t OpenBoard(const uint32_t &Slot,bool init);					//打开板卡
	int32_t ClosBoard();												//关闭板卡
	int32_t GetCalibrate(bool &cal);									//是否校准
	int32_t GetSlot();
private:
	static float arb_level_offset;
	float att_table[101 * 117];										//Tx衰减值表
	float off_table[101 * 117];										//Tx校准表
public:
	int32_t Set_Tx_Sampling_Rate(uint32_t sampling_rate_Hz);		//采样率
	int32_t Get_Tx_Sampling_Rate(uint32_t &sampling_rate_Hz);
	int32_t Set_Tx_Bandwidth(uint32_t  bandwidth_Hz);				//带宽
	int32_t Get_Tx_Bandwidth(uint32_t &bandwidth_Hz);
	int32_t Set_Tx_Att(uint32_t rf_att,double ad9361_att,double bb_att);
	int32_t Set_Tx_Att(double att);
	int32_t Set_Tx(uint64_t freq, double power_dBm, bool iscw);
	int32_t Get_Tx(uint64_t freq, double power_dBm, double &cal_value, double &cal_offset);
private:
	int32_t Get_Tx(uint64_t freq, double power_dBm, uint32_t &rf_att, double &ad9361_att, double &bb_att, double &cal_offset);
private:
	float rx_cal_table[101 * 43];
	static int ad9361_gain[20];
	static int Total_gain[20];
public:
	int32_t Set_Rx_Freq(uint64_t freq);
	int32_t Set_Rx(uint64_t Freq,int ref_level,double &power);
	int32_t Get_Rx(uint64_t Freq,int ref_level,double &total_gain,double &cal_value);
private:
	int32_t Write_To_Flash(const char *filepath,float *data,int size1D,int size2D);
	int32_t	Read_From_Flash(const char *filepath,float *data,int size1D,int size2D);
private:
	uint32_t slot;
public:
	static char *get_last_error();
private:
	static int32_t set_last_error(int32_t func,const char *format, ...);
	static void	   set_last_error(const char *format, ...);
private:
	static char m_last_error[1024];
};

#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif