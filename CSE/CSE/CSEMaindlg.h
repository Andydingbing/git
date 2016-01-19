#pragma once
#include "afxcmn.h"


// CCSEMaindlg 对话框

class CCSEMaindlg : public CDialog
{
	DECLARE_DYNAMIC(CCSEMaindlg)

public:
	CCSEMaindlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCSEMaindlg();

// 对话框数据
	enum { IDD = IDD_CSE_MAIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	CTreeCtrl m_CSE_Function_Tree;	
	CListCtrl m_CSE_Message_List;
	CListCtrl m_CSE_REG_List;

	CTabCtrl m_CSE_CardSel_TabCtrl;
};
