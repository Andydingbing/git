// SG_n5182b.h: interface for the SG_n5182b class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SG_N5182B_H__DECA3D0D_8067_4B0F_9278_692A6EF4DEF2__INCLUDED_)
#define AFX_SG_N5182B_H__DECA3D0D_8067_4B0F_9278_692A6EF4DEF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SignalGenerator.h"

class SG_n5182b : public ISignalGenerator  
{
public:
	SG_n5182b();
	virtual ~SG_n5182b();
public:
	virtual int		GetDefaultPriAddr()        { return 19; }
	virtual int		GetDefaultSecAddr()        { return -1; }
	virtual LPCTSTR GetInstrumentDescription() { return "N5182B"; }
	virtual bool	Init(ViRsrc rsrcName);
public:
	virtual bool	Reset();
	virtual bool	SetCW(double freqHz);
	virtual bool	GetCW(double &freqHz);
	virtual bool	SetPL(double dbm);
	virtual bool	GetPL(double &dbm);
	virtual bool	SetOutputEnable(bool bEnable);
	virtual bool	SetModEnable(bool bEnable);
	virtual bool	LoadOffset();
	virtual bool	SaveOffset(CInstrOffsetData &data, bool isImport);
	virtual bool	ImportCableLoss(CInstrOffsetData *pOffset);
protected:
	CInstrOffsetData m_cableLossData;	// ÏßËð
	CInstrOffsetData m_instrOffset;		// ÒÇÆ÷Îó²î
	CString          m_SN;
	bool             m_isInitialized;
	double           m_dbm;
};

#endif // !defined(AFX_SG_N5182B_H__DECA3D0D_8067_4B0F_9278_692A6EF4DEF2__INCLUDED_)
