// CSEARBdlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CSE.h"
#include "CSEARBdlg.h"

#include "ArbReader.h"

// CCSEARBdlg 对话框

IMPLEMENT_DYNAMIC(CCSEARBdlg, CDialog)

CCSEARBdlg::CCSEARBdlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCSEARBdlg::IDD, pParent)
	, m_CSE_ARB_FreqOff(0)
	, m_CSE_ARB_ReTrigger(FALSE)
	, m_CSE_ARB_Cycles(1)
	, m_CSE_ARB_TrigDelay(0)
	, m_CSE_ARB_AutoStart(TRUE)
	, m_CSE_ARB_AddiSamp(0)
{

}

CCSEARBdlg::~CCSEARBdlg()
{
}

void CCSEARBdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT5, m_CSE_ARB_FilePath);
	DDX_Control(pDX, IDC_COMBO1, m_CSE_ARB_TriSrc);
	DDX_Text(pDX, IDC_EDIT1, m_CSE_ARB_FreqOff);
	DDX_Check(pDX, IDC_CHECK1, m_CSE_ARB_ReTrigger);
	DDX_Text(pDX, IDC_EDIT2, m_CSE_ARB_Cycles);
	DDX_Text(pDX, IDC_EDIT3, m_CSE_ARB_TrigDelay);
	DDX_Check(pDX, IDC_CHECK2, m_CSE_ARB_AutoStart);
	DDX_Text(pDX, IDC_EDIT4, m_CSE_ARB_AddiSamp);
	DDX_Control(pDX, IDC_COMBO2, m_CSE_ARB_RepMode);
	DDX_Control(pDX, IDC_COMBO3, m_CSE_ARB_MultiSegTrigSrc);
	DDX_Control(pDX, IDC_COMBO4, m_CSE_ARB_MultiSegRepMode);
	DDX_Control(pDX, IDC_EDIT6, m_CSE_ARB_File_Info);
}


BEGIN_MESSAGE_MAP(CCSEARBdlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON3, &CCSEARBdlg::OnBnClickedSelectARBFilePath)
END_MESSAGE_MAP()


// CCSEARBdlg 消息处理程序

void CCSEARBdlg::OnBnClickedSelectARBFilePath()
{
	// TODO: 在此添加控件通知处理程序代码
	ArbReader arb_temp;
	CString buf = "";
	CString file_info = "";
	CFileDialog ArbFile(TRUE);
	if (ArbFile.DoModal() == IDOK) {
		m_CSE_ARB_FilePath.SetWindowText(ArbFile.GetPathName());
	}
	else
		return;
	FILE *fp = fopen(ArbFile.GetPathName(), "rb");
	if (fp == NULL)
		return;

	fseek(fp, 0L, SEEK_END);
	arb_temp.m_FileSize = ftell(fp);
	buf.Format("FileSize = %d",arb_temp.m_FileSize);
	file_info += (buf + _T("\r\n\n"));

	fseek(fp, 0, SEEK_SET);

	ArbReader::FileHeader_t temp_FileHeader;
	fread(&temp_FileHeader, sizeof(ArbReader::FileHeader_t), 1, fp);
	buf.Format("********** File Header **********");
	file_info += (buf + _T("\r\n"));
	buf.Format("FileName = %s", temp_FileHeader.FileName);
	file_info += (buf + _T("\r\n"));
	buf.Format("Date     = %s", temp_FileHeader.Date);
	file_info += (buf + _T("\r\n"));
	buf.Format("Version  = %s", temp_FileHeader.Version);
	file_info += (buf + _T("\r\n"));
	buf.Format("Segments = %d", temp_FileHeader.Segments);
	file_info += (buf + _T("\r\n"));
	buf.Format("Checksum = %#x",temp_FileHeader.Checksum);
	file_info += (buf + _T("\r\n"));
	buf.Format("*********************************");
	file_info += (buf + _T("\r\n\n"));

	ArbReader::SegHeader_t tempSegHeader;
	fread(&tempSegHeader, sizeof(ArbReader::SegHeader_t), 1, fp);
	buf.Format("********* Segment Header *********");
	file_info += (buf + _T("\r\n"));
	buf.Format("Samples      = %d", tempSegHeader.Samples);
	file_info += (buf + _T("\r\n"));
	buf.Format("Duration     = %d", tempSegHeader.Duration);
	file_info += (buf + _T("\r\n"));
	buf.Format("LevelOffset  = %f", tempSegHeader.LevelOffset);
	file_info += (buf + _T("\r\n"));
	buf.Format("PeakOffset   = %d", tempSegHeader.PeakOffset);
	file_info += (buf + _T("\r\n"));
	buf.Format("AuthInfo     = %s", tempSegHeader.AuthInfo);
	file_info += (buf + _T("\r\n"));
	buf.Format("Name         = %s", tempSegHeader.Name);
	file_info += (buf + _T("\r\n"));
	buf.Format("MarkerBytes  = %d", tempSegHeader.MarkerBytes);
	file_info += (buf + _T("\r\n"));
	buf.Format("SampleRate   = %f", tempSegHeader.SampleRate);
	file_info += (buf + _T("\r\n"));
	buf.Format("**********************************");
	file_info += (buf + _T("\r\n\n"));
	int markers = tempSegHeader.MarkerBytes / sizeof(ArbReader::Marker_t);

	ArbReader::MarkerList tempMarkerList;
	for (int j = 0; j < markers; j++) {
		ArbReader::Marker_t marker;
		fread(&marker, sizeof(ArbReader::Marker_t), 1, fp);
		tempMarkerList.push_back(marker);
	}
	buf.Format("************ Marker ************");
	file_info += (buf + _T("\r\n"));
	ArbReader::MarkerList::const_iterator iter;

	for (iter = tempMarkerList.begin(); iter != tempMarkerList.end(); iter++) {
		buf.Format("No = %d", iter->No);
		file_info += (buf + _T("\r\n"));
		buf.Format("Type = %s", iter->Type == ArbReader::Period ? "period" : "unperiod");
		file_info += (buf + _T("\r\n"));
		if (iter->Type == ArbReader::Period) {
			buf.Format("StartPos = %d", iter->Period.StartPos);
			file_info += (buf + _T("\r\n"));
			buf.Format("Duration = %d", iter->Period.Duration);
			file_info += (buf + _T("\r\n"));
			buf.Format("Period   = %d", iter->Period.Period);
			file_info += (buf + _T("\r\n"));
		} else if (iter->Type == ArbReader::UnPeriod) {
			for (int i = 0; i < iter->UnPeriod.Count; i++) {
				buf.Format("StartPos = %d", iter->UnPeriod.StartPos[i]);
				file_info += (buf + _T("\r\n"));
				buf.Format("Duration = %d", iter->UnPeriod.Duration[i]);
				file_info += (buf + _T("\r\n"));
			}
		} else {
			buf.Format("Unknown Marker Type");
			file_info += (buf + _T("\r\n"));
		}
	}
	buf.Format("********************************");
	file_info += (buf + _T("\r\n\n"));

// 	Data_t *data = new Data_t[m_SegHeader->Samples];
// 	fread(data, sizeof(Data_t), m_SegHeader->Samples, fp);
// 
// 	m_Data.push_back(data);

	m_CSE_ARB_File_Info.SetWindowText(file_info);
	UpdateData(FALSE);

	fclose(fp);
	return;
}

BOOL CCSEARBdlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//ARB默认配置
	m_CSE_ARB_TriSrc.SetCurSel(0);
	m_CSE_ARB_RepMode.SetCurSel(1);
	m_CSE_ARB_MultiSegTrigSrc.SetCurSel(0);
	m_CSE_ARB_MultiSegRepMode.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
