// SpectrumAnalyzer.h: interface for the CSpectrumAnalyzer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPECTRUMANALYZER_H__62F307AD_5859_4BE4_A9A9_45A54FD4218F__INCLUDED_)
#define AFX_SPECTRUMANALYZER_H__62F307AD_5859_4BE4_A9A9_45A54FD4218F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IVisa.h"
#include "OperationDef.h"
#include "InstrOffsetData.h"

class ISpectrumAnalyzer : public IVisa
{
public:
	ISpectrumAnalyzer() : m_isEnableCableLoss(false) { }
	virtual			~ISpectrumAnalyzer()	{ }
	virtual int		GetDefaultPriAddr() = 0;
	virtual int		GetDefaultSecAddr() = 0;
	virtual LPCTSTR	GetInstrumentDescription() = 0;
	virtual bool	Init(ViRsrc rsrcName) = 0;

public:
	virtual bool	Reset() = 0;
	virtual bool	SetCF(double freqHz) = 0;
	virtual bool	SetSpan(double freqHz) = 0;
	virtual bool	SetRBW(double freqHz, bool isAuto) = 0;
	virtual bool    SetVBW(double freqHz, bool isAuto) = 0;
	virtual bool	SetDectector(DetectorTypes detType) = 0;
	virtual bool	SetPeakSearch(PeakSearchTypes type) = 0;
	virtual bool	SetMarker(MarkerTypes type) = 0;
	virtual void    SetEnableCableLoss(bool isEnableCableLoss) { m_isEnableCableLoss = isEnableCableLoss; }
	virtual bool	SetCableLoss(LPCTSTR cableLossFilePath)    { return m_cableLossData.Load(cableLossFilePath); }
	virtual bool    SetCalibration(CalibrationTypes type) = 0;
	virtual bool    SetAverTrace(bool isEnable, int traceCount) = 0;
	virtual bool	SetRefLevel(double dbm) = 0;
	virtual bool	GetRefLevel(double &dbm) = 0;
	virtual bool	GetMarkerPwr(double &dbm) = 0;
	virtual bool    SetEnablePreamp(bool isEnable) = 0;
	virtual bool	GetMarkerFrequency(double &freqHz) = 0;
	virtual bool	WaitForContinue() = 0;
	virtual bool	SweepOnce() = 0;
	virtual bool	MarkerMove(MarkerMoveTypes type) = 0;
	virtual bool	MarkerMove(double freqHz) = 0;
	virtual bool    SetAverTraceAndGetData(int traceCount, int pointCount, double *data) = 0;
	virtual bool    SetMechAtten(bool isAuto, int AttenValue) = 0;
	virtual bool	SetRefAuto() {
		double dfRefLevel;
		double dfPeakPower;
		do {
			BOOL_CHECK(SweepOnce());
			BOOL_CHECK(SetPeakSearch(PeakHighest));
			BOOL_CHECK(GetMarkerPwr(dfPeakPower));
			BOOL_CHECK(GetRefLevel(dfRefLevel));
			BOOL_CHECK(SetRefLevel((int)dfPeakPower + 5));
			BOOL_CHECK(SweepOnce());
			BOOL_CHECK(SetPeakSearch(PeakHighest));
			BOOL_CHECK(GetMarkerPwr(dfPeakPower));
		} while(dfPeakPower > dfRefLevel);
		Sleep(200);
		return true;
	}
protected:
	CInstrOffsetData m_cableLossData;
	bool             m_isEnableCableLoss;
};

#endif // !defined(AFX_SPECTRUMANALYZER_H__62F307AD_5859_4BE4_A9A9_45A54FD4218F__INCLUDED_)
