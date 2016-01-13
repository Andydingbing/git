#pragma once
#ifndef _VISA_DRIVER_H_
#define _VISA_DRIVER_H_

#include "visa.h"
#include "vpptype.h"
#include "visatype.h"

#define PCIE_Core core::Instance()

enum {
	DCSR       = 0x0000, // device_control_status
	DDMACR     = 0x0004, // device_dma_control_status
	WDMATLPA   = 0x0008, // write_dma_tlp_address
	WDMATLPS   = 0x000C, // write_dma_tlp_size
	WDMATLPC   = 0x0010, // write_dma_tlp_count
	WDMATLPP   = 0x0014, // write_dma_data_pattern
	RDMATLPP   = 0x0018, // read_dma_expected_data_pattern
	RDMATLPA   = 0x001C, // read_dma_tlp_address
	RDMATLPS   = 0x0020, // read_dma_tlp_size
	RDMATLPC   = 0x0024, // read_dma_tlp_count
	WDMAPERF   = 0x0028, // write_dma_performance
	RDMAPERF   = 0x002C, // read_dma_performance
	RDMASTAT   = 0x0030, // read_dma_status
	NRDCOMP    = 0x0034, // number_of_read_completion
	RCOMPDSIZW = 0x0038, // read_completion_data_size
	DLWSTAT    = 0x003C, // device_link_width_status
	DLTRSSTAT  = 0x0040, // device_link_transaction_size_status
	DMISCCONT  = 0x0044, // device_miscellaneous_control
	LEAGACYCLR = 0x0048, // legacy interrupt clear
};

class core
{
public:
	core(void);
	~core(void);
	static core &Instance();
	
	int get_devices(CStringArray &devices);
	int init_device(CString device);

	ViInt32 ibfind(const ViRsrc InstrDesc);
	ViInt32 WriteWordBar0( ViUInt32 offset, ViUInt32 wdata);
	ViInt32 WriteWordBar1( ViUInt32 offset, ViUInt32 wdata);
	ViInt32 WriteWordBar2( ViUInt32 offset, ViUInt32 wdata);
	ViInt32 WriteWordBar3( ViUInt32 offset, ViUInt32 wdata);
	ViInt32 WriteWordBar4( ViUInt32 offset, ViUInt32 wdata);

	ViInt32 ReadWordBar0( ViUInt32 offset,ViPUInt32  readWord);
	ViInt32 ReadWordBar1( ViUInt32 offset,ViPUInt32  readWord);
	ViInt32 ReadWordBar2( ViUInt32 offset,ViPUInt32  readWord);
	ViInt32 ReadWordBar3( ViUInt32 offset,ViPUInt32  readWord);
	ViInt32 ReadWordBar4( ViUInt32 offset,ViPUInt32  readWord);

	int mem_alloc(unsigned int size);
	int mem_release();
	int data_write(byte *buf, ULONG size);
	int data_write32(DWORD *buf, ULONG size);
	int data_read(byte *buf, ULONG size);
	int data_read16(WORD *buf, ULONG size);
	int data_read32(DWORD *buf, ULONG size);

	int init_write_logic(ULONG tlp_size, ULONG tlp_count);
	int init_read_logic(ULONG tlp_size, ULONG tlp_count);

	ViStatus _VI_FUNCH pxi_isr(ViSession vi, ViEventType event_type, ViEvent context, ViAddr user_handle);
public:
	static char *get_last_error();
private:
	static void set_last_error(char *format, ...);
private:
	HANDLE g_event;
	static ViSession m_rsrc_manager;
	static char m_last_error[512];
public:
	ViSession    m_vi_device;
	ViSession    m_vi_memory;
	ViBusAddress m_phy_addr;
	ViBusAddress m_phy_release_addr;
	ViAddr       m_device_map_offset;
private:
	struct isr_params_t {
		ViSession	vi;
		ViEventType event_type;
		ViEvent		context;
		ViAddr		user_handle;
		ViStatus	vi_status;
		DWORD		recv_seq;
		DWORD		recv_data;
	};
public:
	isr_params_t g_isr_params;
	int			 g_Counter;
};

#endif

