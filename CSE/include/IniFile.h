// IniFile.h: interface for the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__1747DDFA_5634_4870_90C5_BEB153EDAEEB__INCLUDED_)
#define AFX_INIFILE_H__1747DDFA_5634_4870_90C5_BEB153EDAEEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIniFile  
{
public:
	CIniFile();
	virtual ~CIniFile();
	bool		SetPath(CString strPath, bool IsCreatfile = FALSE, bool IsSearchSystem = FALSE);
	bool		SectionExist(CString strSection);
	CString		GetKeyValue(CString strSection, CString strKey, CString strDefault = _T(""));
	void		SetKeyValue(CString strSection, CString strKey,CString strKeyValue);
	void		DeleteKey(CString strSection, CString strKey);
	void		DeleteSection(CString strSection);
	int			GetAllSections(CStringArray &strArrSection);
	int			GetAllKeysAndValues(CString strSection, CStringArray &strArrKey, CStringArray &strArrKeyValue);
	void		DeleteAllSections();
	static void	GetCurPath(CString &strCurPath);
protected:
	CString		m_strPath;
};

#endif // !defined(AFX_INIFILE_H__1747DDFA_5634_4870_90C5_BEB153EDAEEB__INCLUDED_)
