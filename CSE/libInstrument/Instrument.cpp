// Instrument.cpp: implementation of the CInstrument class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Instrument.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const char *getInstVer() {
	extern const char *g_SVNRevision;
	return g_SVNRevision;
}

#define INSTR_CHECK(flag, msg)          \
	if (!(flag)) {				        \
		if (m_isEnableExpt) {           \
			throw CInstrException(msg); \
			return false;               \
		}                               \
		return false;                   \
	}

/*CInstrument CInstrument::m_instance;*/

CInstrument::CInstrument()
	: m_saIdx(-1)
	, m_pmIdx(-1)
	, m_sgIdx(-1)
	, m_isEnableExpt(true)
{

	m_powerMeters.push_back(new PM_rsnrp);

	m_spectrumAnalyzers.push_back(new SA_e4440a);
	m_spectrumAnalyzers.push_back(new SA_n9030a);
	m_spectrumAnalyzers.push_back(new SA_n9020a);
	m_spectrumAnalyzers.push_back(new SA_e4445a);
	m_spectrumAnalyzers.push_back(new SA_e4443a);
	m_spectrumAnalyzers.push_back(new SA_n9010a);

	m_signalGenerators.push_back(new SG_e4438c);
	m_signalGenerators.push_back(new SG_n5182b);
	m_signalGenerators.push_back(new SG_e8267d);
	m_signalGenerators.push_back(new SG_n5182a);
}

CInstrument::~CInstrument()
{
	int i, vectorSize;

	vectorSize = m_spectrumAnalyzers.size();
	for (i = 0; i < vectorSize; i++) {
		if (m_spectrumAnalyzers[i] != NULL) {
			delete m_spectrumAnalyzers[i];
			m_spectrumAnalyzers[i] = NULL;
		}
	}

	vectorSize = m_powerMeters.size();
	for (i = 0; i < vectorSize; i++) {
		if (m_powerMeters[i] != NULL) {
			delete m_powerMeters[i];
			m_powerMeters[i] = NULL;
		}
	}

	vectorSize = m_signalGenerators.size();
	for (i = 0; i < vectorSize; i++) {
		if (m_signalGenerators[i] != NULL) {
			delete m_signalGenerators[i];
			m_signalGenerators[i] = NULL;
		}
	}
}

CInstrument &CInstrument::Instance() {
	static CInstrument m_instance;
	return m_instance;
}

void CInstrument::SetEnableExpt(bool isEnable) {
	m_isEnableExpt = isEnable;
}

bool CInstrument::Init() {
	ViFindList findlist;
	ViUInt32   retCnt, ret;
	ViChar     instrDesc[256];
	ViChar     instrumentName[256];
	ViSession  hVisa;

	if ((viOpenDefaultRM(&m_defaultRM) < VI_SUCCESS)) {
		if (m_isEnableExpt) {
			throw CInstrException("viOpenDefaultRM failed");
		}
		return false;
	}

// 	if (viFindRsrc(m_defaultRM, "(RSNRP|GPIB)?*INSTR", &findlist, &retCnt, instrDesc) < VI_SUCCESS) {
// 		viClose(m_defaultRM);
// 		return true;
// 	}
	if (viFindRsrc(m_defaultRM, "GPIB?*INSTR", &findlist, &retCnt, instrDesc) < VI_SUCCESS) {
		viClose(m_defaultRM);
		return true;
	}
	while (retCnt--) {
		if (viOpen(m_defaultRM, instrDesc, VI_NULL, VI_NULL, &hVisa) < VI_SUCCESS) {
			viFindNext(findlist, instrDesc);
			continue;
		}
		if (viWrite(hVisa, (ViBuf)"*IDN?", 5, &ret) < VI_SUCCESS) {
			viClose(hVisa);
			viFindNext(findlist, instrDesc);
			continue;
		}
		if (viRead(hVisa, (ViPBuf)instrumentName, 256, &ret) < VI_SUCCESS) {
			viClose(hVisa);
			viFindNext(findlist, instrDesc);
			continue;
		}
		viClose(hVisa);
		instruFactory(instrumentName, instrDesc);
		viFindNext(findlist, instrDesc);
	}
	viClose(findlist);
	viClose(m_defaultRM);


	ViInt32   lNumSensors  = 0;
	char   szDriverRev[   256 ];
	char   szFirmwareRev[ 256 ];
	char   szSensorName[  256 ];
	char   szSensorType[  256 ];
	char   szSensorSN[    256 ];
 	rsnrpz_GetSensorCount(0, &lNumSensors );

// 	if ( lNumSensors < 1 )
// 	{
// 		printf( "No Sensor found.\n" );
// 		return 0;
// 	}
	CString no;
	no.Format("找到%d个sensor",lNumSensors);
	AfxMessageBox(no);
	for( long lSensor = 1; lSensor <= lNumSensors; lSensor++ )
	{
		long lStat = 0;
		ViBoolean isMeasComplete;
		rsnrpz_GetSensorInfo( 0, lSensor, szSensorName, szSensorType, szSensorSN );
// 		do {
// 			Sleep (50);
// 			rsnrpz_chan_isMeasurementComplete(m_hVisa, 1, &isMeasComplete);
// 		} while ((isMeasComplete == VI_FALSE));
		AfxMessageBox(szSensorName);
// 		lStat = rsnrpz_init( szSensorName, 1, 1, &hVisa );
// 		do {
// 			Sleep (50);
// 			rsnrpz_chan_isMeasurementComplete(m_hVisa, 1, &isMeasComplete);
// 		} while ((isMeasComplete == VI_FALSE));

		if ( lStat == NRP_ERROR_SENSOR_IN_USE )
			AfxMessageBox( "in use\n" );
		else {
			if ( (lStat == NRP_SUCCESS) && (hVisa != 0) )
				instruFactory("ROHDE&SCHWARZ,NRP",szSensorName);
		}
	}

	return true;
}

bool CInstrument::HasSpectrumAnalyzer() {
	INSTR_CHECK(m_saIdx != -1, "未找到频谱仪");
	return true;
}

bool CInstrument::HasPowerMeter() {
	INSTR_CHECK(m_pmIdx != -1, "未找到功率计");
	return true;
}

bool CInstrument::HasSignalGenerator() {
	INSTR_CHECK(m_sgIdx != -1, "未找到信号源");
	return true;
}

bool CInstrument::instruFactory(LPCTSTR instrID, ViRsrc rsrcName) {
	int i, vectorSize;

	vectorSize = m_spectrumAnalyzers.size();
	for (i = 0; i < vectorSize; i++) {
		if (strstr(instrID, m_spectrumAnalyzers[i]->GetInstrumentDescription()) != 0) {
			if (!m_spectrumAnalyzers[i]->Init(rsrcName)) {
				continue;
			} else {
				m_saIdx = i;
				return true;
			}
		}
	}

	vectorSize = m_signalGenerators.size();
	for (i = 0; i < vectorSize; i++) {
		if (strstr(instrID, m_signalGenerators[i]->GetInstrumentDescription()) != 0) {
			if (!m_signalGenerators[i]->Init(rsrcName)) {
				continue;
			} else {
				m_sgIdx = i;
				return true;
			}
		}
	}

	vectorSize = m_powerMeters.size();
	for (i = 0; i < vectorSize; i++) {
		if (strstr(instrID, m_powerMeters[i]->GetInstrumentDescription()) != 0) {
			if (!m_powerMeters[i]->Init(rsrcName)) {
				continue;
			} else {
				m_pmIdx = i;
				return true;
			}
		}
	}
	return false;
}

bool CInstrument::SG_Reset() {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->Reset(), "信号源复位失败");
	return true;
}

bool CInstrument::SG_SetOutputEnable(bool bEnable) {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->SetOutputEnable(bEnable), "信号源设置开关失败");
	return true;
}

bool CInstrument::SG_SetModEnable(bool bEnable) {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->SetModEnable(bEnable), "信号源设置调制开关失败");
	return true;
}

bool CInstrument::SG_SetPL(double dbm) {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->SetPL(dbm), "信号源设置功率失败");
	return true;
}

bool CInstrument::SG_ImportCableLoss(LPCTSTR filePath) {
	BOOL_CHECK(HasSignalGenerator());
	CInstrOffsetData data;
	INSTR_CHECK(data.Load(filePath), "打开线损文件失败");
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->ImportCableLoss(&data), "信号源导入线损失败");
	return true;
}

bool CInstrument::SG_LoadOffset() {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->LoadOffset(), "信号源导入补偿数据失败");
	return true;
}

bool CInstrument::SG_SaveOffset(CInstrOffsetData &data, bool isImport) {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->SaveOffset(data, isImport), "信号源保存补偿数据失败");
	return true;
}

bool CInstrument::SG_SetEnableOffset(bool isEnableInstrOffset, bool isEnableCableLoss) {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->SetEnableOffset(isEnableInstrOffset, isEnableCableLoss), "信号源设置补偿数据使能失败");
	return true;
}

bool CInstrument::SG_SetCW(double freqHz) {
	BOOL_CHECK(HasSignalGenerator());
	INSTR_CHECK(m_signalGenerators[m_sgIdx]->SetCW(freqHz), "信号源设置频率失败");
	return true;
}

bool CInstrument::SA_Reset() {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->Reset(), "频谱仪复位失败");
	return true;
}

bool CInstrument::SA_SetCF(double freqHz) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetCF(freqHz), "频谱仪设置中心频率失败");
	return true;
}

bool CInstrument::SA_SetSpan(double freqHz) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetSpan(freqHz), "频谱仪设置SPAN失败");
	return true;
}

bool CInstrument::SA_SetRBW(double freqHz, bool isAuto) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetRBW(freqHz, isAuto), "频谱仪设置RBW失败");
	return true;
}

bool CInstrument::SA_SetVBW(double freqHz, bool isAuto) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetVBW(freqHz, isAuto), "频谱仪设置RBW失败");
	return true;
}

bool CInstrument::SA_SetDectector(DetectorTypes detType) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetDectector(detType), "频谱仪设置平均失败");
	return true;
}

bool CInstrument::SA_SetPeakSearch(PeakSearchTypes type) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetPeakSearch(type), "频谱仪设置Peak Search失败");
	return true;
}

bool CInstrument::SA_SetRefLevel(double dbm) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetRefLevel(dbm), "频谱仪设置参考电平失败");
	return true;
}

bool CInstrument::SA_SetRefAuto() {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetRefAuto(), "频谱仪设置参考电平失败");
	return true;
}

bool CInstrument::SA_SetMarker(MarkerTypes type) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetMarker(type), "频谱仪设置Marker失败");
	return true;
}

bool CInstrument::SA_GetMarkerPwr(double & dbm) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->GetMarkerPwr(dbm), "频谱仪读取Marker功率失败");
	return true;
}

bool CInstrument::SA_SetEnablePreamp(bool isEnable) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetEnablePreamp(isEnable), "频谱仪测试预放补偿失败");
	return true;
}

bool CInstrument::SA_GetMarkerFrequency(double &freqHz) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->GetMarkerFrequency(freqHz), "频谱仪读取Marker频率失败");
	return true;
}

bool CInstrument::SA_MarkerMove(MarkerMoveTypes type) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->MarkerMove(type), "移动Marker失败");
	return true;
}

bool CInstrument::SA_MarkerMove(double freqHz) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->MarkerMove(freqHz), "移动Marker失败");
	return true;
}

bool CInstrument::SA_SweepOnce() {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SweepOnce(), "单次扫描失败");
	return true;
}

bool CInstrument::SA_SetCableLossFile(LPCTSTR filePath) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetCableLoss(filePath), "频谱仪导入线损失败");
	return true;
}

bool CInstrument::SA_SetEnableCableLoss(bool isEnableCableLoss) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	m_spectrumAnalyzers[m_saIdx]->SetEnableCableLoss(isEnableCableLoss);
	return true;
}

bool CInstrument::SA_SetCalibration(CalibrationTypes type) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetCalibration(type), "频谱仪校准失败");
	return true;
}

bool CInstrument::SA_SetAverTrace(bool isEnable, int traceCount) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetAverTrace(isEnable, traceCount), "平均扫描失败");
	return true;
}

bool CInstrument::SA_SetAverTraceAndGetData(int traceCount, int pointCount, double *data) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetAverTraceAndGetData(traceCount, pointCount, data), "平均扫描失败");
	return true;
}

bool CInstrument::SA_SetMechAtten(bool isAuto, int AttenValue) {
	BOOL_CHECK(HasSpectrumAnalyzer());
	INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->SetMechAtten(isAuto, AttenValue), "设置衰减器失败");
	return true;
}

bool CInstrument::PM_Reset() {
	BOOL_CHECK(HasPowerMeter());
	INSTR_CHECK(m_powerMeters[m_pmIdx]->Reset(), "功率计复位失败");
	return true;
}

bool CInstrument::Close() {
	if (m_pmIdx != -1) {
		INSTR_CHECK(m_powerMeters[m_pmIdx]->Close(), "功率计关闭失败");
		m_pmIdx = -1;
	}
// 	if (m_saIdx != -1) {
// 		INSTR_CHECK(m_spectrumAnalyzers[m_saIdx]->Close(), "频谱仪关闭失败");
// 		m_saIdx = -1;
// 	}
// 	if (m_sgIdx != -1) {
// 		INSTR_CHECK(m_signalGenerators[m_sgIdx]->Close(), "信号源关闭失败");
// 		m_sgIdx = -1;
// 	}
	return true;
}

bool CInstrument::PM_SetFrequency(double freqHz) {
	BOOL_CHECK(HasPowerMeter());
	INSTR_CHECK(m_powerMeters[m_pmIdx]->SetFrequency(freqHz), "功率计设置频率失败");
	return true;
}

bool CInstrument::PM_GetPwr(double & dbm) {
	BOOL_CHECK(HasPowerMeter());
	INSTR_CHECK(m_powerMeters[m_pmIdx]->GetPwr(dbm), "功率计读取功率失败");
	printf("PM_GetPwr : %f\n", dbm);
	return true;
}

bool CInstrument::PM_GetPwr(double freqHz, double & dbm) {
	BOOL_CHECK(HasPowerMeter());
	INSTR_CHECK(m_powerMeters[m_pmIdx]->SetFrequency(freqHz), "功率计设置频率失败");
	INSTR_CHECK(m_powerMeters[m_pmIdx]->GetPwr(dbm), "功率计读取功率失败");
	return true;
}
