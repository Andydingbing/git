// SA_e4440a.cpp: implementation of the SA_e4440a class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SA_e4440a.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool SA_e4445a::SetEnablePreamp(bool isEnable) {
	if (isEnable) {
		if (!Print("POW:GAIN:STAT ON")) { return false; }
	} else {
		if (!Print("POW:GAIN:STAT OFF")) { return false; }
	}

	return true;
}