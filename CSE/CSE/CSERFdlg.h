#pragma once

#include "rf.h"

// CCSERFdlg �Ի���

class CCSERFdlg : public CDialog
{
	DECLARE_DYNAMIC(CCSERFdlg)

public:
	CCSERFdlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCSERFdlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_RF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	rf RF_1;
	rf RF_2;
	rf RF_3;
	rf RF_4;
};
