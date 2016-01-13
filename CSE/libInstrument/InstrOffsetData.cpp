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
	if (m_data.find(freqHz) != m_data.end()) {                    // �ҵ�Ƶ��, ֱ�ӷ���
		return &m_data[freqHz];
	} else {                                              // û���ҵ�Ƶ��, �������Ƶ��
		double abs_diff = 1e10;                                 // ��¼Ƶ����С�������
		M1 *pPowerData = NULL;                 // ��¼��С���Ĺ��ʱ�
		M2::iterator itOffset;
		for (itOffset = m_data.begin(); itOffset != m_data.end(); itOffset++) { // ����
			if (fabs(freqHz - itOffset->first) < abs_diff) {      // �ҵ���С����Ƶ��
				abs_diff = fabs(freqHz - itOffset->first);              // ��¼��С���
				pPowerData   = &(itOffset->second);         // ��¼��С���Ƶ��Ĺ��ʱ�
			}
		}
		return pPowerData;                                  // ������С���Ƶ��Ĺ��ʱ�
	}
}

double CInstrOffsetData::findOffset(M1 *pPowerData, double dbm) {
	if (pPowerData == NULL) {
		return 0.0;
	}
	if (pPowerData->find(dbm) != pPowerData->end()) {                             // �ҵ����ʵ�
		return (*pPowerData)[dbm];                                                  // ֱ�ӷ���
	} else {                                                    // �Ҳ������ʵ�, ������Ĺ��ʵ�
		double abs_diff = 1000.0;                                     // ��¼���ʵ���С�������
		double offset   = 0;                                                    // ��¼��С���
		M1::iterator itPower;
		for (itPower = pPowerData->begin(); itPower != pPowerData->end(); itPower++) {  // ����
			if (fabs(dbm - itPower->first) < abs_diff) {                // �ҵ���С���Ĺ��ʵ�
				abs_diff = fabs(dbm - itPower->first);                          // ��¼��С���
				offset   = itPower->second;                       // ��¼��С���ʵ�Ĳ���ֵ
			}
		}
		return offset;                                            // ������С���ʵ�Ĳ���ֵ
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