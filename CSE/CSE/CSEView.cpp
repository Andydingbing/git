// CSEView.cpp : CCSEView ���ʵ��
//

#include "stdafx.h"
#include "CSE.h"

#include "CSEDoc.h"
#include "CSEView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCSEView

IMPLEMENT_DYNCREATE(CCSEView, CView)

BEGIN_MESSAGE_MAP(CCSEView, CView)
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
