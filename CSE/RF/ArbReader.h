#ifndef ARB_READER_H
#define ARB_READER_H

#include <list>
#include <vector>

#define ARB ArbReader::Instance()

using namespace std;

class ArbReader
{
public:
	ArbReader();
	~ArbReader();
	static ArbReader &Instance();

	int Load(FILE *fp);
	int Load(const char *filePath);

	typedef struct FileHeader_t {
		char FileName[512];
		char Date[512];
		char Version[512];
		int  Segments;
		int  Checksum;
	} FileHeader_t;

	typedef struct SegHeader_t {
		int   SegID;
		int   Samples;
		int   Duration;
		float   LevelOffset;
		int   PeakOffset;
		char  Name[512];
		char  AuthInfo[32];
		int   MarkerBytes;
		float SampleRate;
	} SegHeader_t;

	typedef struct Marker_t {
		int No;
		int Type;
		union {
			struct {
				int StartPos;
				int Duration;
				int Period;
			} Period;
			struct {
				int Count;
				int StartPos[1024];
				int Duration[1024];
			} UnPeriod;
		};
	} Marker_t;

	typedef struct Data_t {
		unsigned short i;
		unsigned short q;
	} Data_t;
    typedef list<Marker_t> MarkerList;
	enum MarkerType { Period, UnPeriod };
    FileHeader_t      *m_FileHeader;
    SegHeader_t       *m_SegHeader;
	vector<MarkerList> m_Marker;
	vector<Data_t *>   m_Data;
	char *m_filebuf;
	int m_FileSize;

	int GetSegments() const                           { return m_FileHeader->Segments; }
	const FileHeader_t *GetFileHeader() const         { return m_FileHeader;          }
	const SegHeader_t  *GetSegInfo(int Segment) const { return &m_SegHeader[Segment]; }
	const MarkerList   &GetMarker(int Segment) const  { return m_Marker[Segment]; }
//	const Marker_t     &GetMarker(int Segment) const  { return m_Marker[Segment];}
	const Data_t       *GetData(int Segment) const    { return m_Data[Segment];       }
	bool operator == (const ArbReader &arbReader) const;
private:
	void Release();
	void Show(const FileHeader_t *p);
	void Show(const SegHeader_t *p);
	void Show(MarkerList &MarkerList);
	void Show(const Data_t *p);
};

struct ArbSeg_t
{
	uint32_t seg_num;
	uint32_t samples;
	uint32_t seg_adr;
	uint32_t inter_fltr;
	uint32_t sr;
	struct marker_t
	{
		int type;
		union
		{
			struct
			{
				int start;
				int keep;
				int period;
			} period;
			struct
			{
				int count;
				int start[1024];
				int keep[1024];
			} unperiod;
		} marker;
	} markers [4];
};

union as_reg_t
{
	struct {unsigned sample			:32;} adr0;
	struct {unsigned seg_adr		:32;} adr1;
	struct {unsigned inter_fltr		:32;} adr2;
	struct {unsigned mkr1_start		:32;} adr3;
	struct {unsigned mkr1_keep		:32;} adr4;
	struct {unsigned mkr1_period	:32;} adr5;
	struct {unsigned mkr2_start		:32;} adr6;
	struct {unsigned mkr2_keep		:32;} adr7;
	struct {unsigned mkr2_period	:32;} adr8;
	struct {unsigned mkr3_start		:32;} adr9;
	struct {unsigned mkr3_keep		:32;} adr10;
	struct {unsigned mkr3_period	:32;} adr11;
	struct {unsigned mkr4_start		:32;} adr12;
	struct {unsigned mkr4_keep		:32;} adr13;
	struct {unsigned mkr4_period	:32;} adr14;
	struct {unsigned sr				:32;} adr16;
	struct {
			unsigned mkr1_type      :1;
			unsigned mkr2_type		:1;
			unsigned mkr3_type		:1;
			unsigned mkr4_type		:1;
	}									  adr17;
	uint32_t data;
};

#define AS_REG(addr) as_reg[addr].adr##addr

#endif 


