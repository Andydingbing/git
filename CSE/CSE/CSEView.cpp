// CSEView.cpp : CCSEView 类的实现
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

// CCSEView 构造/析构

CCSEView::CCSEView()
{
	// TODO: 在此处添加构造代码

}

CCSEView::~CCSEView()
{
}

BOOL CCSEView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CCSEView 绘制

void CCSEView::OnDraw(CDC* /*pDC*/)
{
	CCSEDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CCSEView 诊断

#ifdef _DEBUG
void CCSEView::AssertValid() const
{
	CView::AssertValid();
}

void CCSEView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCSEDoc* CCSEView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCSEDoc)));
	return (CCSEDoc*)m_pDocument;
}
#endif //_DEBUG


// CCSEView 消息处理程序
