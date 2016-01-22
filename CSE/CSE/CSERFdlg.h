#pragma once

#include "rf.h"

// CCSERFdlg 对话框

class CCSERFdlg : public CDialog
{
	DECLARE_DYNAMIC(CCSERFdlg)

public:
	CCSERFdlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCSERFdlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_RF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	rf RF_1;
	rf RF_2;
	rf RF_3;
	rf RF_4;
};
