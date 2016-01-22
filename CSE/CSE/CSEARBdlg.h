#pragma once
#include "afxwin.h"


// CCSEARBdlg �Ի���

class CCSEARBdlg : public CDialog
{
	DECLARE_DYNAMIC(CCSEARBdlg)

public:
	CCSEARBdlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCSEARBdlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_ARB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_CSE_ARB_FilePath;
	afx_msg void OnBnClickedSelectARBFilePath();
	CComboBox m_CSE_ARB_TriSrc;
	virtual BOOL OnInitDialog();
	int m_CSE_ARB_FreqOff;
	BOOL m_CSE_ARB_ReTrigger;
	int m_CSE_ARB_Cycles;
	int m_CSE_ARB_TrigDelay;
	BOOL m_CSE_ARB_AutoStart;
	int m_CSE_ARB_AddiSamp;
	CComboBox m_CSE_ARB_RepMode;
	CComboBox m_CSE_ARB_MultiSegTrigSrc;
	CComboBox m_CSE_ARB_MultiSegRepMode;
	CEdit m_CSE_ARB_File_Info;
};
