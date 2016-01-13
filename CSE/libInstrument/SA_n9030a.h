// SA_n9030a.h: interface for the SA_n9030a class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SA_N9030A_H__7B162184_A09B_4D14_A201_46F0BE2ADDAE__INCLUDED_)
#define AFX_SA_N9030A_H__7B162184_A09B_4D14_A201_46F0BE2ADDAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SpectrumAnalyzer.h"

class SA_n9030a : public ISpectrumAnalyzer  
{
public:
	SA_n9030a();
	virtual ~SA_n9030a();
	virtual int		GetDefaultPriAddr() { return -1; }
	virtual int		GetDefaultSecAddr() { return -1; }
	virtual LPCTSTR	GetInstrumentDescription() { return "N9030A"; }
	virtual bool	Init(ViRsrc rsrcName);
	
public:
	virtual bool	Reset();
	virtual bool	SetCF(double freqHz);
	virtual bool	SetSpan(double freqHz);
	virtual bool	SetRBW(double freqHz, bool isAuto);
	virtual bool	SetVBW(double freqHz, bool isAuto);
	virtual bool	SetDectector(DetectorTypes detType);
	virtual bool	SetPeakSearch(PeakSearchTypes type);
	virtual bool	SetRefLevel(double dbm);
	virtual bool	GetRefLevel(double &dbm);
	virtual bool	SetMarker(MarkerTypes type);
	virtual bool	GetMarkerPwr(double &dbm);
	virtual bool	GetMarkerFrequency(double &freqHz);
	virtual bool	WaitForContinue();
	virtual bool	SweepOnce();
	virtual bool	MarkerMove(MarkerMoveTypes type);
	virtual bool    MarkerMove(double freqHz);
	virtual bool    SetCalibration(CalibrationTypes type);
	virtual bool    SetAverTrace(bool isEnable, int traceCount);
	virtual bool    SetAverTraceAndGetData(int traceCount, int pointCount, double *data);
	virtual bool    SetEnablePreamp(bool isEnable);
	virtual bool    SetMechAtten(bool isAuto, int AttenValue);
private:
	bool m_isInitialized;
};

class SA_n9020a : public SA_n9030a {
public:
	virtual LPCTSTR	GetInstrumentDescription() { return "N9020A"; }
};

class SA_n9010a : public SA_n9030a {
public:
	virtual LPCTSTR	GetInstrumentDescription() { return "N9010A"; }
};

#endif // !defined(AFX_SA_N9030A_H__7B162184_A09B_4D14_A201_46F0BE2ADDAE__INCLUDED_)
