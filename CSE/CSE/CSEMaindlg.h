#pragma once
#include "afxcmn.h"

#include "log.h"
#include "CSERFdlg.h"
#include "CSEARBdlg.h"
#include "CSEIQCapturedlg.h"

#define ShowDlg					\
if (ItemText == "RF") {			\
	ShowRFDlg(TRUE);			\
	ShowARBDlg(FALSE);			\
	ShowIQCapDlg(FALSE);		\
}								\
if (ItemText == "ARB") {		\
	ShowRFDlg(FALSE);			\
	ShowARBDlg(TRUE);			\
	ShowIQCapDlg(FALSE);		\
}								\
if (ItemText == "IQ Capture") {	\
	ShowRFDlg(FALSE);			\
	ShowARBDlg(FALSE);			\
	ShowIQCapDlg(TRUE);			\
}								\
if (ItemText == "Baseband") {   \
	ShowRFDlg(FALSE);			\
	ShowARBDlg(FALSE);			\
	ShowIQCapDlg(FALSE);		\
}

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
	CString ItemText;
public:						//����RFģ��
	rf RF_1,RF_2,RF_3,RF_4;
public:						//����RF�Ի���
	CCSERFdlg RF_1_dlg;
	CCSERFdlg RF_2_dlg;
	CCSERFdlg RF_3_dlg;
	CCSERFdlg RF_4_dlg;
	BOOL ShowRFDlg(BOOL show);
public:						//����ARB�Ի���
	CCSEARBdlg ARB_1_dlg;
	CCSEARBdlg ARB_2_dlg;
	CCSEARBdlg ARB_3_dlg;
	CCSEARBdlg ARB_4_dlg;
	BOOL ShowARBDlg(BOOL show);
public:						//����IQ Capture�Ի���
	CCSEIQCapturedlg IQCAP_1_dlg;
	CCSEIQCapturedlg IQCAP_2_dlg;
	CCSEIQCapturedlg IQCAP_3_dlg;
	CCSEIQCapturedlg IQCAP_4_dlg;
	BOOL ShowIQCapDlg(BOOL show);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	CTreeCtrl m_CSE_Function_Tree;	
	CNewListCtrl m_CSE_Message_List;
	CNewListCtrl m_CSE_REG_List;
	CTabCtrl m_CSE_CardSel_TabCtrl;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkCseFunctionTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickCseFunctionTree(NMHDR *pNMHDR, LRESULT *pResult);
};
