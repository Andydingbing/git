// SG_e4438c.h: interface for the SG_e4438c class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SG_E4438C_H__7B5679AE_6981_4B91_9539_1305E7E049DD__INCLUDED_)
#define AFX_SG_E4438C_H__7B5679AE_6981_4B91_9539_1305E7E049DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SignalGenerator.h"

class SG_e4438c : public ISignalGenerator  
{
public:
	SG_e4438c();
	virtual ~SG_e4438c();
public:
	virtual int			GetDefaultPriAddr()        { return 19; }
	virtual int			GetDefaultSecAddr()        { return -1; }
	virtual LPCTSTR		GetInstrumentDescription() { return "E4438C"; }
	virtual bool		Init(ViRsrc rsrcName);
public:
	virtual bool		Reset();
	virtual bool		SetCW(double freqHz);
	virtual bool		GetCW(double &freqHz);
	virtual bool		SetPL(double dbm);
	virtual bool		GetPL(double &dbm);
	virtual bool		SetOutputEnable(bool bEnable);
	virtual bool		SetModEnable(bool bEnable);
	virtual bool		LoadOffset();
	virtual bool		SaveOffset(CInstrOffsetData &data, bool isImport);
	virtual bool		ImportCableLoss(CInstrOffsetData *pOffset);
protected:
	CInstrOffsetData	m_cableLossData;	// ÏßËð
	CInstrOffsetData	m_instrOffset;		// ÒÇÆ÷Îó²î
	CString             m_SN;
	bool                m_isInitialized;
};

#endif // !defined(AFX_SG_E4438C_H__7B5679AE_6981_4B91_9539_1305E7E049DD__INCLUDED_)
