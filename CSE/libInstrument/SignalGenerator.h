// SignalGenerator.h: interface for the CSignalGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIGNALGENERATOR_H__1F423A9C_7ABD_4BD8_B726_A891CA900D03__INCLUDED_)
#define AFX_SIGNALGENERATOR_H__1F423A9C_7ABD_4BD8_B726_A891CA900D03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IVisa.h"
#include "InstrOffsetData.h"

class ISignalGenerator : public IVisa
{
public:
	ISignalGenerator()	{ }
	virtual				~ISignalGenerator() { }
	virtual int			GetDefaultPriAddr() = 0;
	virtual int			GetDefaultSecAddr() = 0;
	virtual LPCTSTR		GetInstrumentDescription() = 0;
	virtual bool		Init(ViRsrc rsrcName) = 0;
public:
	virtual bool		Reset() = 0;
	virtual bool		SetCW(double freqHz) = 0;
	virtual bool		SetPL(double dbm) = 0;
	virtual bool		SetOutputEnable(bool bEnable) = 0;
	virtual bool		SetModEnable(bool bEnable) = 0;
	virtual bool		LoadOffset() = 0;
	virtual bool		SaveOffset(CInstrOffsetData &data, bool isImport) = 0;
	virtual bool		ImportCableLoss(CInstrOffsetData *pOffset) = 0;
	virtual bool		SetEnableOffset(bool isEnableInstrOffset, bool isEnableCableLoss) {
		m_isEnableInstrOffset = isEnableInstrOffset;
		m_isEnableCableLoss   = isEnableCableLoss;
		return true;
	}
protected:
	CInstrOffsetData	m_cableLossData;	// ÏßËð
	CInstrOffsetData	m_instrOffset;		// ÒÇÆ÷Îó²î
	bool                m_isEnableInstrOffset;
	bool				m_isEnableCableLoss;
};

#endif // !defined(AFX_SIGNALGENERATOR_H__1F423A9C_7ABD_4BD8_B726_A891CA900D03__INCLUDED_)
