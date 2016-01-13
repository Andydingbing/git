#include "StdAfx.h"

#include "RegisterDefine.h"
#include "ArbReader.h"
#include "rf.h"
#include "log.h"
#include "math.h"

ViRsrc instrdesc[100] =			//PCI资源名称
{
	"PXI21::0::INSTR",			//slot0			//TX1	
	"PXI5::0::INSTR",			//slot1			//TX2	
	"PXI20::0::INSTR",			//slot2			//TX3
	"PXI6::0::INSTR",			//slot3			//TX4
	"PXI7::0::INSTR",			//slot4			//TX5
	"PXI8::0::INSTR",			//slot5			//TX6
	"PXI9::0::INSTR",			//slot6			//TX Reserved
	"PXI0::0::INSTR",			//slot7			//TX Reserved
	"PXI3::0::INSTR",			//slot8			//RX1
	"PXI17::0::INSTR",			//slot9			//Reserved
};

char rf::m_last_error[512] = {0};

rf::rf()
{
	tx_memory_status = false;
	rx_memory_status = false;
}

rf &rf::Instance()
{
	static rf Rf;
	return Rf;
}

int32_t rf::Get_FPGA_Version(uint32_t &ver)
{
	REG_DECLARE(0x0000);	//ver = d7---d15
	ad9361_k7r(0x0000);
//	ver = REG(0x0000).ver;
	ver = r0x0000.u32;
	return 0;
}

const char *rf::Get_Driver_Version()
{
	extern const char *multichSG_version;
	return multichSG_version;
}

int32_t rf::Fpga_Reset()
{
	REG_DECLARE(0x0000);	//d0 OP
	OP(0x0000);
	return 0;
}

int32_t	rf::Get_Tx_Rx_Board(TRX &trx)
{
	uint32_t ver;
	int status = Get_FPGA_Version(ver);
	if(!status) {
		if      (ver == 0x15121810) trx = Tx;
		else if (ver == 0x15121410) trx = Rx;
		else {
			set_last_error("RF:FPGA Version:%#x Error!(Tx:0x15121810	Rx:0x15121410)",ver);
			return -1;
		};
	}
	return 0;
}

int32_t rf::PCIE_Open(const int &slot)
{
	this->slot = slot;
	int status = PCIE_Core.ibfind(instrdesc[slot]);
	if (status != 0)
		set_last_error(PCIE_Core.get_last_error());
	return status;
}

int32_t rf::Turn_TxCh_On()
{
	REG_DECLARE(0x0028);
	REG(0x0028).Flash_CS = 1;
	REG(0x0028).Flash_wp = 0;
	REG(0x0028).Tx_Power_Off = 1;
	ad9361_k7w(0x0028);
	return 0;
}

int32_t rf::Tx_DMA_Memory_Alloc()
{
	if (tx_memory_status)
		return 0;
	REG_DECLARE(0x0007);
	static unsigned long defaultValue[tx_dma_mem_size] = { 0x423a35c7 };		//默认值是10进制的 10位"1"
	for (int i = 0; i < tx_dma_mem_size; i++)
		defaultValue[i] = 0x423a35c7;
	if (tx_dma_memory.mem_alloc(tx_dma_mem_size * 4)) {
		Log.stdprintf("tx mem alloc fail...\n");
		set_last_error("RF:Tx Dma Memory Alloc 4M Fail! DMA Detail:%s",tx_dma_memory.get_last_error());
		return -1;
	}
// 	if (tx_dma_memory.data_write32(defaultValue,tx_dma_mem_size)) {
// 		return E_Tx_Mem_Data_Write;
// 	}
	REG(0x0007).adr = (((uint64_t)tx_dma_memory.m_phy_addr & 0x00000003ffffffff) >> 4);
	ad9361_k7w(0x0007);
	tx_memory_status = true;
	return 0;
}

int32_t rf::Tx_DMA_Memory_Release()
{
	if (!tx_memory_status)
		return 0;
	if (tx_dma_memory.mem_release()) {
		set_last_error("RF:Tx Dma Memory Release Fail! DMA Detail:%s",tx_dma_memory.get_last_error());
		return -1;
	}
	tx_memory_status = false;
	return 0;
}

int32_t rf::DMA_Start()
{
	REG_DECLARE(0x0001);
	ad9361_k7OP(0x0001, fpga_rd_op);	//d0 OP
	return 0;
}

int32_t rf::Fpga_DMA_Read(uint32_t *pbuf,uint32_t length,float *time)
{
	REG_DECLARE(0x0007);
	REG_DECLARE(0x0008);
	REG_DECLARE(0x0009);
	REG_DECLARE(0x0020);
	REG_DECLARE(0x0021);

	DWORD DMA_STime = GetTickCount();					//开始时间
	DWORD DMA_ETime = 0;								//结束时间
	float DMA_CTime = 0;								//DMA时间
	int32_t DMA_Status = 0;

	if ((DMA_Status = tx_dma_memory.data_write32(pbuf,length)) < 0) {
		set_last_error("RF:Tx Dma Data Write32 Fail! Dma Detail:%s",tx_dma_memory.get_last_error());
		Log.stdprintf("data_write32(%p, %u) return %d\n", pbuf, length, DMA_Status);
		Log.stdprintf("get last error:%s\n",tx_dma_memory.get_last_error());
		return -1;
	}

	REG(0x0008).tlp_size = 16;
	ad9361_k7w(0x0008);

	REG(0x0009).tlp_count = length / 16;
	ad9361_k7w(0x0009);

// 	REG(0x0007).adr = (((uint64_t)tx_dma_memory.m_phy_addr & 0x00000003ffffffff) >> 4);
// 	ad9361_k7w(0x0007);

	Status_Check(Fpga_Reset());
	Status_Check(DMA_Start());

	int ret = 0;
	/*//等中断 //目前没有中断
	for (int i = 0; i < 3; i++) {
		ret = WaitForSingleObject(g_event, 1000);
		stdprintf("WaitForSingleObject return %d\n", ret);

		ad9361_k7r(0x0021);
		stdprintf("REG(0x0021) = %d\n", REG(0x0021).dma_rd_counter);
		if (ret != WAIT_TIMEOUT)
			break;
	}*/

	unsigned int dmacount[5] = {0};
	while(1) {											//Make Sure DMA Complete
		for (int i=0; i<5; i++) {
			ad9361_k7r(0x0021);
			dmacount[i] = REG(0x0021).dma_rd_counter;
			Log.stdprintf("REG(0x0021) = %d\n",dmacount[i]);
			if (dmacount[i] == length) {
				ret = 1;
				break;
			}
		}
		if (ret == 1) break;
		else if (dmacount[4] == dmacount[3]) {			//Sample数必须为16的整数
			set_last_error("RF:Tx Dma Sample Format Wrong!");
			Log.stdprintf("Sample Format Wrong!\n");
			return -1;
		}
	}

	DMA_ETime = GetTickCount();							
	DMA_CTime = (float)(DMA_ETime - DMA_STime)/1000;	//计算DMA时间
	time = &DMA_CTime;

	ad9361_k7r(0x0020);									//fpga计算的DMA时间
	Log.stdprintf("Timer = %lu\n", REG(0x0020).dma_rd_timer);
	return 0;
}

//单件有bug
int32_t rf::SetArb_Load(char * filepath)
{
	if (ARB.Load(filepath)) 
		return -1;
	if (ARB.GetSegments() != 1)			//目前仅支持单段
		return -1;
	return 0;
}

int32_t rf::SetArb_Segments(uint16_t seg)
{
	REG_DECLARE(0x1005);
	REG(0x1005).seg = seg;
	ad9361_k7w(0x1005);
	return 0;
}

int32_t rf::SetArb_Param(uint32_t addsamp,uint32_t cycl,uint32_t repmo)
{
	REG_DECLARE(0x1002);
	REG(0x1002).addition    = addsamp;			//Additional Samples = 0
	REG(0x1002).cycles      = cycl;				//Cycles =1
	REG(0x1002).repetition  = repmo;			//播放方式:Continuous
	ad9361_k7w(0x1002);
	return 0;
}

int32_t rf::SetArb_Trigger(bool reTrigger, bool autoStart, uint32_t src, uint32_t trigDelay)
{
	REG_DECLARE(0x100a);
	REG(0x100a).retrig      = reTrigger;		//ReTrigger = 0 
	REG(0x100a).delay       = trigDelay;		//Trigger Delay = 0
	REG(0x100a).autostart   = autoStart;		//AutoStart = 1
	REG(0x100a).src         = src;				//触发源：Manual
	ad9361_k7w(0x100a);
	return 0;
}

int32_t rf::SetArb_MultiSegMode(uint32_t msts, uint32_t msrm)
{
	REG_DECLARE(0x100c);
	REG(0x100c).ms_trig     = msts;				//多段Trigger：Manual
	REG(0x100c).ms_rm       = msrm;				//多段重复方式：Auto
	ad9361_k7w(0x100c);
	return 0;
}

int32_t rf::SetArb_FreqOffset(uint32_t freqMHz)
{
	REG_DECLARE(0x1010);
	REG_DECLARE(0x1011);
	REG(0x1011).offset      = freqMHz;			//频偏 = 0	
	ad9361_k7w(0x1011);
	OP(0x1010);
	return 0;
}

int32_t rf::Arb(char* FILEPATH,
		 
		 unsigned Additional_Samples,
		 unsigned Cycles,
		 unsigned repMode,
		 
		 unsigned ReTrigger,
		 unsigned Trigger_Delay,
		 unsigned AutoStart,
		 unsigned triSrc,
		 
		 unsigned multiSegTrigSrc,
		 unsigned multiSegRepMode,
		 unsigned FreqOffset)
{	
	uint32_t totalsample = 0;
	uint32_t sample_tr = 0;
	uint32_t sample_left = 1;
	uint32_t DDRAdr = 0;
	uint32_t samples;
	uint32_t DMA_count = 0;					//DMA计数，每次4MB
	const uint32_t one_sample = 1024 *1024;	//每次DMA 1M Sample = 4MB

	ArbReader arb;

	REG_DECLARE(0x1003);
	REG_DECLARE(0x1004);
	REG_DECLARE(0x1006);
	REG_DECLARE(0x1007);

	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error((RF.Get_Tx_Memory_Status() == false),"Arb Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(Arb_FPGA_CW(0),"Arb FPGA CW Error!"));
	Status_Check(set_last_error(Arb_Stop(),"Arb Stop Error!"));
// 	if (RF.SetArb_Load(FILEPATH))
// 		return -1;
	Status_Check(set_last_error(arb.Load(FILEPATH),"Arb Load Error!"));
 	Status_Check(set_last_error(RF.SetArb_Segments(arb.GetSegments()),"Arb Error! %s",RF.get_last_error()));
	Status_Check(set_last_error((arb.GetSegments() != 1),"Arb File Segments > 1"));			//目前仅支持单段

	uint32_t **p = new uint32_t*[arb.GetSegments()];	//申请segment段的内存,暂只支持单段;
														//每段存放每段的Data(长度未知)
	if (p == NULL)
		return -ENOMEM;
											
	//DMA
	const ArbReader::SegHeader_t *seg_info;
	for (int i = 0; i < arb.GetSegments() ;i++)
	{
		seg_info			 = arb.GetSegInfo(i);
		totalsample			+= seg_info->Samples;		//总的Sample数
		arb_level_offset     = seg_info->LevelOffset;
		double     sr		 = seg_info->SampleRate;	//采样率
		uint32_t   interFltr = 2;						//内插系数 = 1
		if (i == 0) {									//单段的采样率
			Status_Check(set_last_error(AD9361.set_tx_sampling_freq(sr),"Arb Error! %s",AD9361.get_last_error()));
			Sleep(10);
		}
/*		double     sr		 = 245.76e6;
		uint32_t   interFltr = sr / seg_info->SampleRate;*/

		union as_reg_t as_reg[18] = { 0 };
		AS_REG(0).sample		= seg_info->Samples;
		AS_REG(1).seg_adr		= 0;
		AS_REG(2).inter_fltr	= interFltr;
		AS_REG(16).sr			= sr;
		for (int j = 0;j < 18;j++) {
			REG(0x1003).adr	    = j;
			REG(0x1003).seg	    = i;
			REG(0x1004).data    = as_reg[j].data;
			ad9361_k7w(0x1004);
			OP(0x1003);
		}
	}
	sample_left = totalsample;

	for (int i=0 ;i <arb.GetSegments() ;i++)
		p[i] = (uint32_t*)(arb.GetData(i));

	while(sample_left > 0)
	{
		DMA_count++;									//DMA计数
		if (sample_left > one_sample)	
			samples = one_sample;
		else 
			samples = sample_left;

		DDRAdr				= sample_tr >> 1;
		REG(0x1006).samples = samples;
		ad9361_k7w(0x1006);
		REG(0x1007).adr     = DDRAdr;
		ad9361_k7w(0x1007);

 		Log.stdprintf("\n");
 		Log.stdprintf("DMA %d Ready: samples: %d\n",DMA_count,samples);
		set_last_error(RF.Fpga_DMA_Read(&p[0][sample_tr],samples,NULL),"Arb Error! %s",RF.get_last_error());
		Log.stdprintf("DMA %d Complete\n",DMA_count);

		sample_tr	+= samples;
		sample_left  = totalsample-sample_tr;
	}
	delete[] p;
	Log.stdprintf("ALL DMA Complete\n");
	
	Status_Check(set_last_error(RF.SetArb_Param(Additional_Samples,Cycles,repMode),"Arb Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(RF.SetArb_Trigger(ReTrigger,AutoStart,triSrc,Trigger_Delay),"Arb Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(RF.SetArb_MultiSegMode(multiSegTrigSrc,multiSegRepMode),"Arb Error! RF %s",RF.get_last_error()));
	Status_Check(set_last_error(RF.SetArb_FreqOffset(FreqOffset),"Arb Error! %s",RF.get_last_error()));
	Log.stdprintf("Arb Complete\n");

	return 0;
}

int32_t rf::Arb_FPGA_CW(bool fpga_cw,double freq)
{
	REG_DECLARE(0x00e2);
	REG_DECLARE(0x00e3);
	uint32_t samprate;

	Status_Check(OpenBoard(slot,false));
	AD9361.get_rx_sampling_freq(&samprate);
	REG(0x00e3).fpga_cw_freqMHz = (int32_t)(freq * 4294967296.0 / samprate); 
	ad9361_k7w(0x00e3);
	REG(0x00e2).fpga_cw = fpga_cw;
	ad9361_k7w(0x00e2);
	return 0;
}

int32_t rf::Arb_Start()
{
	REG_DECLARE(0x1009);

	Status_Check(OpenBoard(slot,false));
	REG(0x1009).abort		= 0;
	REG(0x1009).en			= 1;
	REG(0x1009).apc			= 0;
	ad9361_k7w(0x1009);
	return 0;
}

int32_t rf::Arb_Stop()
{
	REG_DECLARE(0x1009);

	Status_Check(OpenBoard(slot,false));
	REG(0x1009).abort		= 1;
	REG(0x1009).en			= 0;
	ad9361_k7w(0x1009);
	return 0;
}

int32_t rf::Set_Tx_Rf_Att(uint32_t att)
{
	int io_ext_att = (int)(att / 4) * 4;				//实际值是0,4,8,12,16,20,24,28dB
	if (io_ext_att >= 28)
		io_ext_att = 28;
	Status_Check(Set_Tx_IO_Ext(GPA,io_ext_att / 4));
	return 0;
}

int32_t rf::Set_Tx_IO_Ext(GPIO gpio,uint32_t gp_value)		
{
	uint32_t Addr = 0;
	int32_t status = 0;
	Addr = gpio?0x1:0x0;
	if((status = ioexte_spi_write(Addr,0)) != 0) {			//配置成输出口
		set_last_error("RF:Set Tx IO Ext OUTPUT Error!");
		return status;			
	}
	Addr = gpio?0x13:0x12;
	if((status = ioexte_spi_write(Addr,gp_value)) != 0)	{	//配置GP端口
		set_last_error("RF:Set Tx IO Ext SETGP Error!");
		return status;			
	}
	return 0;
}

int32_t rf::Set_Tx_Baseband_Att(double att)
{
	REG_DECLARE(0x002d);
	REG(0x002d).baseband_att = att * 500.0;
	ad9361_k7w(0x002d);
	return 0;
}

int32_t rf::Get_Tx_Baseband_Att(double &att)
{
	REG_DECLARE(0x002d);
	ad9361_k7r(0x002d);
	att = (double)(REG(0x002d).baseband_att) / 500.0;
	return 0;
}

bool rf::Get_Tx_Memory_Status()
{
	if (!tx_memory_status)
		set_last_error("RF:Tx Memory Not Alloc");
	return tx_memory_status;
}

int32_t rf::Rx_CAP_Memory_Alloc()
{
	if (rx_memory_status)
		return 0;
	REG_DECLARE(0x0016);
	REG_DECLARE(0x0017);
	uint64_t addr_to_FPGA[8] = {0};									//每块内存写8次段地址
	static DWORD defaultValue[R_BlockSamples] = { 0x423a35c7 };		//默认值是10进制的 10位"1"
	for (int i = 0; i < R_BlockSamples; i++)
		defaultValue[i] = 0x423a35c7;
	for (int i=0;i<R_Blocks;i++) {
		if(rx_cap_memory[i].mem_alloc(R_BlockSamples * 4) < 0){
			set_last_error("RF:RX Cap Memory Alloc @Block %d Error! memory detail:%s",i,rx_cap_memory[i],get_last_error());
			Log.stdprintf("Rx Cap Memory Alloc @Block %d Error!, %s\n",i,rx_cap_memory[i].get_last_error());
			return -1;
		}
		if(rx_cap_memory[i].data_write32(defaultValue, R_BlockSamples)) {
			set_last_error("RF:Rx Cap Memory data write32 @Block %d Error! memory detail:%s",i,rx_cap_memory[i].get_last_error());
			return -1;
		}

		for (int j=0;j<8;j++){									//每块内存的8小段的内存地址
			addr_to_FPGA[j] = (uint64_t)rx_cap_memory[i].m_phy_addr + 128 * 1024 * 4 * j;
			addr_to_FPGA[j] = ((addr_to_FPGA[j] & 0x00000003ffffffff) >> 2);
			REG(0x0017).adr = (uint32_t)addr_to_FPGA[j];
			ad9361_k7w(0x0017);
			REG(0x0016).data= 0;
			ad9361_k7w(0x0016);
			REG(0x0016).data=0x80000000 | (i*8+j);
			ad9361_k7w(0x0016);
		}
	}
	rx_memory_status = true;
	return 0;
}

int32_t rf::Rx_CAP_Memory_Release()
{
	if (!rx_memory_status)
		return 0;
	for (int i=0;i<R_Blocks;i++) {	
		if (rx_cap_memory[i].mem_release() == -1) {	
			set_last_error("RF:Rx Cap Memory Release @Block %d Error! memory detail:%s",i,rx_cap_memory[i].get_last_error());
			Log.stdprintf("Rx Cap Memory Release @Block %d Fail!\n",i);
			return -1;
		}
	}	
	rx_memory_status = false;
	return 0;
}

int32_t rf::Rx_Capture(int16_t *			I,
							  int16_t *			Q,
							  double			Threshold_dBm,
							  uint32_t		samples,
							  char *			filepath,
							  bool			SaveToFile,

							  IQCaptureSrcs	Trig_Src,
							  bool			B_Posedge,
							  uint32_t		Trig_Offset,
							  uint32_t		Cap_Offset,
							  uint32_t		Trig_Timeout,
							  uint32_t		Trig_Gap)
{
	REG_DECLARE(0x0014);
	REG_DECLARE(0x1060);
	REG_DECLARE(0x1061);
	REG_DECLARE(0x1062);
	REG_DECLARE(0x1063);
	REG_DECLARE(0x1066);
	REG_DECLARE(0x1067);
	REG_DECLARE(0x1068);
	REG_DECLARE(0x1069);

	Status_Check(OpenBoard(slot,false));
	Status_Check(set_last_error((RF.Get_Rx_Memory_Status() == false),"Rx Cap Memory Error! %s",RF.get_last_error()));

	Status_Check(set_last_error(RF.IQ_Capture_Abort(),"Rx Cap Error! %s",RF.get_last_error()));
	Status_Check(set_last_error(RF.Fpga_Reset(),"Rx Cap Error! %s",RF.get_last_error()));
	uint32_t RxC_Status = 0;

	//Register Configure
	REG(0x1062).iqcap_src_sel		= Trig_Src;					//触发源
	REG(0x1062).iqcap_src_edge		= B_Posedge;				//上升沿/下降沿
	ad9361_k7w(0x1062);
	REG(0x1060).iqcap_trig_offset	= Trig_Offset;				//Trigger Offset
	ad9361_k7w(0x1060);
	if (Cap_Offset % 16 !=0){
		set_last_error("capOffset Wrong!");
		Log.stdprintf("capOffset Wrong!\n");
		return -1;									//Cap Offset error
	}
	REG(0x1061).iqcap_cap_offset	= Cap_Offset;				//Cap Offset
	ad9361_k7w(0x1061);
	REG(0x1066).samples				= samples;					//采样点数
	ad9361_k7w(0x1066);
	REG(0x1063).iqcap_timeout		= Trig_Timeout * 62.5 + 0.5;//超时点数
	ad9361_k7w(0x1063);
	REG(0x1067).gap					= Trig_Gap *62.5 + 0.5;		
	ad9361_k7w(0x1067);
	REG(0x1068).threshold			= (unsigned int)(pow(10.0,(Threshold_dBm / 10.0)) * 1000000);	
	ad9361_k7w(0x1068);

	Status_Check(set_last_error(RF.IQ_Capture_Start(),"Rx Cap Start Error! %s",RF.get_last_error()));

	LARGE_INTEGER freq;											//计时	
	LARGE_INTEGER star_time;
	LARGE_INTEGER stop_time;
	double        time;

	QueryPerformanceFrequency(&freq);							//时钟频率
	QueryPerformanceCounter(&star_time);						//开始时间

	uint32_t capturedsamples[5] = {0};
	uint32_t timeoutflag = 0;
	uint32_t ret = 0;

	while (1) {	
		//		ad9361_k7r(0x1063);										//暂不可读
		//		timeoutflag = REG(0x1063).iqcap_tmo;					//超时标志,1为超时，0为不超时
		//		Log.stdprintf("REG(0x1063).time_out_flag:%d\n",timeoutflag);

		QueryPerformanceCounter(&stop_time);
		time = (double)(stop_time.QuadPart - star_time.QuadPart) / (double)(freq.QuadPart);
		Log.stdprintf("time = %f\n",time);
		if (time * 1e6 >= Trig_Timeout)
			timeoutflag = 1;									//超时
		Log.stdprintf("timeoutflag = %d\n",timeoutflag);
		for (int i=0;i<5;i++) {									//连续读0x0014,上行捕获的Sample数
			ad9361_k7r(0x0014);
			Log.stdprintf("REG(0x0014):%d\n",REG(0x0014).samples);
			capturedsamples[i] = REG(0x0014).samples;
		}
		if (timeoutflag == 0) {									//不超时
			if (capturedsamples[4] == samples) {				//超时时间内捕获到给定的Sample数，成功
				Log.stdprintf("DMA Trans Complete\n");
				break;
			}
		}
		if (timeoutflag == 1) {									//超时	
			if (capturedsamples[0]  < capturedsamples[1])
				continue;
			if (capturedsamples[4] == samples) {				//超时时间外捕获到给定的Sample数，成功
				Log.stdprintf("DMA Trans Complete\n");
				break;
			}
			if (capturedsamples[4] == capturedsamples[3] && capturedsamples[4] < samples) {							
				set_last_error("Trigger Fail!");				//超时时间内没有捕获到给定的Sample数，失败
				Log.stdprintf(get_last_error());
				return -1;	
			}
		}
	}

	Status_Check(set_last_error(RF.Get_IQ_To_Buf(I,Q,samples),"Rx Cap IQ To Buf Error! %s",RF.get_last_error()));
	if (SaveToFile) {
		Status_Check(set_last_error(RF.Get_IQ_To_File(filepath,samples),"Rx Cap IQ To File Error! %s",RF.get_last_error()));
	}
	return 0;
}

int32_t rf::IQ_Capture_Start()
{	
	REG_DECLARE(0x1064);
	REG_DECLARE(0x1065);
	REG_DECLARE(0x106f);
	double RxFs;
  //ad9361_k7r(0x1069);
  //RxFs							= 122.88 / REG(0x1069).rx_dec_num;
	RxFs							= 122.88 / 1;
	REG(0x106f).cap_tlp_count		= ((RxFs * 1e3) /32)/2;
	ad9361_k7w(0x106f);
	REG(0x1065).abort				= 0;
	ad9361_k7w(0x1065);
	OP(0x1064);	
	return 0;
}

int32_t rf::IQ_Capture_Abort()
{
	REG_DECLARE(0x1065);
	REG(0x1065).abort = 1;
	ad9361_k7w(0x1065);
	return 0;
}

int32_t rf::Get_IQ_To_Buf(int16_t *I,int16_t *Q,uint32_t samples)
{
	int totalBlock		 = samples / R_BlockSamples;	
	int LastBlockSamples = samples % R_BlockSamples;			//最后一块内存Sample数
	if (LastBlockSamples!= 0)
		totalBlock++;											//总内存块数(每块1M Sample)

	int		sampleLeft = samples;								//剩余的Sample数
	int		readSize;											//每次写的Sample数
	int		index;												//当前数据总的index
	int		sign_i = 0,sign_q = 0;								//IQ符号位
	double	isum = 0, qsum = 0;

	for (int i = 0; i < totalBlock; i++) {
		readSize = (sampleLeft > R_BlockSamples ? R_BlockSamples : sampleLeft);
		if(rx_cap_memory[i].data_read32(aSamples,readSize)) {
			set_last_error("RF:Rx Cap Memory data read32 @Block %d Error! memory detail:%s",i,rx_cap_memory[i].get_last_error());
			return -1;
		}
		for (int j = 0; j < readSize; j++) {
			index = i * R_BlockSamples + j;
			I[index] = aSamples[j];
			sign_i = aSamples[j] & 0x8000;
			if (sign_i) I[index] = (~I[j]+1)*(-1);

			Q[index] = (aSamples[j] >> 16);
			sign_q = ((aSamples[j] >> 16) & 0x8000);
			if (sign_q) Q[index] = (~Q[j]+1)*(-1);
			sampleLeft -= readSize;

// 			isum += pow((double)I[index], 2);
// 			qsum += pow((double)Q[index], 2);
		}
	}

// 	double iq = (isum + qsum)/Samples;
// 	double power = 10 * log10(iq / 1000000.0);
// 	stdprintf("power = %f, %f\n",power, iq);
	return 0;
}

int32_t	rf::Get_IQ_To_File(char * filepath,uint32_t samples)
{
	int totalBlock		 = samples / R_BlockSamples;	
	int LastBlockSamples = samples % R_BlockSamples;			//最后一块内存Sample数
	if (LastBlockSamples!= 0)
		totalBlock++;											//总内存块数(每块1M Sample)

	int sampleLeft = samples;									//剩余的Sample数
	int readSize;												//每次写的Sample数
	int	sign_i = 0,sign_q = 0;									//IQ符号位

	FILE *fp = fopen(filepath,"w");
	if (fp == NULL) {
		set_last_error("Cannot Open File!");
		Log.stdprintf("Cannot Open File!\n");
		return -1;
	}

	for (int i = 0; i < totalBlock; i++) {
		readSize = (sampleLeft > R_BlockSamples ? R_BlockSamples : sampleLeft);
		if(rx_cap_memory[i].data_read32(aSamples, readSize)) {
			set_last_error("RF:Rx Cap Memory data read32 @Block %d Error! memory detail:%s",i,rx_cap_memory[i].get_last_error());
			return -1;
		}
		for (int j = 0; j < readSize; j++) {
			int16_t i = aSamples[j];
			sign_i = aSamples[j] & 0x8000;
			if (sign_i) i = (~i+1)*(-1);

			int16_t q = (aSamples[j] >> 16);
			sign_q = aSamples[j] & 0x8000;
			if (sign_q) q = (~q+1)*(-1);
			fprintf(fp, "%d, %d\n", i, q);
		}
		sampleLeft -= readSize;
	}

	fclose(fp);
	return 0;
}

int32_t rf::Rx_Level_Select(uint32_t level)
{
// 	REG_DECLARE(0x0025);
// 	ad9361_k7r(0x0025);
// 	REG(0x0025).level_select = level;
// 	ad9361_k7w(0x0025);
// 	return 0;
	REG_DECLARE(0x00e4);
	ad9361_k7r(0x00e4);
	REG(0x00e4).level_select = level;
	ad9361_k7w(0x00e4);
	return 0;
}

int32_t rf::Set_Rx_Rf_Att(uint32_t att)
{
// 	REG_DECLARE(0x0025);
// 	ad9361_k7r(0x0025);
// 	REG(0x0025).rx_43503_16 = (att/16)?1:0;
// 	REG(0x0025).rx_43503_8  = ((att/8)%2)?1:0;
// 	ad9361_k7w(0x0025);
// 	return 0;
	REG_DECLARE(0x00e4);
	ad9361_k7r(0x00e4);
	REG(0x00e4).rx_43503_16 = (att/16)?1:0;
	REG(0x00e4).rx_43503_8  = ((att/8)%2)?1:0;
	ad9361_k7w(0x00e4);
	return 0;
}

int32_t rf::Get_Rx_Power(double &pow,int &iq)
{
	REG_DECLARE(0x002b);
	REG_DECLARE(0x002c);
	ad9361_k7r(0x002c);
	int get_rx_p_count = 0;
	while (REG(0x002c).rx_data_power_en == 0) {
		ad9361_k7r(0x002c);
		get_rx_p_count ++;
		if (get_rx_p_count > 100) {
			set_last_error("RF:Get Rx Power Overtime!");
			return -1;
		}
	}
	ad9361_k7r(0x002b);

	iq = REG(0x002b).ddc_rd;
	pow = 10 * log10((REG(0x002b).ddc_rd) / 1000000.0);	//ADC基准值定为1000000
	return 0;
}

int32_t rf::Set_Rx_Rf_Lo(uint32_t LoFreq)
{
	REG_DECLARE(0x0035);
	REG_DECLARE(0x0036);
	LoFreq = LoFreq / 1000000;
	REG(0x0036).lo_freq = LoFreq;
	ad9361_k7w(0x0036);
	OP(0x0035);
	return 0;
}

bool rf::Get_Rx_Memory_Status()
{
	return rx_memory_status;
}

int32_t rf::Setup_DDC()
{
	REG_DECLARE(0x002a);
	REG_DECLARE(0x0029);
	REG(0x002a).ddc = 10.0 * 4294967296 / 122.88;		//中频10MHz * 2^32 / 122.88
	ad9361_k7w(0x002a);
	OP(0x0029);
	return 0;
}

int32_t rf::Tx_Flash_Write(float *data,int length)
{
	char tx_flash_path[512];
	sprintf(tx_flash_path,"c:\\MultiCHSG\\tx_cal_data@slot%d.cal",slot);
	FILE *fp = fopen(tx_flash_path, "wb+");
	if (fp) {
		fwrite(data, sizeof(float),length,fp);
		fclose(fp);
	}
	return 0;
}

int32_t rf::Tx_Flash_Read(float *data,int length)
{
	char tx_flash_path[512];
	sprintf(tx_flash_path,"c:\\MultiCHSG\\tx_cal_data@slot%d.cal",slot);
	FILE *fp = fopen(tx_flash_path, "rb");
	if (fp) {
		fread(data, sizeof(float),length,fp);
		fclose(fp);
	}
	return 0;
}

void rf::set_last_error(const char *format, ...)
{
	va_list ap;

	memset(m_last_error, 0, 512);

	va_start(ap, format);
	vsprintf(m_last_error, format, ap);
	va_end(ap);
}

char *rf::get_last_error()
{
	return m_last_error;
}

int32_t str_to_freq(char *str_freq,uint64_t &freq)
{
	char *unit;

	if (strstr(str_freq,"e") != NULL || strstr(str_freq,"E") != NULL) {
		sscanf(str_freq,"e%",freq);
		Log.stdprintf("%llu\n",freq);
		return 0;
	}

	freq = (uint64_t)atof(str_freq);
	unit = str_freq + strlen(str_freq) - 1;
	if (!strcmp(unit,"K") || !(strcmp(unit,"k"))) {freq *= 1e3; return 0;}
	if (!strcmp(unit,"M") || !(strcmp(unit,"m"))) {freq *= 1e6; return 0;}
	if (!strcmp(unit,"G") || !(strcmp(unit,"g"))) {freq *= 1e9; return 0;}

	return 0;
}

int32_t str_to_freq(CString str_freq,uint64_t &freq)
{
	char *p;
	p = (LPSTR)(LPCTSTR)str_freq;
	str_to_freq(p,freq);
	return 0;
}