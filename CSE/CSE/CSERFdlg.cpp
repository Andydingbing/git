// CSERFdlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CSE.h"
#include "CSERFdlg.h"


// CCSERFdlg 对话框

IMPLEMENT_DYNAMIC(CCSERFdlg, CDialog)

CCSERFdlg::CCSERFdlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCSERFdlg::IDD, pParent),RF_1(1),RF_2(2),RF_3(3),RF_4(4)
{
	
}

CCSERFdlg::~CCSERFdlg()
{
}

void CCSERFdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCSERFdlg, CDialog)
END_MESSAGE_MAP()


// CCSERFdlg 消息处理程序
