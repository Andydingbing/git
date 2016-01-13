// SA_n9030a.h: interface for the SA_n9030a class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SA_N9030A_H__7B162184_A09B_4D14_A201_46F0BE2ADDAE__INCLUDED_)
#define AFX_SA_N9030A_H__7B162184_A09B_4D14_A201_46F0BE2ADDAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SpectrumAnalyzer.h"

class SA_fsup : public ISpectrumAnalyzer  
{
public:
	SA_fsup();
	virtual ~SA_fsup();
	virtual int		GetDefaultPriAddr() { return -1; }
	virtual int		GetDefaultSecAddr() { return -1; }
	virtual LPCTSTR	GetInstrumentDescription() { return "fsup"; }
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
	virtual bool    SetCalibration();
	virtual bool    SetAverTrace(bool isEnable, int traceCount);
	virtual bool    SetAverTraceAndGetData(int traceCount, int pointCount, double *data);
	virtual bool    SetEnablePreamp(bool isEnable);
private:
	bool m_isInitialized;
};

#endif // !defined(AFX_SA_N9030A_H__7B162184_A09B_4D14_A201_46F0BE2ADDAE__INCLUDED_)
