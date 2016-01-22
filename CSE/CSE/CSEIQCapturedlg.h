#pragma once


// CCSEIQCapturedlg 对话框

class CCSEIQCapturedlg : public CDialog
{
	DECLARE_DYNAMIC(CCSEIQCapturedlg)

public:
	CCSEIQCapturedlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCSEIQCapturedlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_IQ_Capture };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
