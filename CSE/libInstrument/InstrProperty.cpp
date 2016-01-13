// InstrProperty.cpp: implementation of the CInstrProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InstrProperty.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInstrProperty::CInstrProperty()
{
	CIniFile::SetPath(".\\Instrument.ini");
}

CInstrProperty::~CInstrProperty()
{

}

CInstrProperty & CInstrProperty::Instance() {
	static CInstrProperty instrProperty;
	return instrProperty;
}
