// CSEView.h : CCSEView ��Ľӿ�
//


#pragma once


class CCSEView : public CView
{
protected: // �������л�����
	CCSEView();
	DECLARE_DYNCREATE(CCSEView)

// ����
public:
	CCSEDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// ʵ��
public:
	virtual ~CCSEView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // CSEView.cpp �еĵ��԰汾
inline CCSEDoc* CCSEView::GetDocument() const
   { return reinterpret_cast<CCSEDoc*>(m_pDocument); }
#endif

