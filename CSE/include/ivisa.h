#ifndef _IVISA_H_
#define _IVISA_H_

#define VI_PEEKPOKE_BAR1
#define MAX_VI_BUFFER_SIZE 2048
//#define NIVISA_PXI

#if defined VI_PEEKPOKE_BAR1
//#define NIVISA_PEEKPOKE
#endif

#include "visa.h"
#include "vpptype.h"
#include "visatype.h"
#include "stdint.h"


class AFX_EXT_CLASS IVisa {
protected:
	ViSession        m_defaultRM;
#if defined VI_PEEKPOKE_BAR1
	ViAddr           m_viAddr;
	bool MemoryMap() {
		if (viMapAddress(m_hVisa, VI_PXI_BAR1_SPACE, 0, 0x1000, VI_FALSE, VI_NULL, &m_viAddr) < VI_SUCCESS) {
			if (viMapAddress(m_hVisa, VI_PXI_BAR1_SPACE, 0, 0x1000, VI_FALSE, VI_NULL, &m_viAddr) < VI_SUCCESS) {
				return false;
			}
		}
		return true;
	}

	bool PxIMemWr(uint32_t addr, uint32_t data) {
		if (m_viAddr == 0 && !MemoryMap()) {
			return false;
		}
		viPoke32(m_hVisa, (ViPByte)(m_viAddr) + addr, data);
		return true;
	};
	bool PxIMemRd(uint32_t addr, uint32_t &data) {
		if (m_viAddr == 0 && !MemoryMap()) {
			return false;
		}
		viPeek32(m_hVisa, (ViPByte)(m_viAddr) + addr, &data);
		return true;
	}
	bool PxIMemWr(uint32_t addr, uint32_t *data, uint32_t length) {
		for (uint32_t i = 0; i < length; i++) {
			PxIMemWr(addr, data[i]);
		}
		return true;
	}
#else
	bool PxIMemWr(uint32_t addr, uint32_t data) {
		ViStatus status;
		status = viOut32(m_hVisa, VI_PXI_BAR1_SPACE, addr, data);
		return !(status < VI_SUCCESS);
	};
	bool PxIMemRd(uint32_t addr, uint32_t &data) {
		ViStatus status;
		status = viIn32(m_hVisa, VI_PXI_BAR1_SPACE, addr, &data);
		return !(status < VI_SUCCESS);
	}
	bool PxIMemWr(uint32_t addr, uint32_t *data, uint32_t length) {
		ViStatus status;
		status = viMoveOut32(m_hVisa, VI_PXI_BAR1_SPACE, addr, length, data);
		return !(status < VI_SUCCESS);
	}
#endif
	bool PxIIOWr(uint32_t addr, uint32_t data) {
		ViStatus status;
		status = viOut32(m_hVisa, VI_PXI_BAR0_SPACE, addr, data);
		return !(status < VI_SUCCESS);
	}
	bool PxIIORd(uint32_t addr, uint32_t &data) {
		ViStatus status;
		status = viIn32(m_hVisa, VI_PXI_BAR0_SPACE, addr, &data);
		return !(status < VI_SUCCESS);
	}
	bool setPosedge(uint32_t addr, uint32_t shift, BOOL isMemSpace = true) {
		if (isMemSpace) {
			BOOL_CHECK(PxIMemWr(addr, 0));
			BOOL_CHECK(PxIMemWr(addr, 1 << shift));
			BOOL_CHECK(PxIMemWr(addr, 0));
		} else {
			BOOL_CHECK(PxIIOWr(addr, 0));
			BOOL_CHECK(PxIIOWr(addr, 1 << shift));
			BOOL_CHECK(PxIIOWr(addr, 0));
		}
		return true;
	}

	bool setPosedge(uint32_t addr, uint32_t shift, uint32_t regValue, BOOL isMemSpace = true) {
		if (isMemSpace) {
			BOOL_CHECK(PxIMemWr(addr, regValue & ~(1 << shift)));
			BOOL_CHECK(PxIMemWr(addr, regValue | (1 << shift)));
			BOOL_CHECK(PxIMemWr(addr, regValue & ~(1 << shift)));
		} else {
			BOOL_CHECK(PxIIOWr(addr, regValue & ~(1 << shift)));
			BOOL_CHECK(PxIIOWr(addr, regValue | (1 << shift)));
			BOOL_CHECK(PxIIOWr(addr, regValue & ~(1 << shift)));
		}
		return true;
	}
protected:
	bool Print(const char *format, ...) {
		char     buffer[MAX_VI_BUFFER_SIZE];
		uint32_t retCnt = 0;
		ViStatus ret;
		
		ZeroMemory(buffer, MAX_VI_BUFFER_SIZE);
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buffer, MAX_VI_BUFFER_SIZE, format, ap);
		va_end(ap);

		ret = viWrite(m_hVisa, (uint8_t *)buffer, strlen(buffer), &retCnt);
		if (ret < VI_SUCCESS || retCnt != strlen(buffer) ) {
			return false;
		}
		return true;
	}
	bool Read(LPCTSTR buffer, uint32_t maxLength, int timeout = 3000) {
		uint32_t retCnt;
		uint32_t last_timeout;
		ViStatus status;
		if (timeout != 3000) {
			viGetAttribute(m_hVisa, VI_ATTR_TMO_VALUE, &last_timeout);
			viSetAttribute(m_hVisa, VI_ATTR_TMO_VALUE, timeout);
		}
		status = viRead(m_hVisa, (uint8_t *)buffer, maxLength, &retCnt);
		if (timeout != 3000) {
			viSetAttribute(m_hVisa, VI_ATTR_TMO_VALUE, last_timeout);
		}
		return !(status < VI_SUCCESS);
	}
protected:
	IVisa() {
		m_hVisa  = 0;
		m_viAddr = 0;
	}
	
	~IVisa() {
		Close();
	}
public:
	virtual bool Close() {
		if (m_hVisa != 0) {
#ifdef VI_PEEKPOKE_BAR1
// 			if (m_viAddr != 0) {
// 				viUnmapAddress(m_hVisa);
// 				m_viAddr = 0;
// 			}
#endif
			viClose(m_hVisa);
#ifdef VI_PEEKPOKE_BAR1
			m_viAddr = 0;
#endif
		}
		m_hVisa = 0;
		return true;
	}

	bool Init(LPCTSTR expr) {
		ViSession  defaultRM;
		ViFindList findlist;
		ViUInt32   retCnt;
		ViChar     instrDesc[256];
		
		if ((viOpenDefaultRM(&defaultRM) < VI_SUCCESS)) {
			return false;
		}
		if (viFindRsrc(defaultRM, (LPSTR)expr, &findlist, &retCnt, instrDesc) < VI_SUCCESS) {
			viClose(defaultRM);
			return false;
		}
		Close();
		while (retCnt--) {
			if (viOpen(defaultRM, instrDesc, VI_NULL, VI_NULL, &m_hVisa) < VI_SUCCESS) {
				viFindNext(findlist, instrDesc);
				continue;
			}
			viClose(findlist);
			return true;
		}
		m_hVisa = 0;
		viClose(findlist);
		return false;
	}
	ViSession m_hVisa;
};

#endif