// PM_rsnrp.cpp: implementation of the PM_rsnrp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PM_rsnrp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#if defined NRP_DEBUG
#define NRP_CHECK(func)                                \
	do {                                               \
		ViStatus ret;                                  \
		char errMsg[256];                              \
		ZeroMemory(errMsg, 256);                       \
		if ((ret = (func)) != 0) {                     \
			FILE *fp;                                  \
			fopen_s(&fp, "C:\\nrp.log", "a");          \
			rsnrp_error_message(m_hVisa, ret, errMsg); \
			fprintf(fp, "F : %s\n\t%s\n", #func, errMsg);   \
		} else {                                       \
			printf("T : %s\n", #func);                 \
		}                                              \
	} while (0);
#else
#define NRP_CHECK(func) func;
#endif

PM_rsnrp::PM_rsnrp() : m_isInitialized(false)
{
	
}

PM_rsnrp::~PM_rsnrp()
{
//	Close();
	m_hVisa = 0;
}

bool PM_rsnrp::Init(ViRsrc rsrcName) {
//	char     sensorString[256];
	uint32_t usbid = 0, serialNumber = 0;
	ViBoolean isMeasComplete;
//	sscanf_s(rsrcName, "RSNRP::%x::%d::INSTR", &usbid, &serialNumber);
// 	if (usbid == 0 || serialNumber == 0) {
// 		return false;
// 	}
//	ZeroMemory(sensorString, 256);
//	sprintf_s(sensorString, 256, "USB::0x0AAD::0x%04x::%d", usbid, serialNumber);
//	NRP_CHECK(rsnrpz_init(sensorString, FALSE, FALSE, &m_hVisa));
	NRP_CHECK(rsnrpz_init(rsrcName, FALSE, FALSE, &m_hVisa));
	Sleep(100);
// 	do {
// 		Sleep (50);
// 		rsnrpz_chan_isMeasurementComplete(m_hVisa, 1, &isMeasComplete);
// 	} while ((isMeasComplete == VI_FALSE));
	m_isInitialized = true;
	return true;
}

bool PM_rsnrp::Reset() {
	ViBoolean isMeasComplete;
	NRP_CHECK(rsnrpz_status_preset(m_hVisa));
	NRP_CHECK(rsnrpz_avg_setAutoEnabled(m_hVisa, 1, VI_FALSE));
	NRP_CHECK(rsnrpz_avg_setCount(m_hVisa, 1, 3));
	NRP_CHECK(rsnrpz_trigger_setSource(m_hVisa, 1, RSNRPZ_TRIGGER_SOURCE_IMMEDIATE));
	// NRP_CHECK(rsnrp_system_setSpeed(m_hVisa, RSNRP_SPEED_FAST));
	// this always return "The given session or object reference is invalid"
//	rsnrp_system_setSpeed(m_hVisa, RSNRP_SPEED_FAST); 
	NRP_CHECK(rsnrpz_reset(m_hVisa));
	NRP_CHECK(rsnrpz_chans_initiate(m_hVisa));
	do {
		Sleep (50);
		NRP_CHECK(rsnrpz_chan_isMeasurementComplete(m_hVisa, 1, &isMeasComplete));
	} while ((isMeasComplete == VI_FALSE));
	AfxMessageBox("reset complete");
	Sleep(100);
	return true;
}

bool PM_rsnrp::SetFrequency(double freqHz) {
	NRP_CHECK(rsnrpz_chan_setCorrectionFrequency(m_hVisa, 1, freqHz));
	return true;
}

bool PM_rsnrp::GetPwr(double & dbm) {
	double nrpResult;
	ViBoolean isMeasComplete;

	NRP_CHECK(rsnrpz_chan_initiate(m_hVisa, 1));
	do {
		Sleep (50);
		NRP_CHECK(rsnrpz_chan_isMeasurementComplete(m_hVisa, 1, &isMeasComplete));
	} while ((isMeasComplete == VI_FALSE));

	NRP_CHECK(rsnrpz_meass_readMeasurement(m_hVisa, 1, 10000, &nrpResult));
	dbm = 10 * log10(fabs(nrpResult * 1000));
	return true;	
}

bool PM_rsnrp::GetPwr(double freqHz,double &dbm) {
	if (!SetFrequency(freqHz)) { return false; }
	if (!GetPwr(dbm))          { return false; }
	return true;
}

bool PM_rsnrp::Close() {
	if (m_hVisa != 0) {
		rsnrpz_status_preset(m_hVisa);
		rsnrpz_CloseSensor(m_hVisa, 1);
		rsnrpz_close(m_hVisa);
		m_hVisa = 0;
	}
	return true;
}