// SA_fsup.cpp: implementation of the SA_fsup class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SA_fsup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SA_fsup::SA_fsup()
	: m_isInitialized(false)
{

}

SA_fsup::~SA_fsup()
{

}

bool SA_fsup::Init(ViRsrc rsrcName) {
	if (m_isInitialized) {
		return true;
	}
	if ((viOpenDefaultRM(&m_defaultRM) < VI_SUCCESS)) {
		return false;
	}
	if (viOpen(m_defaultRM, rsrcName, VI_NULL, 5000, &m_hVisa) < VI_SUCCESS) {
		m_isInitialized = false;
	//	viClose(m_defaultRM);
		return false;
	}
//	viClose(m_defaultRM);
	m_isInitialized = true;
	return true;
}

bool SA_fsup::Reset() {
	return Print("SYSTem:PRESet;*WAI");
}

bool SA_fsup::SetCF(double freqHz) {
	return Print("FREQ:CENT %.0f;*WAI", freqHz);
}

bool SA_fsup::SetSpan(double freqHz) {
	return Print("FREQ:SPAN %.0f;*WAI", freqHz);
}

bool SA_fsup::SetRBW(double freqHz, bool isAuto) {
	if (isAuto) {
		return Print("BAND:AUTO ON;*WAI");
	}
	return Print("BAND %.0f Hz;*WAI", freqHz);
}

bool SA_fsup::SetVBW(double freqHz, bool isAuto) {
	if (isAuto) {
		return Print("BAND:VID:AUTO ON;*WAI");
	}
	return Print("BAND:VID %.0f Hz;*WAI", freqHz);
}

bool SA_fsup::SetDectector(DetectorTypes detType) {
	switch (detType) {
		case DetAverage:
			return Print("DET RMS;*WAI");
		case DetNormal:
			return Print("DET APE;*WAI");
		case DetPeak:
			return Print("DET POS;*WAI");
		case DetAUTO:
			return Print("DET:AUTO ON;*WAI");
		case DetSample:
			return Print("DET SAMP;*WAI");
		case DetNegPeak:
			return Print("DET NEG;*WAI");
	}
	return false;
}

bool SA_fsup::SetPeakSearch(PeakSearchTypes type) {
	switch (type) {
	case PeakHighest:   return Print("CALC:MARK1:MAX;*WAI");
	case PeakNextHigh:  return Print("CALC:MARK1:MAX:NEXT;*WAI");
	case PeakNextRight: return Print("CALC:MARK1:MAX:RIGH;*WAI");
	case PeakNextLeft:  return Print("CALC:MARK1:MAX:LEFT;*WAI");
	}
	return false;
}

bool SA_fsup::SetRefLevel(double dbm) {
	return Print("DISP:WIND:TRAC:Y:RLEV %f dbm;*WAI", dbm);
}

bool SA_fsup::GetRefLevel(double &dbm) {
	char dbmBuffer[200];
	ZeroMemory(dbmBuffer, 200);
	if (!Print("DISP:WIND:TRAC:Y:RLEV?;*WAI")) { return false; }
	if (!Read(dbmBuffer, 200))   { return false; }
	dbm = atof(dbmBuffer);
	return true;
}

bool SA_fsup::SetMarker(MarkerTypes type) {
	// 
	switch (type) {
	case MarkerOff:    return Print("CALC:MARK1:STAT OFF;*WAI");
	case MarkerNormal: return Print("CALC:MARK1:MODE POS;*WAI");
	case MarkerDelta:  return Print("CALC:MARK1:MODE DELT;*WAI");
	}
	return false;
}

bool SA_fsup::SetEnablePreamp(bool isEnable) {
	double freqHz;
	if (!GetMarkerFrequency(freqHz))  { return false; }

	if (isEnable) {
		return Print("INP:GAIN:STAT ON;*WAI");
	} else {
		return Print("INP:GAIN:STAT OFF;*WAI");
	}

	return true;
}

bool SA_fsup::GetMarkerPwr(double &dbm) {
	char dbmBuffer[200];
	double freqHz;

	ZeroMemory(dbmBuffer, 200);
	BOOL_CHECK(Print("CALC:MARK1:Y?;*WAI"));
	BOOL_CHECK(Read(dbmBuffer, 200));
	BOOL_CHECK(GetMarkerFrequency(freqHz));

	dbm = atof(dbmBuffer);

	if (m_isEnableCableLoss) {
		dbm -= m_cableLossData.GetData(freqHz, dbm);
	}
	return true;
}

bool SA_fsup::GetMarkerFrequency(double &freqHz) {
	char freqBuffer[200];
	ZeroMemory(freqBuffer, 200);
	BOOL_CHECK(Print("CALC:MARK1:X?;*WAI"));
	BOOL_CHECK(Read(freqBuffer, 200));
	freqHz = atof(freqBuffer);
	return true;
}

bool SA_fsup::WaitForContinue() {
	return Print("*WAI");
}

bool SA_fsup::SweepOnce() {
	BOOL_CHECK(Print("INIT1:CONT OFF;*WAI;"));
	BOOL_CHECK(Print("INIT1:IMM;*WAI;"));
	return true;
}

bool SA_fsup::MarkerMove(MarkerMoveTypes type) {
	switch (type) {
	case MarkerToCF:     return Print("CALC:MARK1:FUNC:CENT;*WAI;");
	case MarkerToRefLvl: return Print("CALC:MARK1:FUNC:REF;*WAI;");
	}
	return false;
}

bool SA_fsup::MarkerMove(double freqHz) {
	return Print("CALC:MARK1:X %f HZ;*WAI", freqHz);
}

bool SA_fsup::SetCalibration() {
	return Print("CAL:ALL;*WAI");
}

bool SA_fsup::SetAverTrace(bool isEnable, int traceCount) {
	if (isEnable) {
		BOOL_CHECK(Print("AVER:COUN %d;*WAI", traceCount));
		BOOL_CHECK(Print("AVER:STAT ON;*WAI"));
	} else {
		BOOL_CHECK(Print("AVER:STAT OFF;*WAI"));
	}
	return true;
}

bool SA_fsup::SetAverTraceAndGetData(int traceCount, int pointCount, double *data) {
	int bufferSize = 18 * pointCount;
	char *buffer = new char[bufferSize];
	BOOL_CHECK(Print("SWE:POIN %d;*WAI", pointCount));
	BOOL_CHECK(SetAverTrace(true, traceCount));
	BOOL_CHECK(SweepOnce());
	BOOL_CHECK(Print("FORM ASCii;*WAI"));
	BOOL_CHECK(Print("TRAC? TRACE1;*WAI"));
	BOOL_CHECK(Read(buffer, bufferSize));

	int dataIdx = 0, lastIdx = 0;
	for (int i = 0; i < bufferSize; i++) {
		if (buffer[i] == ',' || i == bufferSize - 1) {
			buffer[i] = 0;
			data[dataIdx++] = atof(&buffer[lastIdx]);
			lastIdx = i + 1;
			i++;
		}
	}
	delete[] buffer;
	return true;
}
