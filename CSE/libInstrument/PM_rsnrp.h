// PM_rsnrp.h: interface for the PM_rsnrp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PM_RSNRP_H__1FF61833_FB6E_4391_A91B_2C2A66935001__INCLUDED_)
#define AFX_PM_RSNRP_H__1FF61833_FB6E_4391_A91B_2C2A66935001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>

#include "PowerMeter.h"
#include "rsnrp.h"
#include "rsnrpz.h"

class PM_rsnrp : public IPowerMeter
{
public:
	PM_rsnrp();
	virtual ~PM_rsnrp();
	virtual int			GetDefaultPriAddr() { return -1; }
	virtual int			GetDefaultSecAddr() { return -1; }
	virtual LPCTSTR		GetInstrumentDescription() { return "ROHDE&SCHWARZ,NRP"; }
	virtual bool		Init(ViRsrc rsrcName);
public:
	virtual bool		Reset();
	virtual bool		SetFrequency(double freqHz);
	virtual bool		GetPwr(double & dbm);
	virtual bool		GetPwr(double freqHz, double &dbm);
	virtual bool        Close();
private:
	bool m_isInitialized;
};

#endif // !defined(AFX_PM_RSNRP_H__1FF61833_FB6E_4391_A91B_2C2A66935001__INCLUDED_)
