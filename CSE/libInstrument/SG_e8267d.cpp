#include "stdafx.h"
#include "SG_e8267d.h"

SG_e8267d::SG_e8267d(void)
{
}

SG_e8267d::~SG_e8267d(void)
{
}

bool SG_e8267d::LoadOffset() {
	CString fileName = "./SG_E8267D_" + m_SN + ".xml";
	CIniFile::GetCurPath(fileName);
	m_instrOffset.Load(fileName);
	return true;
}

bool SG_e8267d::SaveOffset(CInstrOffsetData &data, bool isImport) {
	if (isImport) {
		m_instrOffset = data;
	}
	CString fileName = "./SG_E8267D_" + m_SN + ".xml";
	CIniFile::GetCurPath(fileName);
	return data.Save(fileName);
}