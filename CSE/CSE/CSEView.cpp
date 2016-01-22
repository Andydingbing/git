// CSEView.cpp : CCSEView 类的实现
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

int CCSEView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	CSE_Model1_Maindlg.Create(IDD_CSE_MAIN,this);
	CSE_Model2_Maindlg.Create(IDD_CSE_MAIN,this);
	return 0;
}

void CCSEView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	CSE_Model1_Maindlg.ShowWindow(TRUE);												//显示主对话框
	CSE_Model2_Maindlg.ShowWindow(FALSE);

	CRect maindlg_cr;
	CSE_Model1_Maindlg.GetWindowRect(&maindlg_cr);										//获得主对话框大小
	CSE_Model1_Maindlg.MoveWindow(0,0,maindlg_cr.Width(),maindlg_cr.Height(),TRUE);		//移动主对话框到视图左上角
	CSE_Model2_Maindlg.MoveWindow(0,0,maindlg_cr.Width(),maindlg_cr.Height(),TRUE);
	maindlg_cr.bottom += 70;
	maindlg_cr.right += 10;
	AfxGetMainWnd()->MoveWindow(&maindlg_cr,true);										//改变视图的大小以适应主对话框的大小
}
