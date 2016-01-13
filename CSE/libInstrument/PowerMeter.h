// PowerMeter.h: interface for the CPowerMeter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POWERMETER_H__A141DA56_E79E_4D7B_9CD3_7D0AAB6510E9__INCLUDED_)
#define AFX_POWERMETER_H__A141DA56_E79E_4D7B_9CD3_7D0AAB6510E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IVisa.h"

class IPowerMeter : public IVisa
{
public:
	IPowerMeter()		{ }
	virtual				~IPowerMeter() { }
	virtual int			GetDefaultPriAddr() = 0;
	virtual int			GetDefaultSecAddr() = 0;
	virtual LPCTSTR		GetInstrumentDescription() = 0;
	virtual bool		Init(ViRsrc rsrcName) = 0;
public:
	virtual bool		Reset() = 0;
	virtual bool		SetFrequency(double freqHz) = 0;
	virtual bool		GetPwr(double & dbm) = 0;
	virtual bool		GetPwr(double freqHz, double &dbm) = 0;
	virtual bool        Close() = 0;
};

#endif // !defined(AFX_POWERMETER_H__A141DA56_E79E_4D7B_9CD3_7D0AAB6510E9__INCLUDED_)
