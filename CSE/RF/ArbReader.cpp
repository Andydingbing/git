#include "StdAfx.h"
#include "ArbReader.h"
#include "visa_driver.h"
#include "log.h"
#include <malloc.h>


ArbReader::ArbReader()
	: m_FileHeader(NULL)
	, m_SegHeader(NULL)
	, m_FileSize(-1)
{

}

ArbReader::~ArbReader()
{
	Release();
}

ArbReader &ArbReader::Instance()
{
	static ArbReader arbreader;
	return arbreader;
}

int ArbReader::Load(FILE *fp) {
	if (fp == NULL)
		return 1;

	fseek(fp, 0L, SEEK_END);
	m_FileSize = ftell(fp);
 	Log.stdprintf("m_FileSize = %d\n",m_FileSize);

	fseek(fp, 0, SEEK_SET);

	m_FileHeader = new FileHeader_t;
	fread(m_FileHeader, sizeof(FileHeader_t), 1, fp);
	Show(m_FileHeader);

	m_SegHeader = new SegHeader_t;
	fread(m_SegHeader, sizeof(SegHeader_t), 1, fp);

	Show(m_SegHeader);
	int markers = m_SegHeader->MarkerBytes / sizeof(Marker_t);

	MarkerList markerList;
	for (int j = 0; j < markers; j++) {
		Marker_t marker;
		fread(&marker, sizeof(Marker_t), 1, fp);
		markerList.push_back(marker);
	}
	Show(markerList);
	m_Marker.push_back(markerList);

	Data_t *data = new Data_t[m_SegHeader->Samples];
	fread(data, sizeof(Data_t), m_SegHeader->Samples, fp);
	
	m_Data.push_back(data);

	return 0;
}

int ArbReader::Load(const char *filePath) {
	int ret;
	FILE *fp = fopen(filePath, "rb");
	ret = Load(fp);
	fclose(fp);
	return ret;
}

void ArbReader::Release() {
	if (m_FileHeader) {
		free(m_FileHeader);
		m_FileHeader = NULL;
	}
	if (m_SegHeader) {
		free(m_SegHeader);
		m_SegHeader = NULL;
	}
	vector<Data_t *>::iterator iter;
    for (iter = m_Data.begin(); iter != m_Data.end(); iter++) {
    	if (*iter) {
    		delete *iter;
    		*iter = NULL;
    	}
    }
}

void ArbReader::Show(const FileHeader_t *p) {
 	Log.stdprintf("**************************** file header ****************************\n");
 	Log.stdprintf("FileName = %s\n", p->FileName);
 	Log.stdprintf("Date     = %s\n", p->Date);
 	Log.stdprintf("Version  = %s\n", p->Version);
 	Log.stdprintf("Segments = %d\n", p->Segments);
 	Log.stdprintf("Checksum = 0x%x\n", p->Checksum);
 	Log.stdprintf("*********************************************************************\n");
}

void ArbReader::Show(const SegHeader_t *p) {
 	Log.stdprintf("************************** Segment Header ***************************\n");
 	Log.stdprintf("Samples      = %d\n", p->Samples);
 	Log.stdprintf("Duration     = %d\n", p->Duration);
 	Log.stdprintf("LevelOffset  = %f\n", p->LevelOffset);
 	Log.stdprintf("PeakOffset   = %d\n", p->PeakOffset);
 	Log.stdprintf("AuthInfo     = %s\n", p->AuthInfo);
 	Log.stdprintf("Name         = %s\n", p->Name);
 	Log.stdprintf("MarkerBytes  = %d\n", p->MarkerBytes);
 	Log.stdprintf("SampleRate   = %f\n", p->SampleRate);
 	Log.stdprintf("*********************************************************************\n");
}

void ArbReader::Show(MarkerList &markerList) {
	Log.stdprintf("****************************** Marker *******************************\n");
	MarkerList::const_iterator iter;

	for (iter = markerList.begin(); iter != markerList.end(); iter++) {
		Log.stdprintf("No = %d\n", iter->No);
		Log.stdprintf("Type = %s\n", iter->Type == Period ? "period" : "unperiod");
		if (iter->Type == Period) {
			Log.stdprintf("StartPos = %d\n", iter->Period.StartPos);
			Log.stdprintf("Duration = %d\n", iter->Period.Duration);
			Log.stdprintf("Period   = %d\n", iter->Period.Period);
		} else if (iter->Type == UnPeriod) {
			for (int i = 0; i < iter->UnPeriod.Count; i++) {
				Log.stdprintf("StartPos = %d\n", iter->UnPeriod.StartPos[i]);
				Log.stdprintf("Duration = %d\n", iter->UnPeriod.Duration[i]);
			}
		} else {
			Log.stdprintf("Unknown Marker Type");
		}
	}
	Log.stdprintf("*********************************************************************\n");
}

void ArbReader::Show(const Data_t *p) {
 	Log.stdprintf("data.i = %d, data.q = %d\n", p->i, p->q);
}

bool ArbReader::operator == (const ArbReader &arbReader) const {
	return (arbReader.GetFileHeader()->Checksum == this->GetFileHeader()->Checksum);
}
