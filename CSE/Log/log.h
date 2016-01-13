#ifndef _LOG_H_
#define _LOG_H_

#define MFC

#define Log logger::Instance()

#define Result_Check(return_value)\
	if (return_value)			  \
		return;

#define Status_Check(func)								\
	for (int i = 0;i < 1;i ++) {						\
		int p = func;									\
		if (p) {										\
			Log.Write_Message_List(p,get_last_error());	\
			return p;									\
		}												\
	}

#define InstrumentStatusCheck(status)	\
	if (status != true)					\
	return -1;

#define no_log			0x0000
#define console_log		0x0001		//命令行log
#define console_log_f	0x0002		
#define message_log		0x0004		//message list log
#define message_log_f	0x0008
#define k7reg_log		0x0010		//k7 log
#define k7reg_log_f		0x0020
#define spi_log_f		0x0040		//spi log
#define all_log			0xffff

//log 开关
//const int log_flag = console_log|message_log|message_log_f|k7reg_log|k7reg_log_f;
const int log_flag = message_log|k7reg_log|console_log;
//const int log_flag = all_log;
//const int log_flag = console_log;

#ifdef MFC
class CNewListCtrl:public CListCtrl					//MFC ListCtrl
{
protected:
	DECLARE_MESSAGE_MAP()
public:
	void SetItemColor(DWORD iItem,COLORREF color);
public:
	afx_msg void OnNMCustomdraw(NMHDR*, LRESULT*);
	CMap<DWORD,DWORD&,COLORREF,COLORREF&>
		MapItemColor;
};
#endif

class logger
{
public:
	logger();
	static logger &Instance();
public:
	virtual void stdprintf(const char *format, ...);

	virtual int Write_Message_List(const char * action,...);
	virtual int Write_Message_List(int result,const char * action,...);
	virtual int Write_REG_List(int result,int regaddr,unsigned w_value = 0xffffffff,unsigned r_value = 0xffffffff);
	virtual int Write_SPI_Log(int addr,int w_value,int r_value);

	virtual int writetofile_float_1D(const char *filepath,float *data,int length);
	virtual int readfromfile_float_1D(const char *filepath,float *data,int length);
	virtual int writetofile_float_2D(const char *filepath, float *data, int size1D, int size2D);
	virtual int readfromfile_float_2D(const char *filepath, float *data, int size1D, int size2D);
	virtual int writetofile_double_2D(const char *filepath,double *data,int size1D,int size2D);
private:
	virtual double get_timestamp();
private:
	static int message_list_count;			//message_list计数
	static int reg_list_count;				//reg_list计数
	char newpropicture[100];
};

#endif