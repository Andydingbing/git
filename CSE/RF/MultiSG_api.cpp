#include "StdAfx.h"

#include "RegisterDefine.h"
#include "ArbReader.h"
#include "log.h"
#include "rf.h"
#include "MultiSG_api.h"
#include "math.h"

multichSG MULTICHSG_Tx;
multichSG MULTICHSG_Rx;

#define wv_peak_amplitude  32768.0
#define cal_peak_amplitude 16384.0

float multichSG::arb_level_offset = 0;
char multichSG::m_last_error[1024] = {0};

//RX Gain Table
int multichSG::ad9361_gain[20] = {20,25,30,35,40,45,50,55,60,5,10,15,20,25,30,35,40,45,50,55};
int multichSG::Total_gain[20] = {-14,-9,-4,1,-4,1,6,11,16,15,20,25,30,35,40,45,50,55,60,65};

multichSG::multichSG()
{
}
multichSG::~multichSG()
{
}

int32_t multichSG::OpenBoard(const uint32_t &Slot,bool init)
{
	TRX tr;
	bool caled;
	slot = Slot;
	Status_Check(set_last_error(RF.PCIE_Open(slot),"OpenBoard Error! %s",RF.get_last_error()));
	if (init) {
// 		Status_Check(set_last_error(PCIE_Core.WriteWordBar1(0x88,0x0008),"OpenBoard Error! %s",PCIE_Core.get_last_error()));
// 		Status_Check(set_last_error(PCIE_Core.WriteWordBar1(0x88,0x0000),"OpenBoard Error! %s",PCIE_Core.get_last_error()));
		Status_Check(set_last_error(AD9361.SoftReset(),"OpenBoard Error! %s",AD9361.get_last_error()));
		Log.Write_Message_List(0,"AD9361 Reset Complete");
		Status_Check(set_last_error(AD9361.Init(slot),"OpenBoard Error! %s",AD9361.get_last_error()));
		Log.Write_Message_List(0,"AD9361 Init Complete");

		Status_Check(set_last_error(RF.Get_Tx_Rx_Board(tr),"OpenBoard Error! %s",RF.get_last_error()));
		GetCalibrate(caled);
		Log.Write_Message_List(caled?0:-1,"模块%s校准",caled?"已":"没有");
		switch (tr) {
			case Tx: 
				{
					Status_Check(set_last_error(RF.Tx_DMA_Memory_Alloc(),"OpenBoard Error! %s",RF.get_last_error()));
					Log.Write_Message_List(0,"Tx DMA Memory Alloc Success");
					Status_Check(set_last_error(Arb_Stop(),"OpenBoard Error! %s",get_last_error()));					
					Status_Check(set_last_error(RF.Turn_TxCh_On(),"OpenBoard Error! %s",RF.get_last_error()));
					Log.Write_Message_List(0,"Tx 通道打开");
					Status_Check(set_last_error(Set_Tx_Att(0,76,0),"OpenBoard Error! %s",RF.get_last_error()));
					Status_Check(set_last_error(AD9361.set_tx_rf_bandwidth(30e6),"OpenBoard Error! %s",AD9361.get_last_error()));
					Status_Check(set_last_error(AD9361.set_en_state_machine_mode(ENSM_STATE_TX),""));

					if (caled) {
 						char att_path[512],off_path[512];
// 						sprintf(att_path,"c:\\tx_cal_data@slot%d.txt",slot);
 						sprintf(off_path,"c:\\MultiCHSG\\tx_cal_offset@slot%d.txt",slot);
// 						Read_From_Flash(att_path,(float *)att_table,101,117);
 						Read_From_Flash(off_path,(float *)off_table,101,117);
						RF.Tx_Flash_Read(att_table,101 * 117);
					}
					break;
				}
			case Rx:
				{
					Status_Check(set_last_error(RF.Rx_CAP_Memory_Alloc(),"OpenBoard Error! %s",RF.get_last_error()));
					Log.Write_Message_List(0,"Rx Capture Memory Alloc Success");
					Status_Check(set_last_error(RF.Rx_Level_Select(1),"OpenBoard Error! %s",RF.get_last_error()));
					Status_Check(set_last_error(RF.Set_Rx_Rf_Att(24),"OpenBoard Error! %s",RF.get_last_error()));
					Status_Check(set_last_error(RF.Setup_DDC(),"OpenBoard Error! %s",RF.get_last_error()));
					Status_Check(set_last_error(AD9361.set_rx_rf_bandwidth(30e6),"OpenBoard Error! %s",AD9361.get_last_error()));
					Status_Check(set_last_error(AD9361.set_rx_sampling_freq(61.44e6),"OpenBoard Error! %s",AD9361.get_last_error()));
					Status_Check(set_last_error(AD9361.set_rx_gain_control_mode(0,RF_GAIN_MGC),"OpenBoard Error! %s",AD9361.get_last_error()));
					Status_Check(set_last_error(AD9361.set_rx_rf_gain(0,0),"OpenBoard Error! %s",AD9361.get_last_error()));
					Status_Check(set_last_error(AD9361.set_en_state_machine_mode(ENSM_STATE_RX),"OpenBoard Error! %s",AD9361.get_last_error()));

					if (caled) {
						char rx_cal_path[512];
						sprintf(rx_cal_path,"c:\\MultiCHSG\\rx_cal_data@slot%d.txt",slot);
						Read_From_Flash(rx_cal_path,(float *)rx_cal_table,101,43);
					}
					break;
				}
		}
	}
	Sleep(10);
	return 0;
}

int32_t multichSG::ClosBoard()
{
	TRX tr;
	Status_Check(set_last_error(RF.Get_Tx_Rx_Board(tr),"ClosBoard Error! %s",RF.get_last_error()));
	switch (tr) {
		case Tx:								//Tx			
			Status_Check(set_last_error(Arb_Stop(),"Arb Stop Error!"));	
			Log.Write_Message_List(0,"Arb Stop");
			Status_Check(set_last_error(AD9361.set_en_state_machine_mode(ENSM_STATE_ALERT),"ClosBoard Error! %s",AD9361.get_last_error()));
			Status_Check(set_last_error(RF.Tx_DMA_Memory_Release(),"ClosBoard Error! %s",RF.get_last_error()));	
			Log.Write_Message_List(0,"Tx DMA Memory Release Success");
			break;
		case Rx:									//Rx
			Status_Check(set_last_error(RF.Rx_Level_Select(1),"ClosBoard Error! %s",RF.get_last_error()));
			Status_Check(set_last_error(AD9361.set_en_state_machine_mode(ENSM_STATE_ALERT),"ClosBoard Error! %s",AD9361.get_last_error()));
			Status_Check(set_last_error(RF.Rx_CAP_Memory_Release(),"ClosBoard Error! %s",RF.get_last_error()));
			Log.Write_Message_List(0,"Rx Capture Memory Release Success");
			break;
	}
	AD9361.SoftReset();
	return 0;
}

int32_t multichSG::GetCalibrate(bool &cal)
{
	TRX tr;
	char cal_path[512];
	FILE *fp;

	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(RF.Get_Tx_Rx_Board(tr),"GetCal Error! %s",RF.get_last_error()));
	switch (tr) {
		case Tx:
			sprintf(cal_path,"c:\\MultiCHSG\\tx_cal_data@slot%d.cal",slot);
			break;
		case Rx:
			sprintf(cal_path,"c:\\MultiCHSG\\rx_cal_data@slot%d.txt",slot);
			break;
	}
	fp = fopen(cal_path,"rb");
	if (fp)	{
		cal = true;
		fclose(fp);
	}
	else	
		cal = false;
	return 0;
}

int32_t multichSG::GetSlot()
{
	return slot;
}

int32_t multichSG::Set_Tx_Sampling_Rate(uint32_t sampling_rate_Hz)
{
	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(AD9361.set_tx_sampling_freq(sampling_rate_Hz),"Set Sample Rate Error! %s",AD9361.get_last_error()));
	return 0;
}

int32_t multichSG::Get_Tx_Sampling_Rate(uint32_t &sampling_rate_Hz)
{
	uint32_t rate;
	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(AD9361.get_tx_sampling_freq(&rate),"Get Sample Rate Error! %s",AD9361.get_last_error()));
	sampling_rate_Hz = rate;
	return 0;
}

int32_t multichSG::Set_Tx_Bandwidth(uint32_t  bandwidth_Hz)
{
	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(AD9361.set_tx_rf_bandwidth(bandwidth_Hz),"Set Bandwidth Error! %s",AD9361.get_last_error()));
	return 0;
}

int32_t multichSG::Get_Tx_Bandwidth(uint32_t &bandwidth_Hz)
{
	uint32_t bandwidth;
	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(AD9361.get_tx_rf_bandwidth(&bandwidth),"Get Bandwidth Error! %s",AD9361.get_last_error()));
	bandwidth_Hz = bandwidth;
	return 0;
}

int32_t multichSG::Set_Tx_Att(uint32_t rf_att,double ad9361_att,double bb_att)
{
	Status_Check(OpenBoard(slot,false));
	Status_Check(RF.Set_Tx_Rf_Att(rf_att));
	if (AD9361.set_tx_attenuation(0, ad9361_att * 1000.0)) {
		set_last_error("RF:Set Tx Att Error! %s",AD9361.get_last_error());
		return -1;
	}
	Status_Check(RF.Set_Tx_Baseband_Att(bb_att));
	return 0;
}

int32_t multichSG::Set_Tx_Att(double att)
{
	uint32_t rf_att		= 0;		//(24)
	double	 ad9361_att = 0.0;		//(0,2,4,6,...)
	double	 bb_att		= 0.0;

	rf_att = (att >= 24)?24:0;
	if (att > 3 && (((uint32_t)(att - (double)rf_att) + 1) % 10 == 0)) {
		if (((att - (double)rf_att) - (uint32_t)(att - (double)rf_att)) > 0.3 &&
			((att - (double)rf_att) - (uint32_t)(att - (double)rf_att)) < 1)
			ad9361_att = (uint32_t)(att - (double)rf_att) / 10 * 10 + 10;
		else
			ad9361_att = (uint32_t)(att - (double)rf_att) / 10 * 10;
	}
	else
		ad9361_att = (uint32_t)(att - (double)rf_att) / 10 * 10;
	bb_att = -1 * (att - (double)rf_att - ad9361_att);

	Status_Check(OpenBoard(slot,false));
	Status_Check(RF.Set_Tx_Rf_Att(rf_att));
	// 	if (AD9361.set_tx_attenuation(0, ad9361_att * 1000.0)) {
	// 		set_last_error("RF:Set Tx Att Error! %s",AD9361.get_last_error());
	// 		return -1;
	// 	}	
	Status_Check(RF.Set_Tx_Baseband_Att(bb_att));

//	Log.stdprintf("rf_att = %d	ad9361_att = %6.3lf	bb_att = %6.3lf\n",rf_att,ad9361_att,bb_att);
	return 0;
}

int32_t multichSG::Set_Tx(uint64_t freq, double power_dBm, bool iscw)
{
	double OFF = 0.0;
	double arb_off = 0;

	double ad9361_att = 0.0;									
	double bb_att	  = 0.0;
	uint32_t rf_att   = 0;

	Get_Tx(freq,power_dBm,rf_att,ad9361_att,bb_att,OFF);

	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(AD9361.set_tx_lo_freq(freq + (iscw?1e6:0)),"Set Tx Error! %s",AD9361.get_last_error()));
	
	arb_off = iscw?0:(arb_level_offset - 20 * log10(wv_peak_amplitude / cal_peak_amplitude));
	Log.stdprintf("arb_offset = %f\n",arb_off);
	bb_att += arb_off;
	Status_Check(set_last_error(Set_Tx_Att(rf_att,ad9361_att,bb_att),"Set Tx Error! %s",RF.get_last_error()));
	return 0;
}

int32_t multichSG::Get_Tx(uint64_t freq, double power_dBm, double &cal_value, double &cal_offset)
{
	double ad9361_att = 0.0;									
	double bb_att	  = 0.0;
	uint32_t rf_att   = 0;
	Get_Tx(freq,power_dBm,rf_att,ad9361_att,bb_att,cal_offset);
	cal_value = (double)rf_att + ad9361_att - bb_att;
	return 0;
}

int32_t multichSG::Get_Tx(uint64_t freq, double power_dBm, uint32_t &rf_att, double &ad9361_att, double &bb_att, double &cal_offset)
{
	uint64_t u_freq = 0,d_freq = 0;
	int u_dBm = 0,d_dBm = 0;
	float att1,att2,att3,att4,bbatt[2];
	float off1,off2,off3,off4,off[2];
	double arb_off = 0;

	freq = (uint64_t)(freq / 1e6);					//MHz
	if (power_dBm > 15)		power_dBm = 15;
	if (power_dBm < -85)	power_dBm = -85;

	u_freq = (freq / 50 + 1) * 50;					//拟合的上一个频率点
	if (u_freq >= 6000) u_freq = 6000;
	d_freq = freq / 50  * 50;						//拟合的下一个频率点
	if (d_freq <= 200)	d_freq = 200;

	u_dBm = ((int)power_dBm) <= 0?((int)power_dBm):((int)power_dBm + (int)((abs(power_dBm) == power_dBm)?0:1));
	if (u_dBm > 15)	u_dBm = 15;						//拟合的上一个功率点
	d_dBm = u_dBm - 1;								//拟合的下一个功率点
	if (d_dBm <-85) d_dBm = -85;

	int index_dBm = 15 - u_dBm;						//拟合的上功率的index
	int index_fre = (int)(d_freq - 200) / 50;		//拟合的下频率的index

	att1 = att_table[117 * index_dBm + index_fre];
	att2 = att_table[117 * index_dBm + index_fre + 1];
	att3 = att_table[117 * (index_dBm + 1) + index_fre];
	att4 = att_table[117 * (index_dBm + 1) + index_fre + 1];

	off1 = off_table[117 * index_dBm + index_fre];
	off2 = off_table[117 * index_dBm + index_fre + 1];
	off3 = off_table[117 * (index_dBm + 1) + index_fre];
	off4 = off_table[117 * (index_dBm + 1) + index_fre + 1];
	if (att1 == 0) off1 = 0;
	if (att2 == 0) off2 = 0;
	if (att3 == 0) off3 = 0;
	if (att4 == 0) off4 = 0;
 	Log.stdprintf("att1234 =%f  %f  %f  %f\n",att1,att2,att3,att4);
 	Log.stdprintf("off1234 =%f  %f  %f  %f\n",off1,off2,off3,off4);

	rf_att = 0;
	ad9361_att = (double)(uint32_t)(abs(att1));							//整数部分代表9361衰减
	att1 = att1 + ((att1>=0)?-1:1) * (double)(uint32_t)(abs(att1));		//取出小数部分并拟合
	att2 = att2 + ((att2>=0)?-1:1) * (double)(uint32_t)(abs(att2));
	att3 = att3 + ((att3>=0)?-1:1) * (double)(uint32_t)(abs(att3));
	att4 = att4 + ((att4>=0)?-1:1) * (double)(uint32_t)(abs(att4));

	bbatt[0] = att1 + (att2 - att1) * (freq - d_freq) / (u_freq - d_freq);	//拟合
	bbatt[1] = att3 + (att4 - att3) * (freq - d_freq) / (u_freq - d_freq);
	bb_att = bbatt[0] + (bbatt[1] - bbatt[0]) * (power_dBm - u_dBm) / (d_dBm - u_dBm);
	bb_att *= 100.0;

	off[0] = off1 + (off2 - off1) * (freq -d_freq) / (u_freq - d_freq);
	off[1] = off3 + (off4 - off3) * (freq -d_freq) / (u_freq - d_freq);
	cal_offset    = off[0] + (off[1] - off[0]) * (power_dBm - u_dBm) / (d_dBm - u_dBm);
	Log.stdprintf("cal_offset = %f\n",cal_offset);

	return 0;
}


int32_t multichSG::Set_Rx_Freq(uint64_t freq)
{
	// 	REG_DECLARE(0x0025);
	// 	ad9361_k7r(0x0025);
	REG_DECLARE(0x00e4);
	ad9361_k7r(0x00e4);
	int Rx_Select[5] = {0,3,2,1,7};
	int rs;
	uint64_t ext_lo,ad9361_lo;
	if	   ( freq <= 1500e6 ) rs = Rx_Select[0];
	else if( freq <= 2200e6 ) rs = Rx_Select[1];
	else if( freq <= 2700e6 ) rs = Rx_Select[2];
	else if( freq <= 3800e6 ) rs = Rx_Select[3];
	else					  rs = Rx_Select[4];

	Status_Check(OpenBoard(slot,false));
	if(freq < 1500e6) {
		if(freq % 10000000 == 0) {						//整数频率
			Status_Check(RF.Set_Rx_Rf_Lo(freq+1500e6));
			if (AD9361.set_rx_lo_freq(1510e6) < 0) {				//中频10MHz
				set_last_error("RF:Set Rx Freq Error! %s",AD9361.get_last_error());
				return -1;
			}
		}
		else {
			ext_lo    = (int)(freq / 10e6) * 10e6 + 1500e6;	//小数频率
			ad9361_lo = 1510e6 - freq % 10000000;
			Status_Check(RF.Set_Rx_Rf_Lo(ext_lo));
			if (AD9361.set_rx_lo_freq(ad9361_lo)) {
				set_last_error("RF:Set Rx Freq Error! %s",AD9361.get_last_error());
				return -1;
			}
		}
	}
	else
		if (AD9361.set_rx_lo_freq(freq + 10e6)) {				//中频10MHz
			set_last_error("RF:Set Rx Freq Error! %s",AD9361.get_last_error());
			return -1;
		}
		// 	REG(0x0025).rx_select = rs;
		// 	ad9361_k7w(0x0025);
		REG(0x00e4).rx_select = rs;
		ad9361_k7w(0x00e4);
		return 0;
}

int32_t multichSG::Set_Rx(uint64_t Freq,int ref_level,double &power)
{
	double adc_power,total_gain,calvalue = 0;
	int gain_index = (35 - ref_level) / 5;
	int iq;

	Get_Rx(Freq,ref_level,total_gain,calvalue);
	Log.stdprintf("ad9361_gain = %d\n",ad9361_gain[gain_index]);

	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error(RF.Rx_Level_Select(ref_level >= -5),"Set Rx Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(RF.Set_Rx_Rf_Att(((ref_level >= -5)?24:0)),"Set Rx Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(AD9361.set_rx_rf_gain(0,ad9361_gain[gain_index]),"Set Rx Error! %s",AD9361.get_last_error()));
	Status_Check(set_last_error(Set_Rx_Freq(Freq),"Set Rx Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(RF.Get_Rx_Power(adc_power,iq),"Set Rx Error! %s",RF.get_last_error()));
	Log.stdprintf("iq = %d\n",iq);
	power = 10.0 * log10(iq / 1000000.0) - total_gain + calvalue;
	return 0;
}

int32_t multichSG::Get_Rx(uint64_t Freq,int ref_level,double &total_gain,double &cal_value)
{
	double cal_value_u,cal_value_d;

	int freq   = Freq / 1e6;
	int freq_u = freq / 50 * 50;
	int freq_d = freq_u + 50;
	
	int ref_index  = 35 - ref_level;
	int gain_index = (35 - ref_level) / 5;
	int fre_index  = (freq - 700) / 50;

	total_gain	= (double)Total_gain[gain_index];
	cal_value_u = (double)rx_cal_table[ref_index * 43 + fre_index];
	cal_value_d = (double)rx_cal_table[ref_index * 43 + fre_index + 1];
	cal_value   = cal_value_u + (cal_value_d - cal_value_u) * ((freq - freq_u) / 50.0);

	return 0;
}

int32_t	multichSG::Read_From_Flash(const char *filepath,float *data,int size1D,int size2D)
{
	FILE *fp = fopen(filepath, "r");
	if (fp) {
		for (int i = 0; i < size1D; i++) {
			for (int j = 0; j < size2D; j++) {
				fscanf(fp, "%f ", &data[i * size2D + j]);
			}
		}
		fclose(fp);
	}
	return 0;
}

int32_t multichSG::set_last_error(int32_t func,const char *format, ...)
{
	if (func == 0)
		return func;
	va_list ap;

	memset(m_last_error, 0, 1024);

	va_start(ap, format);
	vsprintf(m_last_error, format, ap);
	va_end(ap);
	return func;
}

void multichSG::set_last_error(const char *format, ...)
{
	va_list ap;

	memset(m_last_error, 0, 512);

	va_start(ap, format);
	vsprintf(m_last_error, format, ap);
	va_end(ap);
}

char *multichSG::get_last_error()
{
	return m_last_error;
}