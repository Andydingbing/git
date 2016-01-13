#include "StdAfx.h"

#ifndef NIVISA_PXI
#define NIVISA_PXI
#endif

#include "afx.h"
#include "visa.h"
#include "vpptype.h"
#include "visatype.h"
#include "visa_driver.h"
#include "log.h"

ViSession core::m_rsrc_manager    = 0;
char      core::m_last_error[512] = { 0 };
//CRITICAL_SECTION cs;

core::core(void): m_vi_device(0)
				, m_vi_memory(0)
				, m_phy_addr(0)
{
	//InitializeCriticalSection(&cs);
}

core::~core(void)
{
}

core &core::Instance()
{
	static core pcie_core;
	return pcie_core;
}

ViInt32 core::ibfind(const ViRsrc InstrDesc)
{
	ViStatus OpenRM_status;
	ViStatus status;
	ViUInt16 iManf, iModel;

	if ((OpenRM_status = viOpenDefaultRM(&m_rsrc_manager)) != VI_SUCCESS) {
		set_last_error("viOpenDefaultRM error");
		return -1;
	}

	if ((status = viOpen(m_rsrc_manager, InstrDesc, VI_NULL, VI_NULL, &m_vi_device)) < VI_SUCCESS) {
		set_last_error("viOpen %s error",InstrDesc);
		return -1;
	}

	if ((status = viGetAttribute(m_vi_device,VI_ATTR_MANF_ID, &iManf)) < VI_SUCCESS || (iManf != 0x10EE)) {
		viClose(m_vi_device);
		return -1;
	}
	if ((status = viGetAttribute(m_vi_device, VI_ATTR_MODEL_CODE, &iModel)) < VI_SUCCESS || (iModel != 7 )) {	
		viClose(m_vi_device);
		return -1;
	}
	return 0;
}

ViInt32 core::WriteWordBar0(ViUInt32 offset, ViUInt32 wdata)
{
	if (viOut32(m_vi_device, VI_PXI_BAR0_SPACE, offset, wdata) < VI_SUCCESS) {
		return -1;
	}
	return 0;
}

ViInt32 core::WriteWordBar1(ViUInt32 offset, ViUInt32 wdata)
{
	if (viOut32(m_vi_device, VI_PXI_BAR1_SPACE, offset, wdata) < VI_SUCCESS) {
		return -1;
	}
	return 0;
}

ViInt32 core::WriteWordBar2(ViUInt32 offset, ViUInt32 wdata)
{
	if (viOut32(m_vi_device, VI_PXI_BAR2_SPACE, offset, wdata) < VI_SUCCESS) {
		return -1;
	}
	return 0;
}

// VOID LowWriteWordBar2(ViUInt32 offset, ViUInt32 wdata)
// {
// 	viPoke32(m_vi_device, ((ViPByte)address)+offset, wdata);
// }
// 
// VOID LowReadWordBar2(ViUInt32 offset, ViUInt32 *readWord)
// {
// 	viPeek32(m_vi_device, ((ViPByte)address)+offset,readWord);
// }

ViInt32 core::WriteWordBar3( ViUInt32 offset, ViUInt32 wdata)
{
	if (viOut32(m_vi_device, VI_PXI_BAR3_SPACE, offset, wdata) < VI_SUCCESS) {
		return -1;
	}
	return 0;
}

ViInt32 core::WriteWordBar4( ViUInt32 offset, ViUInt32 wdata)
{
	if (viOut32(m_vi_device, VI_PXI_BAR4_SPACE, offset, wdata) < VI_SUCCESS) {
		return -1;
	}
	return 0;
}

ViInt32 core::ReadWordBar0( ViUInt32 offset,ViPUInt32  readWord)
{
	ViUInt32 rdata;
	if (viIn32(m_vi_device, VI_PXI_BAR0_SPACE, offset, &rdata) < VI_SUCCESS) {
		return -1;
	}
	*readWord=rdata;
	return 0;
}

ViInt32 core::ReadWordBar1( ViUInt32 offset,ViPUInt32  readWord)
{
	ViUInt32 rdata;
	if (viIn32(m_vi_device, VI_PXI_BAR1_SPACE, offset, &rdata) < VI_SUCCESS) {
		return -1;
	}
	*readWord=rdata;
	return 0;
}

ViInt32 core::ReadWordBar2( ViUInt32 offset,ViPUInt32  readWord)
{
	ViUInt32 rdata;
	if (viIn32(m_vi_device, VI_PXI_BAR2_SPACE, offset, &rdata) < VI_SUCCESS) {
		return -1;
	}
	*readWord=rdata;
	return 0;
}

ViInt32 core::ReadWordBar3(ViUInt32 offset,ViPUInt32  readWord)
{
	ViUInt32 rdata;
	if (viIn32(m_vi_device, VI_PXI_BAR3_SPACE, offset, &rdata) < VI_SUCCESS) {
		return -1;
	}
	*readWord=rdata;
	return 0;
}

ViInt32 core::ReadWordBar4( ViUInt32 offset,ViPUInt32  readWord)
{
	ViUInt32 rdata;
	if (viIn32(m_vi_device, VI_PXI_BAR4_SPACE, offset, &rdata) < VI_SUCCESS) {
		return -1;
	}
	*readWord=rdata;
	return 0;
}

int core::get_devices(CStringArray &devices) {
	ViFindList     findlist;
	ViUInt32       retCnt;
	char           buf[255];
	ViStatus       vi_status;

	devices.RemoveAll();

	if (m_rsrc_manager != 0)
		viClose(m_rsrc_manager);

	if ((vi_status = viOpenDefaultRM(&m_rsrc_manager)) < VI_SUCCESS) {
		set_last_error("viOpenDefaultRM Error, ViStatus 0x%x", vi_status);
		return -1;
	}

	if ((vi_status = viFindRsrc(m_rsrc_manager, "PXI?*INSTR", &findlist, &retCnt, buf)) < VI_SUCCESS) {
		set_last_error("viFindRsrc Error, ViStatus 0x%x", vi_status);
		return -1;
	}
	devices.Add(CString(buf));
	while (retCnt--) {
		if ((vi_status = viFindNext(findlist, buf)) < VI_SUCCESS) {
			set_last_error("viFindNext Error, ViStatus 0x%x", vi_status);
			return -1;
		}
		devices.Add(CString(buf));
	}
	viClose(findlist);
	return 0;
}

int core::init_device(CString device) {
	ViStatus vi_status;
	if ((vi_status = viOpenDefaultRM(&m_rsrc_manager)) < VI_SUCCESS) {
		set_last_error("viOpenDefaultRM Error, ViStatus 0x%x", vi_status);
		return -1;
	}
	if ((vi_status = viOpen(m_rsrc_manager, (LPSTR)(LPCTSTR)device, VI_NULL, VI_NULL, &m_vi_device)) < VI_SUCCESS) {
		set_last_error("viOpen %s Error, ViStatus 0x%x", device, vi_status);
		return -1;
	}
	if ((vi_status = viMapAddress(m_vi_device, VI_PXI_BAR1_SPACE, 0, 0x1000, VI_NULL, VI_NULL, &m_device_map_offset)) < VI_SUCCESS) {
		set_last_error("viMapAddress %s Error, ViStatus 0x%x", device, vi_status);
		return -1;
	}
// 	g_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	return 0;
}

int core::mem_alloc(unsigned int size) {
	ViStatus vi_status;
	if (!m_rsrc_manager) {
		if ((vi_status = viOpenDefaultRM(&m_rsrc_manager)) < VI_SUCCESS) {
			set_last_error("viOpenDefaultRM Error, ViStatus 0x%x", vi_status);
			return -1;
		}
	}
	if ((vi_status = viOpen(m_rsrc_manager, (ViRsrc)"PXI::MEMACC", VI_NULL, VI_NULL, &m_vi_memory)) < VI_SUCCESS) {
		set_last_error("viOpen PXI::MEMACC Error, ViStatus 0x%x", vi_status);
		return -1;
	}

	size = size + 4096;										// 物理地址4K对齐，申请空间需要增加4K

	if ((vi_status = viMemAlloc(m_vi_memory, size, &m_phy_addr)) < VI_SUCCESS) {
		set_last_error("viMemAlloc Error, ViStatus 0x%x", vi_status);
		return -1;
	}
	m_phy_release_addr = m_phy_addr;						// 释放物理地址

	m_phy_addr = (DWORD)(m_phy_addr / 4096 + 1) * 4096;		// 物理地址4K对齐

	if ((vi_status = viSetAttribute(m_vi_memory, VI_ATTR_DEST_INCREMENT, 1)) < VI_SUCCESS) {
		set_last_error("viSetAttribute Error, ViStatus 0x%x", vi_status);
		return -1;
	}
	//	viInstallHandler(m_vi_device, VI_EVENT_PXI_INTR, pxi_isr, 0);
	//	viEnableEvent(m_vi_device, VI_EVENT_PXI_INTR, VI_QUEUE, VI_NULL);
	//	stdprintf("install event\n");
	//	if ((vi_status = viInstallHandler(m_vi_device, VI_EVENT_PXI_INTR, (ViHndlr)pxi_isr, 0)) < VI_SUCCESS) {
	//		set_last_error("viInstallHandler Error, ViStatus 0x%x", vi_status);
	//	}
	//	stdprintf("enable event\n");
	//	if ((vi_status = viEnableEvent(m_vi_device, VI_EVENT_PXI_INTR, VI_HNDLR, VI_NULL)) < VI_SUCCESS) {
	//		set_last_error("viEnableEvent Error, ViStatus 0x%x", vi_status);
	//	}

	// 	if ((vi_status = viInstallHandler(m_vi_memory, VI_EVENT_PXI_INTR, pxi_isr, 0)) < VI_SUCCESS) {
	// 		set_last_error("viInstallHandler Error, ViStatus 0x%x", vi_status);
	// 	}
	// 	if ((vi_status = viEnableEvent(m_vi_memory, VI_EVENT_PXI_INTR, VI_HNDLR, VI_NULL)) < VI_SUCCESS) {
	// 		set_last_error("viEnableEvent Error, ViStatus 0x%x", vi_status);
	// 	}

	return 0;
}

int core::init_write_logic(ULONG tlp_size, ULONG tlp_count) {
	Log.stdprintf("init write logic\n");
	//	QueryPerformanceCounter(&start_timestamp);

	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, LEAGACYCLR, 0x00000000);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, DCSR, 0x00000001);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, DCSR, 0x00000000);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, WDMATLPA, m_phy_addr);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, WDMATLPS, tlp_size);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, WDMATLPC, tlp_count);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, DDMACR, 0x00000001);
	// 	
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + LEAGACYCLR, 0x00000000);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + DCSR, 0x00000001);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + DCSR, 0x00000000);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + WDMATLPA, m_phy_addr);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + WDMATLPS, tlp_size);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + WDMATLPC, tlp_count);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + DDMACR, 0x00000001);
	return 0;
}

int core::init_read_logic(ULONG tlp_size, ULONG tlp_count) {
	Log.stdprintf("init read logic\n");
	//	QueryPerformanceCounter(&start_timestamp);

	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, LEAGACYCLR, 0x00000000);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, DCSR, 0x00000001);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, DCSR, 0x00000000);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, RDMATLPA, m_phy_addr);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, RDMATLPS, tlp_size);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, RDMATLPC, tlp_count);
	// 	viOut32(m_vi_device, VI_PXI_BAR1_SPACE, DDMACR, 0x00010000);

	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + LEAGACYCLR, 0x00000000);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + DCSR, 0x00000001);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + DCSR, 0x00000000);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + RDMATLPA, m_phy_addr);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + RDMATLPS, tlp_size);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + RDMATLPC, tlp_count);
	viPoke32(m_vi_device, (ViPByte)m_device_map_offset + DDMACR, 0x00010000);
//	g_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	return 0;
}

int core::data_read(byte *buf, ULONG size) {
	ViStatus vi_status;
	if ((vi_status = viMoveIn8(m_vi_memory, VI_PXI_ALLOC_SPACE, m_phy_addr, size, buf)) < VI_SUCCESS) {
		set_last_error("viMoveIn8 Error, ViStatus = 0x%x", vi_status);
		return -1;
	}
	return 0;
}

int core::data_write(byte *buf, ULONG size) {
	ViStatus vi_status;
	if ((vi_status = viMoveOut8(m_vi_memory, VI_PXI_ALLOC_SPACE, m_phy_addr, size, buf)) < VI_SUCCESS) {
		set_last_error("viMoveOut8 Error, ViStatus = 0x%x", vi_status);
		return -1;
	}
	return 0;
}

int core::data_read32(DWORD *buf, ULONG size) {
	ViStatus vi_status;
	if ((vi_status = viMoveIn32(m_vi_memory, VI_PXI_ALLOC_SPACE, m_phy_addr, size, buf)) < VI_SUCCESS) {
		set_last_error("viMoveIn32 Error, ViStatus = 0x%x", vi_status);
		return -1;
	}
	return 0;
}

int core::data_read16(WORD *buf, ULONG size) {
	ViStatus vi_status;
	if ((vi_status = viMoveIn16(m_vi_memory, VI_PXI_ALLOC_SPACE, m_phy_addr, size, buf)) < VI_SUCCESS) {
		set_last_error("viMoveIn16 Error, ViStatus = 0x%x", vi_status);
		return -1;
	}
	return 0;
}

int core::data_write32(DWORD *buf, ULONG size) {
	ViStatus vi_status;
	if ((vi_status = viMoveOut32(m_vi_memory, VI_PXI_ALLOC_SPACE, m_phy_addr, size, buf)) < VI_SUCCESS) {
		set_last_error("viMoveOut32 Error, ViStatus = 0x%x", vi_status);
		return -1;
	}
	return 0;
}

int core::mem_release() {
	ViStatus vi_status;
	if (m_vi_memory) {
		if ((vi_status = viMemFree(m_vi_memory, m_phy_release_addr)) < VI_SUCCESS) {
			set_last_error("viMemFree Error, ViStatus = 0x%x", vi_status);
			return -1;
		}
		if ((vi_status = viClose(m_vi_memory)) < VI_SUCCESS) {
			set_last_error("viClose memory Error, ViStatus = 0x%x", vi_status);
			return -1;
		}
	}

	if (m_vi_device) {
		if ((vi_status = viUnmapAddress(m_vi_device)) < VI_SUCCESS) {
			set_last_error("ViUnMapAddress Device Error, ViStatus = 0x%x", vi_status);
			return -1;
		}
		if ((vi_status = viClose(m_vi_device)) < VI_SUCCESS) {
			set_last_error("viClose Device Error, ViStatus = 0x%x", vi_status);
			return -1;
		}
	}
	return 0;
}

ViStatus _VI_FUNCH core::pxi_isr(ViSession vi, ViEventType event_type, ViEvent context, ViAddr user_handle) {

	//stdprintf("recv pxi interrupt, Enter Critical Section\n");
	//QueryPerformanceCounter(&end_timestamp);
	g_Counter++;
	//	SetEvent(&g_event);

	//	viOut32(vi, VI_PXI_BAR1_SPACE, LEAGACYCLR, 0x00000100);
	//	viOut32(vi, VI_PXI_BAR1_SPACE, LEAGACYCLR, 0x00000000);
	Log.stdprintf("clear pxi interrupt, Leave Critical Section, %d\n", g_Counter);

	//double dfCalc[3];
	//dfCalc[0] = timestamp_freq.QuadPart;
	//dfCalc[1] = start_timestamp.QuadPart;
	//dfCalc[2] = end_timestamp.QuadPart;
	//stdprintf("trans spent %.3f ms\n", (dfCalc[2] - dfCalc[1]) / dfCalc[0] * 1000);

	g_isr_params.vi          = vi;
	g_isr_params.event_type  = event_type;
	g_isr_params.context     = context;
	g_isr_params.user_handle = user_handle;

	viGetAttribute(context, VI_ATTR_PXI_RECV_INTR_SEQ, &g_isr_params.recv_seq);
	viGetAttribute(context, VI_ATTR_PXI_RECV_INTR_DATA, &g_isr_params.recv_data);

	g_isr_params.vi_status   = viClose(context);

	viDisableEvent(vi, VI_EVENT_PXI_INTR, VI_HNDLR);
	//viEnableEvent(vi, VI_EVENT_PXI_INTR, VI_HNDLR, VI_NULL);
	return VI_SUCCESS;
}

void core::set_last_error(char *format, ...)
{
	va_list ap;

	memset(m_last_error, 0, 512);

	va_start(ap, format);
	vsprintf(m_last_error, format, ap);
	va_end(ap);
}

char *core::get_last_error()
{
	return m_last_error;
}