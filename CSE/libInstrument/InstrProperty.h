// InstrProperty.h: interface for the CInstrProperty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSTRPROPERTY_H__F70760AA_BA3F_423E_B962_8F0890B142C6__INCLUDED_)
#define AFX_INSTRPROPERTY_H__F70760AA_BA3F_423E_B962_8F0890B142C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IniFile.h"

#define InstrProperties CInstrProperty::Instance()

class CInstrProperty : public CIniFile  
{
public:
	virtual ~CInstrProperty();
	static CInstrProperty &Instance();
protected:
	CInstrProperty();
	CInstrProperty(CInstrProperty &);
public:
};

#endif // !defined(AFX_INSTRPROPERTY_H__F70760AA_BA3F_423E_B962_8F0890B142C6__INCLUDED_)
