// CSEDoc.cpp : CCSEDoc 类的实现
//

#include "stdafx.h"
#include "CSE.h"

#include "CSEDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCSEDoc

IMPLEMENT_DYNCREATE(CCSEDoc, CDocument)

BEGIN_MESSAGE_MAP(CCSEDoc, CDocument)
END_MESSAGE_MAP()


// CCSEDoc 构造/析构

CCSEDoc::CCSEDoc()
{
	// TODO: 在此添加一次性构造代码

}

CCSEDoc::~CCSEDoc()
{
}

BOOL CCSEDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	CDocument::SetTitle(_T("CSE"));
	return TRUE;
}




// CCSEDoc 序列化

void CCSEDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CCSEDoc 诊断

#ifdef _DEBUG
void CCSEDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCSEDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCSEDoc 命令
