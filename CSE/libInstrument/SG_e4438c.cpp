// SG_e4428c.cpp: implementation of the SG_e4428c class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SG_e4438c.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__ ;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SG_e4438c::SG_e4438c() : m_isInitialized(false)
{

}

SG_e4438c::~SG_e4438c()
{

}


bool SG_e4438c::Init(ViRsrc rsrcName) {
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

bool SG_e4438c::Reset() {
	return Print("*RST");
}

bool SG_e4438c::SetCW(double freqHz) {
	return Print("FREQ:CW %.0f Hz", freqHz);
}

bool SG_e4438c::GetCW(double &freqHz) {
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

bool SG_e4438c::SetPL(double dbm) {
	double freqHz;
	if (!GetCW(freqHz)) {
		return false;
	}
	if (m_isEnableCableLoss) {
		dbm -= m_cableLossData.GetData(freqHz, dbm);
	}
	if (m_isEnableInstrOffset) {
		dbm -= m_instrOffset.GetData(freqHz, dbm);
	}
	TRACE("Set PL %.0f MHz %.4f dBm\n", freqHz / 1e6, dbm);
	return Print("POW %f dbm", dbm);
}

bool SG_e4438c::GetPL(double &dbm) {
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

bool SG_e4438c::SetOutputEnable(bool bEnable) {
	if (bEnable) {
		return Print("OUTP ON");
	} else {
		return Print("OUTP OFF");
	}
}

bool SG_e4438c::SetModEnable(bool bEnable) {
	if (bEnable) {
		return Print("OUTP:MOD ON");
	} else {
		return Print("OUTP:MOD OFF");
	}
}

bool SG_e4438c::LoadOffset() {
	CString fileName = "./SG_E4438C_" + m_SN + ".xml";
	CIniFile::GetCurPath(fileName);
	m_instrOffset.Load(fileName);
	return true;
}

bool SG_e4438c::SaveOffset(CInstrOffsetData &data, bool isImport) {
	if (isImport) {
		m_instrOffset = data;
	}
	CString fileName = "./SG_E4438C_" + m_SN + ".xml";
	CIniFile::GetCurPath(fileName);
	return data.Save(fileName);
}

bool SG_e4438c::ImportCableLoss(CInstrOffsetData *pOffset) {
	m_cableLossData = *pOffset;
	return true;
}

