// CSEMaindlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CSE.h"
#include "CSEMaindlg.h"

#ifdef MFC
extern CNewListCtrl m_CSE_Message_List;
extern CNewListCtrl m_CSE_REG_List;
#else
CListCtrl m_message_list;
CListCtrl m_reg_list;
#endif

// CCSEMaindlg 对话框

IMPLEMENT_DYNAMIC(CCSEMaindlg, CDialog)

CCSEMaindlg::CCSEMaindlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCSEMaindlg::IDD, pParent),RF_1(1),RF_2(2),RF_3(3),RF_4(4)
{
	ItemText = "RF";			//启动时默认显示RF调试的对话框
	RF_1_dlg.RF_1 = RF_1;
	RF_2_dlg.RF_2 = RF_2;
	RF_3_dlg.RF_3 = RF_3;
	RF_4_dlg.RF_4 = RF_4;
}

CCSEMaindlg::~CCSEMaindlg()
{
}

void CCSEMaindlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CSE_FUNCTION_TREE, m_CSE_Function_Tree);
	DDX_Control(pDX, IDC_MESSAGE_LIST, m_CSE_Message_List);
	DDX_Control(pDX, IDC_REG_LIST, m_CSE_REG_List);
	DDX_Control(pDX, IDC_TAB1, m_CSE_CardSel_TabCtrl);
}


BEGIN_MESSAGE_MAP(CCSEMaindlg, CDialog)
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CCSEMaindlg::OnTcnSelchangeTab1)
	ON_NOTIFY(NM_DBLCLK, IDC_CSE_FUNCTION_TREE, &CCSEMaindlg::OnNMDblclkCseFunctionTree)
	ON_NOTIFY(NM_CLICK, IDC_CSE_FUNCTION_TREE, &CCSEMaindlg::OnNMClickCseFunctionTree)
END_MESSAGE_MAP()


// CCSEMaindlg 消息处理程序

BOOL CCSEMaindlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//CSE单个模块功能目录树
	CFont CSE_Function_Tree_Font;				//设置字体
	LOGFONT logFont;
	logFont.lfHeight = 15;
	logFont.lfWidth = 10;
	logFont.lfEscapement = 10;
	logFont.lfOrientation = 10;
	logFont.lfWeight = FW_NORMAL;
	logFont.lfItalic = 0;
	logFont.lfUnderline = 0;
	logFont.lfStrikeOut = 0;
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logFont.lfQuality = PROOF_QUALITY;
	logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
	CSE_Function_Tree_Font.CreateFontIndirect(&logFont);
	m_CSE_Function_Tree.SetFont(&CSE_Function_Tree_Font);

	HTREEITEM hRoot;
	HTREEITEM hChild;

	hRoot  = m_CSE_Function_Tree.InsertItem(_T("调试"),TVI_ROOT);
	hChild = m_CSE_Function_Tree.InsertItem(_T("RF"),hRoot);
	m_CSE_Function_Tree.SelectItem(hChild);								//默认选择RF
	hChild = m_CSE_Function_Tree.InsertItem(_T("ARB"),hRoot);
	hChild = m_CSE_Function_Tree.InsertItem(_T("IQ Capture"),hRoot);
	hChild = m_CSE_Function_Tree.InsertItem(_T("Baseband"),hRoot);
	m_CSE_Function_Tree.Expand(hRoot,TVE_EXPAND);

	hRoot  = m_CSE_Function_Tree.InsertItem(_T("校准"));
	m_CSE_Function_Tree.Expand(hRoot,TVE_EXPAND);

	//Message List
	DWORD message_list_dwStyle;
	message_list_dwStyle = m_CSE_Message_List.GetStyle();
	message_list_dwStyle |= LVS_EX_GRIDLINES;
	m_CSE_Message_List.SetExtendedStyle(message_list_dwStyle);			//显示分割线
	m_CSE_Message_List.InsertColumn(1,_T("Time"),0,80);
	m_CSE_Message_List.InsertColumn(2,_T("Action"),0,325);
	m_CSE_Message_List.InsertColumn(3,_T("Result      Return Code"),0,125);

	//REG List
	DWORD reg_list_dwStyle;
	reg_list_dwStyle = m_CSE_REG_List.GetStyle();
	reg_list_dwStyle |= LVS_EX_GRIDLINES;
	m_CSE_REG_List.SetExtendedStyle(reg_list_dwStyle);					//显示分割线
	m_CSE_REG_List.InsertColumn(1,_T("Time"),0,80);
	m_CSE_REG_List.InsertColumn(2,_T("REG Address"),0,90);
	m_CSE_REG_List.InsertColumn(3,_T("Writed Value"),0,100);
	m_CSE_REG_List.InsertColumn(4,_T("Readed Value"),0,100);
	m_CSE_REG_List.InsertColumn(5,_T("Result"),0,50);

	//射频板卡选择TabCtrl
	m_CSE_CardSel_TabCtrl.InsertItem(0,_T("RF_1"));
	m_CSE_CardSel_TabCtrl.InsertItem(1,_T("RF_2"));
	m_CSE_CardSel_TabCtrl.InsertItem(2,_T("RF_3"));
	m_CSE_CardSel_TabCtrl.InsertItem(3,_T("RF_4"));
	m_CSE_CardSel_TabCtrl.SetCurSel(0);

	//射频板卡选项卡中的RF对话框
	CRect cse_cardsel_tabctrl_cr;
	m_CSE_CardSel_TabCtrl.GetClientRect(&cse_cardsel_tabctrl_cr);
	cse_cardsel_tabctrl_cr.top    += 30;
	cse_cardsel_tabctrl_cr.bottom -= 8;
	cse_cardsel_tabctrl_cr.left   += 8;
	cse_cardsel_tabctrl_cr.right  -= 8;
	RF_1_dlg.Create(IDD_DIALOG_RF,&m_CSE_CardSel_TabCtrl);
	RF_1_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	RF_1_dlg.ShowWindow(TRUE);
	RF_2_dlg.Create(IDD_DIALOG_RF,&m_CSE_CardSel_TabCtrl);
	RF_2_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	RF_2_dlg.ShowWindow(FALSE);
	RF_3_dlg.Create(IDD_DIALOG_RF,&m_CSE_CardSel_TabCtrl);
	RF_3_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	RF_3_dlg.ShowWindow(FALSE);
	RF_4_dlg.Create(IDD_DIALOG_RF,&m_CSE_CardSel_TabCtrl);
	RF_4_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	RF_4_dlg.ShowWindow(FALSE);
	
	//射频板卡选项卡中的ARB对话框
	ARB_1_dlg.Create(IDD_DIALOG_ARB,&m_CSE_CardSel_TabCtrl);
	ARB_1_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	ARB_1_dlg.ShowWindow(FALSE);
	ARB_2_dlg.Create(IDD_DIALOG_ARB,&m_CSE_CardSel_TabCtrl);
	ARB_2_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	ARB_2_dlg.ShowWindow(FALSE);
	ARB_3_dlg.Create(IDD_DIALOG_ARB,&m_CSE_CardSel_TabCtrl);
	ARB_3_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	ARB_3_dlg.ShowWindow(FALSE);
	ARB_4_dlg.Create(IDD_DIALOG_ARB,&m_CSE_CardSel_TabCtrl);
	ARB_4_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	ARB_4_dlg.ShowWindow(FALSE);

	//射频板卡选项卡中的IQCapture对话框
	IQCAP_1_dlg.Create(IDD_DIALOG_IQ_Capture,&m_CSE_CardSel_TabCtrl);
	IQCAP_1_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	IQCAP_1_dlg.ShowWindow(FALSE);
	IQCAP_2_dlg.Create(IDD_DIALOG_IQ_Capture,&m_CSE_CardSel_TabCtrl);
	IQCAP_2_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	IQCAP_2_dlg.ShowWindow(FALSE);
	IQCAP_3_dlg.Create(IDD_DIALOG_IQ_Capture,&m_CSE_CardSel_TabCtrl);
	IQCAP_3_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	IQCAP_3_dlg.ShowWindow(FALSE);
	IQCAP_4_dlg.Create(IDD_DIALOG_IQ_Capture,&m_CSE_CardSel_TabCtrl);
	IQCAP_4_dlg.MoveWindow(&cse_cardsel_tabctrl_cr);
	IQCAP_4_dlg.ShowWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

int CCSEMaindlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}

BOOL CCSEMaindlg::ShowRFDlg(BOOL show)
{
	int RF_Curseltab = m_CSE_CardSel_TabCtrl.GetCurSel() + 1;
	if (show) {
		return (RF_1_dlg.ShowWindow((RF_Curseltab == 1)?TRUE:FALSE) |
				RF_2_dlg.ShowWindow((RF_Curseltab == 2)?TRUE:FALSE) |
				RF_3_dlg.ShowWindow((RF_Curseltab == 3)?TRUE:FALSE) |
				RF_4_dlg.ShowWindow((RF_Curseltab == 4)?TRUE:FALSE));
	}
	else {
		return (RF_1_dlg.ShowWindow(FALSE) |
				RF_2_dlg.ShowWindow(FALSE) |
				RF_3_dlg.ShowWindow(FALSE) |
				RF_4_dlg.ShowWindow(FALSE));
	}
}

BOOL CCSEMaindlg::ShowARBDlg(BOOL show)
{
	int ARB_Curseltab = m_CSE_CardSel_TabCtrl.GetCurSel() + 1;
	if (show) {
		return (ARB_1_dlg.ShowWindow((ARB_Curseltab == 1)?TRUE:FALSE) |
				ARB_2_dlg.ShowWindow((ARB_Curseltab == 2)?TRUE:FALSE) |
				ARB_3_dlg.ShowWindow((ARB_Curseltab == 3)?TRUE:FALSE) |
				ARB_4_dlg.ShowWindow((ARB_Curseltab == 4)?TRUE:FALSE));
	}
	else {
		return (ARB_1_dlg.ShowWindow(FALSE) |
				ARB_2_dlg.ShowWindow(FALSE) |
				ARB_3_dlg.ShowWindow(FALSE) |
				ARB_4_dlg.ShowWindow(FALSE));
	}
}

BOOL CCSEMaindlg::ShowIQCapDlg(BOOL show)
{
	int IQCap_Curseltab = m_CSE_CardSel_TabCtrl.GetCurSel() + 1;
	if (show) {
		return (IQCAP_1_dlg.ShowWindow((IQCap_Curseltab == 1)?TRUE:FALSE) |
				IQCAP_2_dlg.ShowWindow((IQCap_Curseltab == 2)?TRUE:FALSE) |
				IQCAP_3_dlg.ShowWindow((IQCap_Curseltab == 3)?TRUE:FALSE) |
				IQCAP_4_dlg.ShowWindow((IQCap_Curseltab == 4)?TRUE:FALSE));
	}
	else {
		return (IQCAP_1_dlg.ShowWindow(FALSE) |
				IQCAP_2_dlg.ShowWindow(FALSE) |
				IQCAP_3_dlg.ShowWindow(FALSE) |
				IQCAP_4_dlg.ShowWindow(FALSE));
	}
}

void CCSEMaindlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	ShowDlg;
	*pResult = 0;
}

void CCSEMaindlg::OnNMDblclkCseFunctionTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	UINT uFlags = 0;
	CPoint cse_function_tree_pt = GetCurrentMessage()->pt;
	m_CSE_Function_Tree.ScreenToClient(&cse_function_tree_pt);
	HTREEITEM hItem = m_CSE_Function_Tree.HitTest(cse_function_tree_pt,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
		m_CSE_Function_Tree.SelectItem(hItem);
	HTREEITEM hCurItem = m_CSE_Function_Tree.GetSelectedItem();
	ItemText = m_CSE_Function_Tree.GetItemText(hCurItem);
	ShowDlg;
	Log.Write_Message_List("5789");
	*pResult = 0;
}

void CCSEMaindlg::OnNMClickCseFunctionTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	OnNMDblclkCseFunctionTree(pNMHDR,pResult);
	*pResult = 0;
}
