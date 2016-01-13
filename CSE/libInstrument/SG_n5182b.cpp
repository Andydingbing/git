// SG_n5182b.cpp: implementation of the SG_n5182b class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SG_n5182b.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SG_n5182b::SG_n5182b()
	: m_isInitialized(false)
	, m_dbm(-110)
{

}

SG_n5182b::~SG_n5182b()
{

}

bool SG_n5182b::Init(ViRsrc rsrcName) {
	if (m_isInitialized) {
		return true;
	}
	ViSession defaultRM;
	if (viOpenDefaultRM(&defaultRM) < VI_SUCCESS) {
		m_isInitialized;
		return false;
	}
	if (viOpen(defaultRM, rsrcName, NULL, 3000, &m_hVisa) < VI_SUCCESS) {
		m_isInitialized = false;
		viClose(m_defaultRM);
		return false;
	}
	viClose(m_defaultRM);
	char buffer[256];
	ZeroMemory(buffer, 256);
	if (!Print("*IDN?")) {
		viClose(m_hVisa);
		m_isInitialized = false;
	}
	if (!Read(buffer, 256)) {
		viClose(m_hVisa);
		m_isInitialized = false;
	}
	int i = 0;

	char *pSign;
	if ((pSign =  strstr(buffer, "MY")) == NULL) {
		if ((pSign = strstr(buffer, "US")) == NULL) {
			return false;
		}
	}
	
	while (pSign[i] != ',') {
		m_SN += pSign[i];
		i++;
	}

	LoadOffset();
	m_isInitialized = true;
	return true;
}

bool SG_n5182b::Reset() {
	return Print("*RST");
}

bool SG_n5182b::SetCW(double freqHz) {
	if (!Print("FREQ:CW %.0f Hz", freqHz)) {
		return false;
	}
	if (!SetPL(m_dbm)) {
		return false;
	}
	return true;
}

bool SG_n5182b::GetCW(double &freqHz) {
	char buffer[512];
	ZeroMemory(buffer, 512);
	if (!Print("FREQ:CW?")) {
		return false;
	}
	if (!Read(buffer, 512)) {
		return false;
	}
	freqHz = atof(buffer);
	return true;
}

bool SG_n5182b::SetPL(double dbm) {
	double freqHz;
	m_dbm = dbm;
	if (!GetCW(freqHz)) {
		return false;
	}
	if (m_isEnableCableLoss) {
		dbm -= m_cableLossData.GetData(freqHz, dbm);
	}
	if (m_isEnableInstrOffset) {
		dbm -= m_instrOffset.GetData(freqHz, dbm);
	}
	return Print("POW %f dbm", dbm);
}

bool SG_n5182b::GetPL(double &dbm) {
	char buffer[512];
	ZeroMemory(buffer, 512);
	if (!Print("POW?")) {
		return false;
	}
	if (!Read(buffer, 512)) {
		return false;
	}
	dbm = atof(buffer);
	return true;
}

bool SG_n5182b::SetOutputEnable(bool bEnable) {
	if (bEnable) {
		return Print("OUTP ON");
	} else {
		return Print("OUTP OFF");
	}
}

bool SG_n5182b::SetModEnable(bool bEnable) {
	if (bEnable) {
		return Print("OUTP:MOD ON");
	} else {
		return Print("OUTP:MOD OFF");
	}
}

bool SG_n5182b::LoadOffset() {
	CString fileName = "./SG_n5182b_" + m_SN + ".xml";
	CIniFile::GetCurPath(fileName);
	m_instrOffset.Load(fileName);
	return true;
}

bool SG_n5182b::SaveOffset(CInstrOffsetData &data, bool isImport) {
	if (isImport) {
		m_instrOffset = data;
	}
	CString fileName = "./SG_n5182b_" + m_SN + ".xml";
	CIniFile::GetCurPath(fileName);
	return data.Save(fileName);
}

bool SG_n5182b::ImportCableLoss(CInstrOffsetData *pOffset) {
	m_cableLossData = *pOffset;
	return true;
}

