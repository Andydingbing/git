// CSEView.cpp : CCSEView ���ʵ��
//

#include "stdafx.h"
#include "CSE.h"

#include "CSEDoc.h"
#include "CSEView.h"

#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCSEView

IMPLEMENT_DYNCREATE(CCSEView, CView)

BEGIN_MESSAGE_MAP(CCSEView, CView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CCSEView ����/����

CCSEView::CCSEView()
{
	// TODO: �ڴ˴���ӹ������

}

CCSEView::~CCSEView()
{
}

BOOL CCSEView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ
	return CView::PreCreateWindow(cs);
}

// CCSEView ����

void CCSEView::OnDraw(CDC* /*pDC*/)
{
	CCSEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CCSEView ���

#ifdef _DEBUG
void CCSEView::AssertValid() const
{
	CView::AssertValid();
}

void CCSEView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCSEDoc* CCSEView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCSEDoc)));
	return (CCSEDoc*)m_pDocument;
}
#endif //_DEBUG


// CCSEView ��Ϣ�������

int CCSEView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	CSE_Model1_Maindlg.Create(IDD_CSE_MAIN,this);
	CSE_Model2_Maindlg.Create(IDD_CSE_MAIN,this);
	return 0;
}

void CCSEView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: �ڴ����ר�ô����/����û���
	CSE_Model1_Maindlg.ShowWindow(TRUE);												//��ʾ���Ի���
	CSE_Model2_Maindlg.ShowWindow(FALSE);

	CRect maindlg_cr;
	CSE_Model1_Maindlg.GetWindowRect(&maindlg_cr);										//������Ի����С
	CSE_Model1_Maindlg.MoveWindow(0,0,maindlg_cr.Width(),maindlg_cr.Height(),TRUE);		//�ƶ����Ի�����ͼ���Ͻ�
	CSE_Model2_Maindlg.MoveWindow(0,0,maindlg_cr.Width(),maindlg_cr.Height(),TRUE);
	maindlg_cr.bottom += 70;
	maindlg_cr.right += 10;
	AfxGetMainWnd()->MoveWindow(&maindlg_cr,true);										//�ı���ͼ�Ĵ�С����Ӧ���Ի���Ĵ�С
}
