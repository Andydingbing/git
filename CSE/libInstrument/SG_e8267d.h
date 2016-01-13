#pragma once
#include "SG_e4438c.h"
#include "SG_n5182b.h"

class SG_e8267d :
	public SG_n5182b
{
public:
	SG_e8267d(void);
	~SG_e8267d(void);

	virtual LPCTSTR GetInstrumentDescription() { return "E8267D"; }
	bool LoadOffset();
	bool SaveOffset(CInstrOffsetData &data, bool isImport);
};
