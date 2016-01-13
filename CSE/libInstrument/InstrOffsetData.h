// InstrOffsetData.h: interface for the CInstrOffsetData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSTROFFSETDATA_H__73FD0CD1_893B_4F90_9E53_1E718140D219__INCLUDED_)
#define AFX_INSTROFFSETDATA_H__73FD0CD1_893B_4F90_9E53_1E718140D219__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "Markup.h"
#include "IniFile.h"

using namespace std;

class AFX_EXT_CLASS CInstrOffsetData  
{
public:
	CInstrOffsetData();
	virtual ~CInstrOffsetData();
private:
	typedef map<double, double> M1;
	typedef map<double, M1> M2;
public:
	bool	Load(LPCTSTR filePath, bool isCreate = false);
	bool	Save(LPCTSTR filePath);
	double	GetData(double freqHz, double dbm);
	bool	SetData(double freqHz, double dbm, double offset);
private:
	M1    *findFreqTable(double freqHz);
	double findOffset(M1 *pPowerData, double dbm);
public:
	enum Operator { Plus, Sub, };
	class M2Operator {
	private:
		class M1Operator {
		public:
			M1Operator(Operator oper, M1 &m1) : m_operator(oper), m_m1(m1) { }
			void operator () (const pair<double, double> &data) {
				switch (m_operator) {
				case Plus: m_m1[data.first] += data.second; break;
				case Sub:  m_m1[data.first] -= data.second; break;
				}
			}
			operator M1 () { return m_m1; }
		private:
			M1 m_m1;
			Operator m_operator;
		};
	public:
		M2Operator(M2 &data, Operator oper) : m_data(data), m_operator(oper) {}
		void operator () (const pair<double, M1> &m1) { for_each(m1.second.begin(), m1.second.end(), M1Operator (m_operator, m_data[m1.first])); }
		operator M2 () { return m_data; }
	private:
		M2 m_data;
		Operator m_operator;
	};
public:
	CInstrOffsetData operator + (CInstrOffsetData &data);
	CInstrOffsetData operator - (CInstrOffsetData &data);
private:
	CInstrOffsetData(M2 &data);
	M2 m_data;
};

#endif // !defined(AFX_INSTROFFSETDATA_H__73FD0CD1_893B_4F90_9E53_1E718140D219__INCLUDED_)
