#ifndef _REG_DEFINE_H_
#define _REG_DEFINE_H_

#include "stdint.h"
#include "visa_driver.h"
#include "log.h"

#define REG_DEF_S(reg_addr)      \
    union reg_##reg_addr {       \
        uint32_t u32;            \
        struct reg_t {

#define REG_DEF_E				 \
    } r;						 \
};

#define REG_DECLARE(reg_addr) union reg_##reg_addr r##reg_addr = { 0 };
#define REG(reg_addr) r##reg_addr.r

#define CLEAR(reg_addr) r##reg_addr.u32 = 0;

#define OP(reg_addr) ad9361_k7OP(reg_addr,op)

#define ad9361_k7OP(reg_addr,reg_member)			\
	do {											\
	r##reg_addr.r.reg_member = 0;					\
	ad9361_k7w(reg_addr);							\
	r##reg_addr.r.reg_member = 1;					\
	ad9361_k7w(reg_addr);							\
	r##reg_addr.r.reg_member = 0;					\
	ad9361_k7w(reg_addr);							\
	}while (0);

#define ad9361_k7w(reg_addr)													\
	do {																		\
		if (PCIE_Core.WriteWordBar1(uint16_t(reg_addr <<2),(r##reg_addr.u32))){	\
			Log.Write_REG_List(1,reg_addr,r##reg_addr.u32);						\
			if (strstr(__FILE__,"rf.cpp")) {									\
				Log.stdprintf("RF Line:%d",__FILE__);							\
				return (E_K7REG_W | reg_addr | (__LINE__ << 16));				\
			}																	\
		}																		\
		Log.Write_REG_List(0,reg_addr,r##reg_addr.u32);							\
	}while (0);
				
#define ad9361_k7r(reg_addr)													\
	do {																		\
		if (PCIE_Core.ReadWordBar1(uint16_t(reg_addr <<2),(&r##reg_addr.u32))){	\
			Log.Write_REG_List(1,reg_addr,-1,0xFFFF);							\
			if (strstr(__FILE__,"rf.cpp")) {									\
				Log.stdprintf("RF Line:%d",__LINE__);							\
				return (E_K7REG_R | reg_addr | (__LINE__ << 16));				\
			}																	\
		}																		\
		Log.Write_REG_List(0,reg_addr,-1,r##reg_addr.u32);						\
	}while (0);

//Register Define
REG_DEF_S(0x0000)
    unsigned op  : 1;
    unsigned rsv : 7;
    unsigned ver : 8;
    unsigned cdw : 4;
#define cdw_invalid 0
#define cdw_32      1
#define cdw_64      2
#define cdw_128     3
REG_DEF_E

REG_DEF_S(0x0001)
    unsigned fpga_wr_op   : 1;
    unsigned rsv          : 6;
	unsigned dis_wr_int   : 1;
    unsigned fpga_wr_done : 1;
    unsigned rsv2         : 7;

    unsigned fpga_rd_op   : 1;
    unsigned rsv3         : 6;
	unsigned dis_rd_int   : 1;
	unsigned fpga_rd_done : 1;
	unsigned rsv4         : 7;
REG_DEF_E

REG_DEF_S(0x0007)
	unsigned rsv : 2;
	unsigned adr : 30;
REG_DEF_E

REG_DEF_S(0x0008)
	unsigned tlp_size : 16;
REG_DEF_E

REG_DEF_S(0x0009)
	unsigned tlp_count : 20;
REG_DEF_E

REG_DEF_S(0x0014)
//	unsigned ram_tlp : 16;
//	unsigned ram_num : 16;
	unsigned samples : 32;
REG_DEF_E

REG_DEF_S(0x0016)
	unsigned data : 32;
REG_DEF_E

REG_DEF_S(0x0017)
	unsigned adr : 32;
REG_DEF_E

REG_DEF_S(0x0020)
	unsigned dma_rd_timer : 32;
REG_DEF_E

REG_DEF_S(0x0021)
	unsigned dma_rd_counter : 32;
REG_DEF_E

REG_DEF_S(0x0025)
	unsigned level_select : 1;
	unsigned rx_select	  : 3;
	unsigned rx_43503_8   : 1;
	unsigned rx_43503_16  : 1;
REG_DEF_E

REG_DEF_S(0x0028)
	unsigned Flash_CS	  : 1;
	unsigned Flash_wp	  : 1;
	unsigned Tx_Power_Off : 1;
REG_DEF_E

REG_DEF_S(0x0035)
	unsigned op : 1;
REG_DEF_E

REG_DEF_S(0x0036)
	unsigned lo_freq : 16;
REG_DEF_E 

REG_DEF_S(0x0029)
	unsigned op : 1;
REG_DEF_E

REG_DEF_S(0x002a)
	unsigned ddc : 32;
REG_DEF_E

REG_DEF_S(0x002b)
	unsigned ddc_rd : 32;
REG_DEF_E

REG_DEF_S(0x002c)
	unsigned rx_data_power_en : 1;
REG_DEF_E

REG_DEF_S(0x002d)
	signed baseband_att : 16;
	signed rsv			: 16;
REG_DEF_E

REG_DEF_S(0x00e2)
	unsigned fpga_cw : 1;
REG_DEF_E

REG_DEF_S(0x00e3)
	unsigned fpga_cw_freqMHz : 32;
REG_DEF_E

REG_DEF_S(0x00e4)
	unsigned level_select : 1;
	unsigned rx_select	  : 3;
	unsigned rx_43503_8   : 1;
	unsigned rx_43503_16  : 1;
REG_DEF_E

REG_DEF_S(0x1002)
	unsigned repetition : 1;
	unsigned cycles     : 12;
	unsigned rsv        : 2;
	unsigned addition   : 17;
REG_DEF_E

REG_DEF_S(0x1003)
	unsigned adr : 5;
	unsigned seg : 6;
	unsigned rsv : 20;
	unsigned op  : 1;
REG_DEF_E

REG_DEF_S(0x1004)
	unsigned data : 32;
REG_DEF_E

REG_DEF_S(0x1005)
    unsigned seg : 7;
    unsigned rsv : 25;
REG_DEF_E

REG_DEF_S(0x1006)
	unsigned samples : 31;
	unsigned rsv     : 1;
REG_DEF_E

REG_DEF_S(0x1007)
	unsigned adr : 32;
REG_DEF_E

//REG_DEF_S(0x1008)
//	unsigned flag : 32;
//REG_DEF_E

REG_DEF_S(0x1009)
	unsigned apc	: 1;
	unsigned rsv	: 7;
	unsigned en		: 1;
	unsigned rsv2	: 22;
	unsigned abort  : 1;
REG_DEF_E

REG_DEF_S(0x100a)
	unsigned delay      : 27;
	unsigned src        : 3;
	unsigned autostart  : 1;
	unsigned retrig     : 1;
REG_DEF_E

REG_DEF_S(0x100c)
	unsigned ms_rm   : 2;
	unsigned ms_trig : 2;
	unsigned rsv     : 28;
REG_DEF_E

REG_DEF_S(0x1010)
	unsigned op      : 1;
	unsigned rsv     : 31;
REG_DEF_E

REG_DEF_S(0x1011)
	unsigned offset : 32;
REG_DEF_E

REG_DEF_S(0x1060)
	unsigned iqcap_trig_offset :32;
REG_DEF_E

REG_DEF_S(0x1061)
	unsigned iqcap_cap_offset : 32;
REG_DEF_E

REG_DEF_S(0x1062)
	unsigned iqcap_src_sel  :6;
	unsigned iqcap_src_edge :1;
REG_DEF_E

REG_DEF_S(0x1063)
	unsigned iqcap_timeout : 31;
	unsigned iqcap_tmo	   : 1;
REG_DEF_E

REG_DEF_S(0x1064)
	unsigned op : 1;
REG_DEF_E

REG_DEF_S(0x1065)
	unsigned abort : 1;
REG_DEF_E

REG_DEF_S(0x1066)
	unsigned samples : 32;
REG_DEF_E

REG_DEF_S(0x1067)
	unsigned gap : 32;
REG_DEF_E

REG_DEF_S(0x1068)
	unsigned threshold : 32;
REG_DEF_E

REG_DEF_S(0x1069)
	unsigned rx_dec_num : 11;
	unsigned rsv		: 4;
	unsigned rx_dec_op  : 1;

	unsigned rx_1st_fltr_num : 3;
	unsigned rsv2			 : 4;
	unsigned rx_1st_fltr_op  : 1;

	unsigned rx_last_fltr_num: 4;
	unsigned rsv3			 : 3;
	unsigned rx_last_fltr_op : 1;
REG_DEF_E

REG_DEF_S(0x106f)
	unsigned cap_tlp_count : 16;
REG_DEF_E

#endif