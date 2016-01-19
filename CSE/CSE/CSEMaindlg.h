#pragma once
#include "afxcmn.h"


// CCSEMaindlg �Ի���

class CCSEMaindlg : public CDialog
{
	DECLARE_DYNAMIC(CCSEMaindlg)

public:
	CCSEMaindlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCSEMaindlg();

// �Ի�������
	enum { IDD = IDD_CSE_MAIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	CTreeCtrl m_CSE_Function_Tree;	
	CListCtrl m_CSE_Message_List;
	CListCtrl m_CSE_REG_List;

	CTabCtrl m_CSE_CardSel_TabCtrl;
};
