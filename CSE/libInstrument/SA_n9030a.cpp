// SA_n9030a.cpp: implementation of the SA_n9030a class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SA_n9030a.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SA_n9030a::SA_n9030a()
	: m_isInitialized(false)
{

}

SA_n9030a::~SA_n9030a()
{

}

bool SA_n9030a::Init(ViRsrc rsrcName) {
	if (m_isInitialized) {
		return true;
	}
	if ((viOpenDefaultRM(&m_defaultRM) < VI_SUCCESS)) {
		return false;
	}
	if (viOpen(m_defaultRM, rsrcName, VI_NULL, 50000, &m_hVisa) < VI_SUCCESS) {
		m_isInitialized = false;
	//	viClose(m_defaultRM);
		return false;
	}
	viSetAttribute(m_hVisa, VI_ATTR_TMO_VALUE, 50000);
//	viClose(m_defaultRM);
	m_isInitialized = true;
	return true;
}

bool SA_n9030a::Reset() {
	return Print("*RST");
}

bool SA_n9030a::SetCF(double freqHz) {
	return Print("FREQ:CENT %.0f", freqHz);
}

bool SA_n9030a::SetSpan(double freqHz) {
	return Print("FREQ:SPAN %.0f", freqHz);
}

bool SA_n9030a::SetRBW(double freqHz, bool isAuto) {
	if (isAuto) {
		return Print("BAND:AUTO ON");
	}
	return Print("BAND %.0f Hz", freqHz);
}

bool SA_n9030a::SetVBW(double freqHz, bool isAuto) {
	if (isAuto) {
		return Print("BAND:VID:AUTO ON");
	}
	return Print("BAND:VID %.0f Hz", freqHz);
}

bool SA_n9030a::SetDectector(DetectorTypes detType) {
	switch (detType) {
		case DetAverage:
			return Print("DET RMS");
		case DetNormal:
			return Print("DET NORM");
		case DetPeak:
			return Print("DET POS");
		case DetAUTO:
			return Print("DET:AUTO ON");
		case DetSample:
			return Print("DET SAMP");
		case DetNegPeak:
			return Print("DET NEG");
	}
	return false;
}

bool SA_n9030a::SetPeakSearch(PeakSearchTypes type) {
	switch (type) {
	case PeakHighest:   return Print("CALC:MARK1:MAX");
	case PeakNextHigh:  return Print("CALC:MARK1:MAX:NEXT");
	case PeakNextRight: return Print("CALC:MARK1:MAX:RIGH");
	case PeakNextLeft:  return Print("CALC:MARK1:MAX:LEFT");
	}
	return false;
}

bool SA_n9030a::SetRefLevel(double dbm) {
	return Print("DISP:WIND:TRAC:Y:RLEV %f dbm", dbm);
}

bool SA_n9030a::GetRefLevel(double &dbm) {
	char dbmBuffer[200];
	ZeroMemory(dbmBuffer, 200);
	if (!Print("DISP:WIND:TRAC:Y:RLEV?")) { return false; }
	if (!Read(dbmBuffer, 200))   { return false; }
	dbm = atof(dbmBuffer);
	return true;
}

bool SA_n9030a::SetMarker(MarkerTypes type) {
	switch (type) {
	case MarkerOff:    return Print("CALC:MARK1:STAT OFF");
	case MarkerNormal: return Print("CALC:MARK1:MODE POS");
	case MarkerDelta:  return Print("CALC:MARK1:MODE DELT");
	}
	return false;
}

bool SA_n9030a::SetEnablePreamp(bool isEnable) {
	if (isEnable) {
		if (!Print(":POW:GAIN:STAT ON;:POW:GAIN:BAND FULL")) { return false; }
	} else {
		if (!Print("POW:GAIN:STAT OFF")) { return false; }
	}

	return true;
}

bool SA_n9030a::GetMarkerPwr(double &dbm) {
	char dbmBuffer[200];
	double freqHz;

	ZeroMemory(dbmBuffer, 200);
	BOOL_CHECK(Print("CALC:MARK1:Y?"));
	BOOL_CHECK(Read(dbmBuffer, 200));
	BOOL_CHECK(GetMarkerFrequency(freqHz));

	dbm = atof(dbmBuffer);

	if (m_isEnableCableLoss) {
		dbm -= m_cableLossData.GetData(freqHz, dbm);
	}
	return true;
}

bool SA_n9030a::GetMarkerFrequency(double &freqHz) {
	char freqBuffer[200];
	ZeroMemory(freqBuffer, 200);
	BOOL_CHECK(Print("CALC:MARK1:X?"));
	BOOL_CHECK(Read(freqBuffer, 200));
	freqHz = atof(freqBuffer);
	return true;
}

bool SA_n9030a::WaitForContinue() {
	return Print("*WAI");
}

bool SA_n9030a::SweepOnce() {
	char buffer[200];
	BOOL_CHECK(Print("INIT:IMM"));
	BOOL_CHECK(Print("*OPC?"));
	BOOL_CHECK(Read(buffer, 200, 20000));
	return true;
}

bool SA_n9030a::MarkerMove(MarkerMoveTypes type) {
	switch (type) {
	case MarkerToCF:     return Print("CALC:MARK1:CENT");
	case MarkerToRefLvl: return Print("CALC:MARK1:RLEV");
	}
	return false;
}

bool SA_n9030a::MarkerMove(double freqHz) {
	return Print("CALC:MARK1:X %f HZ", freqHz);
}

bool SA_n9030a::SetCalibration(CalibrationTypes type) {
	switch (type) {
		case AUTO_OFF : return Print("CAL:AUTO OFF");
		case AUTO_ON  : return Print("CAL:AUTO ON");
		case CAL_NOW  : return Print("CAL:ALL");
		default: return false;
	}
}

bool SA_n9030a::SetAverTrace(bool isEnable, int traceCount) {
	if (isEnable) {
		BOOL_CHECK(Print("AVER:COUN %d", traceCount));
		BOOL_CHECK(Print("AVER ON"));
	} else {
		BOOL_CHECK(Print("AVER OFF"));
	}
	return true;
}

bool SA_n9030a::SetAverTraceAndGetData(int traceCount, int pointCount, double *data) {
	int bufferSize = 18 * pointCount;
	char *buffer = new char[bufferSize];
	BOOL_CHECK(Print("SWE:POIN %d", pointCount));
	BOOL_CHECK(SetAverTrace(true, traceCount));
	BOOL_CHECK(SweepOnce());
	BOOL_CHECK(Print("FORM ASCii"));
	BOOL_CHECK(Print("TRAC? TRACE1"));
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

bool SA_n9030a::SetMechAtten(bool isAuto, int AttenValue) {
	if (isAuto) {
		BOOL_CHECK(Print("POW:ATT:AUTO ON"));
	} else {
		BOOL_CHECK(Print("POW:ATT:AUTO OFF"));
		BOOL_CHECK(Print("POW:ATT %d", AttenValue));
	}
	return true;
}