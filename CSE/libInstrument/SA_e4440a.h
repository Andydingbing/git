// SA_e4440a.h: interface for the SA_e4440a class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SA_E4440A_H__2AD02497_A6EF_4749_BC47_64869CC2A406__INCLUDED_)
#define AFX_SA_E4440A_H__2AD02497_A6EF_4749_BC47_64869CC2A406__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SpectrumAnalyzer.h"
#include "SA_n9030a.h"

class SA_e4440a : public SA_n9030a
{
public:
	virtual LPCTSTR	GetInstrumentDescription() { return "E4440A"; }
};

class SA_e4445a : public SA_n9030a {
	public:
		virtual LPCTSTR GetInstrumentDescription() { return "E4445A"; }
		virtual bool SetEnablePreamp(bool isEnable);
};

class SA_e4443a : public SA_n9030a
{
public:
	virtual LPCTSTR	GetInstrumentDescription() { return "E4443A"; }
};


#endif // !defined(AFX_SA_E4440A_H__2AD02497_A6EF_4749_BC47_64869CC2A406__INCLUDED_)
