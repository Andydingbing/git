// CSEDoc.h : CCSEDoc ��Ľӿ�
//


#pragma once


class CCSEDoc : public CDocument
{
protected: // �������л�����
	CCSEDoc();
	DECLARE_DYNCREATE(CCSEDoc)

// ����
public:

// ����
public:

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// ʵ��
public:
	virtual ~CCSEDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};


