#pragma once


// CCSEIQCapturedlg �Ի���

class CCSEIQCapturedlg : public CDialog
{
	DECLARE_DYNAMIC(CCSEIQCapturedlg)

public:
	CCSEIQCapturedlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCSEIQCapturedlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_IQ_Capture };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
