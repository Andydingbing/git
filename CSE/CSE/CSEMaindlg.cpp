// CSEMaindlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CSE.h"
#include "CSEMaindlg.h"


// CCSEMaindlg �Ի���

IMPLEMENT_DYNAMIC(CCSEMaindlg, CDialog)

CCSEMaindlg::CCSEMaindlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCSEMaindlg::IDD, pParent)
{

}

CCSEMaindlg::~CCSEMaindlg()
{
}

void CCSEMaindlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CSE_FUNCTION_TREE, m_CSE_Function_Tree);
	DDX_Control(pDX, IDC_LIST1, m_CSE_Message_List);
	DDX_Control(pDX, IDC_LIST2, m_CSE_REG_List);
	DDX_Control(pDX, IDC_TAB1, m_CSE_CardSel_TabCtrl);
}


BEGIN_MESSAGE_MAP(CCSEMaindlg, CDialog)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CCSEMaindlg ��Ϣ�������

BOOL CCSEMaindlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	//����ģ�鹦��Ŀ¼��
	CFont CSE_Function_Tree_Font;				//��������
	LOGFONT logFont;
	logFont.lfHeight = 15;
	logFont.lfWidth = 15;
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

	hRoot  = m_CSE_Function_Tree.InsertItem(_T("����"),TVI_ROOT);
	hChild = m_CSE_Function_Tree.InsertItem(_T("RF"),hRoot);
	hChild = m_CSE_Function_Tree.InsertItem(_T("Baseband"),hRoot);
	m_CSE_Function_Tree.Expand(hRoot,TVE_EXPAND);

	hRoot  = m_CSE_Function_Tree.InsertItem(_T("У׼"));
	m_CSE_Function_Tree.Expand(hRoot,TVE_EXPAND);

	//Message List
	DWORD message_list_dwStyle;
	message_list_dwStyle = m_CSE_Message_List.GetStyle();
	message_list_dwStyle |= LVS_EX_GRIDLINES;
	m_CSE_Message_List.SetExtendedStyle(message_list_dwStyle);			//��ʾ�ָ���
	m_CSE_Message_List.InsertColumn(1,_T("Time"),0,80);
	m_CSE_Message_List.InsertColumn(2,_T("Action"),0,325);
	m_CSE_Message_List.InsertColumn(3,_T("Result      Return Code"),0,125);

	//REG List
	DWORD reg_list_dwStyle;
	reg_list_dwStyle = m_CSE_REG_List.GetStyle();
	reg_list_dwStyle |= LVS_EX_GRIDLINES;
	m_CSE_REG_List.SetExtendedStyle(reg_list_dwStyle);					//��ʾ�ָ���
	m_CSE_REG_List.InsertColumn(1,_T("Time"),0,80);
	m_CSE_REG_List.InsertColumn(2,_T("REG Address"),0,90);
	m_CSE_REG_List.InsertColumn(3,_T("Writed Value"),0,100);
	m_CSE_REG_List.InsertColumn(4,_T("Readed Value"),0,100);
	m_CSE_REG_List.InsertColumn(5,_T("Result"),0,50);

	//��Ƶ�忨ѡ��TabCtrl
	m_CSE_CardSel_TabCtrl.InsertItem(0,_T("RF_1"));
	m_CSE_CardSel_TabCtrl.InsertItem(1,_T("RF_2"));
	m_CSE_CardSel_TabCtrl.InsertItem(2,_T("RF_3"));
	m_CSE_CardSel_TabCtrl.InsertItem(3,_T("RF_4"));
	m_CSE_CardSel_TabCtrl.SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

int CCSEMaindlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������

	return 0;
}
