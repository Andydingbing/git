// CSEView.h : CCSEView 类的接口
//


#pragma once

#include "CSEMaindlg.h"

class CCSEView : public CView
{
protected: // 仅从序列化创建
	CCSEView();
	DECLARE_DYNCREATE(CCSEView)

// 属性
public:
	CCSEDoc* GetDocument() const;

// 操作
public:
	CCSEMaindlg CSE_Model1_Maindlg;
	CCSEMaindlg CSE_Model2_Maindlg;
// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 实现
public:
	virtual ~CCSEView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // CSEView.cpp 中的调试版本
inline CCSEDoc* CCSEView::GetDocument() const
   { return reinterpret_cast<CCSEDoc*>(m_pDocument); }
#endif

