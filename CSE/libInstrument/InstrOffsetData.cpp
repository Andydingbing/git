// InstrOffsetData.cpp: implementation of the CInstrOffsetData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InstrOffsetData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CInstrOffsetData::CInstrOffsetData()
{
	
}

CInstrOffsetData::~CInstrOffsetData()
{

}

CInstrOffsetData::CInstrOffsetData(M2 &data) {
	m_data = data;
}

bool CInstrOffsetData::Load(LPCTSTR filePath, bool isCreate) {
	m_data.clear();
	CMarkup xml;
	if (!xml.Load(filePath)) {
		return false;
	}
	if (!xml.FindElem("root")) {
		return false;
	}
	xml.IntoElem();
	if (!xml.FindElem("data")) {
		return false;
	}
	xml.IntoElem();
	while (xml.FindElem("frequency")) {
		double frequency = atof(xml.GetAttrib("value"));
		xml.IntoElem();
		while (xml.FindElem("power")) {
			double dest_power = atof(xml.GetAttrib("dest_power"));
			double offset     = atof(xml.GetAttrib("offset"));
			m_data[frequency][dest_power] = offset;
		}
		xml.OutOfElem();
	}
	return true;
}

CInstrOffsetData::M1 *CInstrOffsetData::findFreqTable(double freqHz) {
	if (m_data.find(freqHz) != m_data.end()) {                    // 找到频点, 直接返回
		return &m_data[freqHz];
	} else {                                              // 没有找到频点, 找相近的频点
		double abs_diff = 1e10;                                 // 记录频点最小绝对误差
		M1 *pPowerData = NULL;                 // 记录最小误差的功率表
		M2::iterator itOffset;
		for (itOffset = m_data.begin(); itOffset != m_data.end(); itOffset++) { // 遍历
			if (fabs(freqHz - itOffset->first) < abs_diff) {      // 找到最小误差的频点
				abs_diff = fabs(freqHz - itOffset->first);              // 记录最小误差
				pPowerData   = &(itOffset->second);         // 记录最小误差频点的功率表
			}
		}
		return pPowerData;                                  // 返回最小误差频点的功率表
	}
}

double CInstrOffsetData::findOffset(M1 *pPowerData, double dbm) {
	if (pPowerData == NULL) {
		return 0.0;
	}
	if (pPowerData->find(dbm) != pPowerData->end()) {                             // 找到功率点
		return (*pPowerData)[dbm];                                                  // 直接返回
	} else {                                                    // 找不到功率点, 找相近的功率点
		double abs_diff = 1000.0;                                     // 记录功率点最小绝对误差
		double offset   = 0;                                                    // 记录最小误差
		M1::iterator itPower;
		for (itPower = pPowerData->begin(); itPower != pPowerData->end(); itPower++) {  // 遍历
			if (fabs(dbm - itPower->first) < abs_diff) {                // 找到最小误差的功率点
				abs_diff = fabs(dbm - itPower->first);                          // 记录最小误差
				offset   = itPower->second;                       // 记录最小误差功率点的补偿值
			}
		}
		return offset;                                            // 返回最小误差功率点的补偿值
	}
}

double CInstrOffsetData::GetData(double freqHz, double dbm) {
	return findOffset(findFreqTable(freqHz), dbm);
}

bool CInstrOffsetData::SetData(double freqHz, double dbm, double offset) {
	m_data[freqHz][dbm] = offset;
	return true;
}

bool CInstrOffsetData::Save(LPCTSTR filePath) {
	CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
	xml.AddNode(CMarkup::MNT_ELEMENT, "root");
	xml.IntoElem();

	M2::iterator m2it;

	xml.AddNode(CMarkup::MNT_ELEMENT, "header");
	xml.SetAttrib("version", 1);

	xml.AddNode(CMarkup::MNT_ELEMENT, "data");
	xml.IntoElem();

	for (m2it = m_data.begin(); m2it != m_data.end(); m2it++) {
		CString frequency;
		frequency.Format("%f", m2it->first);
		xml.AddNode(CMarkup::MNT_ELEMENT, "frequency");
		xml.SetAttrib("value", frequency);
		xml.IntoElem();
		M1::iterator m1it;
		for (m1it = m2it->second.begin(); m1it != m2it->second.end(); m1it++) {
			CString dbm, offset;
			dbm.Format("%f", m1it->first);
			offset.Format("%f", m1it->second);
			
			xml.AddNode(CMarkup::MNT_ELEMENT, "power");
			
			xml.SetAttrib("dest_power", dbm);
			xml.SetAttrib("offset", offset);
		}
		xml.OutOfElem();
	}
	return xml.Save(filePath);
}

CInstrOffsetData CInstrOffsetData::operator + (CInstrOffsetData &data) {
	M2 m2 =  for_each(data.m_data.begin(), data.m_data.end(), M2Operator (this->m_data, Plus));
	return CInstrOffsetData(m2);
}

CInstrOffsetData CInstrOffsetData::operator - (CInstrOffsetData &data) {
	M2 m2 =  for_each(data.m_data.begin(), data.m_data.end(), M2Operator (this->m_data, Sub));
	return CInstrOffsetData(m2);
}