// CSEDoc.cpp : CCSEDoc ���ʵ��
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


// CCSEDoc ����/����

CCSEDoc::CCSEDoc()
{
	// TODO: �ڴ����һ���Թ������

}

CCSEDoc::~CCSEDoc()
{
}

BOOL CCSEDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)
	CDocument::SetTitle(_T("CSE"));
	return TRUE;
}




// CCSEDoc ���л�

void CCSEDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}


// CCSEDoc ���

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


// CCSEDoc ����
