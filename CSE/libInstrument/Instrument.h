// Instrument.h: interface for the CInstrument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSTRUMENT_H__47803468_2180_49D8_8E39_069E4289D885__INCLUDED_)
#define AFX_INSTRUMENT_H__47803468_2180_49D8_8E39_069E4289D885__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

using namespace std;

#include "IVisa.h"
#include "SpectrumAnalyzer.h"
#include "SignalGenerator.h"
#include "PowerMeter.h"
#include "PM_rsnrp.h"
#include "NrpControl2.h"
#include "SA_e4440a.h"
#include "SA_n9030a.h"
#include "SG_e4438c.h"
#include "SG_n5182b.h"
#include "SG_e8267d.h"
#include "SG_n5182a.h"
#include "InstrProperty.h"

#ifdef __cplusplus
extern "C" {
#endif
	AFX_EXT_API extern const char *getInstVer();
#ifdef __cplusplus
};
#endif

#define Instrument CInstrument::Instance()

class AFX_CLASS_EXPORT CInstrException {
public:
	CInstrException(LPCTSTR format, ...) {
		ZeroMemory(m_buffer, 2048);
		va_list ap;
		va_start(ap, format);
		vsprintf(m_buffer, format, ap);
		va_end(ap);
	}
	CString GetMessage() {
		return CString(m_buffer);
	}
private:
	char m_buffer[2048];
};

class AFX_EXT_CLASS CInstrument : public IVisa  
{
protected:
	CInstrument();
	CInstrument(CInstrument &) {}
/*	static CInstrument m_instance;*/
public:
	virtual ~CInstrument();
	static CInstrument &Instance();
	void SetEnableExpt(bool isEnable);
public:
	bool	Init();
	bool    Close();
	bool	HasSpectrumAnalyzer();
	bool	HasPowerMeter();
	bool	HasSignalGenerator();
public:
	// Singal Generator Methods
	bool	SG_Reset();
	bool	SG_SetOutputEnable(bool bEnable = true);
	bool	SG_SetModEnable(bool bEnable = true);
	bool	SG_SetCW(double freq);
	bool	SG_SetPL(double dbm);
	bool	SG_ImportCableLoss(LPCTSTR filePath);
	bool	SG_LoadOffset();
	bool	SG_SaveOffset(CInstrOffsetData &data, bool isImport = true);
	bool	SG_SetEnableOffset(bool isEnableInstrOffset, bool isEnableCableLoss);
public:
	// Spectrum Analyzer Methods
	bool	SA_Reset();
	bool	SA_SetCF(double freqHz);
	bool	SA_SetSpan(double freqHz);
	bool	SA_SetRBW(double freqHz, bool isAuto = false);
	bool    SA_SetVBW(double freqHz, bool isAuto = false);
	bool	SA_SetDectector(DetectorTypes detType);
	bool	SA_SetPeakSearch(PeakSearchTypes type = PeakHighest);
	bool    SA_SetRefLevel(double dbm);
	bool	SA_SetRefAuto();
	bool	SA_SetMarker(MarkerTypes type = MarkerDelta);
	bool	SA_GetMarkerPwr(double & dbm);
	bool    SA_SetEnablePreamp(bool isEnable);
	bool    SA_GetMarkerFrequency(double &freqHz);
	bool	SA_MarkerMove(MarkerMoveTypes type);
	bool    SA_MarkerMove(double freqHz);
	bool	SA_SweepOnce();
	bool    SA_SetCableLossFile(LPCTSTR filePath);
	bool    SA_SetEnableCableLoss(bool isEnableCableLoss);
	bool    SA_SetCalibration(CalibrationTypes type);
	bool    SA_SetAverTrace(bool isEnable, int traceCount = -1);
	bool    SA_SetAverTraceAndGetData(int traceCount, int pointCount, double *data);
	bool    SA_SetMechAtten(bool isAuto, int AttenValue = 0);
public:
	// Power Meter Methods
	bool	PM_Reset();
	bool	PM_SetFrequency(double freqHz);
	bool	PM_GetPwr(double &dbm);
	bool	PM_GetPwr(double freqHz, double &dbm);
private:
	bool	instruFactory(LPCTSTR instrID, ViRsrc rsrcName);
private:
	vector<ISpectrumAnalyzer *>	m_spectrumAnalyzers;
	int m_saIdx;
	
	vector<IPowerMeter *>			m_powerMeters;
	int	m_pmIdx;
	
	vector<ISignalGenerator *>	m_signalGenerators;
	int m_sgIdx;
private:
	bool          m_isEnableExpt;
};

#endif // !defined(AFX_INSTRUMENT_H__47803468_2180_49D8_8E39_069E4289D885__INCLUDED_)
