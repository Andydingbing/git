#include "StdAfx.h"

#include "log.h"
#include "visa_driver.h"
#include <time.h>

static LARGE_INTEGER timestamp_freq;	//高精度计数器频率

#ifdef MFC
CNewListCtrl m_message_list;			//message_list全局变量
CNewListCtrl m_reg_list;				//reg_list全局变量

BEGIN_MESSAGE_MAP(CNewListCtrl,CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnNMCustomdraw)
END_MESSAGE_MAP()

void CNewListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = CDRF_DODEFAULT;
	NMLVCUSTOMDRAW *lplvdr = (NMLVCUSTOMDRAW *)pNMHDR;
	NMCUSTOMDRAW &nmcd = lplvdr->nmcd;
	switch(lplvdr->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			{
				*pResult = CDRF_NOTIFYITEMDRAW;
				break;
			}
		case CDDS_ITEMPREPAINT:
			{
				COLORREF ItemColor;
				unsigned long itemspec = nmcd.dwItemSpec;
				if (MapItemColor.Lookup(itemspec,ItemColor))
 				{
					lplvdr->clrText = ItemColor;
					*pResult = CDRF_DODEFAULT;
				}
			}
			break;
	}
}

void CNewListCtrl::SetItemColor(DWORD iItem,COLORREF color)
{
	MapItemColor.SetAt(iItem,color);
	this->RedrawItems(iItem,0);
 	this->SetFocus();
 	UpdateWindow();
}
#endif

int logger::message_list_count = 0;
int logger::reg_list_count = 0;

logger::logger()
{
	memset(newpropicture,42,sizeof(newpropicture) / sizeof(char) - 1);
	strcat(newpropicture,"\n");
}

logger &logger::Instance()
{
	static logger instance;
	return instance;
}

double logger::get_timestamp() 
{
	LARGE_INTEGER ll;
	double freq, t;
	QueryPerformanceFrequency(&ll);	
	freq = (double)ll.QuadPart;
	timestamp_freq = ll;
	QueryPerformanceCounter(&ll);
	static  double first_counter = (double)ll.QuadPart;
	t = (double)ll.QuadPart;
	return (t - first_counter) / freq;
}

void logger::stdprintf(const char *format, ...) 
{	
	char buf[512];
	static HANDLE console = NULL;
	double time = get_timestamp();
	sprintf(buf, "%6.3f s  ", time);

	va_list ap;
	va_start(ap, format);
	vsprintf(&buf[strlen(buf)], format, ap);
	va_end(ap);

	if ((log_flag & console_log_f)) {
		FILE *fp = fopen("c://console_log.txt", "a");
		if (time == 0.0)
			fprintf(fp,newpropicture);
		fprintf(fp,"%s",buf);
		fclose(fp);
	}
	if ((log_flag & console_log)) {
		if (console == NULL) {
			AllocConsole();
			console = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		WriteConsole(console, buf, strlen(buf), NULL, NULL);
	}
}

int logger::Write_Message_List(const char * action,...)
{
	char time_buf[512],action_buf[512];

	time_t t;
	tm *timeinfo;
	time(&t);
	timeinfo = localtime(&t);
	sprintf(time_buf,"%d-%d-%d-%d-%d-%d",(timeinfo->tm_year + 1900),(timeinfo->tm_mon + 1),timeinfo->tm_mday,
		timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

	va_list va_message;
	va_start(va_message,action);
	vsprintf(action_buf,action,va_message);

	va_end(va_message);

	if ((log_flag & message_log_f)) {
		FILE *fp = fopen("c://message_log.txt", "a");
		if (message_list_count == 0)
			fprintf(fp,newpropicture);
		fprintf(fp,"%s	%s\n",time_buf,action_buf);
		fclose(fp);
	}

#ifdef MFC
	if ((log_flag & message_log)) {
		int item_i;
		message_list_count ++;
		item_i = m_message_list.InsertItem(message_list_count,time_buf);
		m_message_list.SetItemText(item_i,1,action_buf);
		m_message_list.SetItemColor(item_i,RGB(0,0,255));
		m_message_list.EnsureVisible(item_i,FALSE);
	}
#endif

	return 0;
}

int logger::Write_Message_List(int result,const char * action,...)
{
	char time_buf[512],action_buf[512],result_buf[512];
	COLORREF col;
	time_t t;
	tm *timeinfo;
	time(&t);
	timeinfo = localtime(&t);
	sprintf(time_buf,"%d-%d-%d-%d-%d-%d",(timeinfo->tm_year + 1900),(timeinfo->tm_mon + 1),timeinfo->tm_mday,
							  timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

	va_list va_message;
	va_start(va_message,action);
	vsprintf(action_buf,action,va_message);

	if (result == 0) {
		sprintf(result_buf,"  √          0x%x",result);
		col = RGB(0,0,255);
	}
// 	else if (result > 0) {
// 		sprintf(result_buf,"  ╳          0x%x",result);
// 		col = RGB(255,0,0);
// 	}
	else {
		sprintf(result_buf,"  ╳          0x%x",result);
		col = RGB(255,0,0);
	}
	va_end(va_message);

	if ((log_flag & message_log_f)) {
		FILE *fp = fopen("c://message_log.txt", "a");
		if (message_list_count == 0)
			fprintf(fp,newpropicture);
		fprintf(fp,"%s	%40s%s\n",time_buf,action_buf,result_buf);
		fclose(fp);
	}

#ifdef MFC
	if ((log_flag & message_log)) {
		int item_i;
		message_list_count ++;
		item_i = m_message_list.InsertItem(message_list_count,time_buf);
		m_message_list.SetItemText(item_i,1,action_buf);
		m_message_list.SetItemText(item_i,2,result_buf);
		m_message_list.SetItemColor(item_i,col);
		m_message_list.EnsureVisible(item_i,FALSE);
	}
#endif

	return result;
}

int logger::Write_REG_List(int result,int regaddr,unsigned w_value,unsigned r_value)
{
	char time_buf[512],addr_buf[512],write_buf[512],read_buf[512],result_buf[512];
	COLORREF col;
	time_t t;
	tm *timeinfo;
	time(&t);
	timeinfo = localtime(&t);
	sprintf(time_buf,"%d-%d-%d-%d-%d-%d",(timeinfo->tm_year + 1900),(timeinfo->tm_mon + 1),timeinfo->tm_mday,
		timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

	if (result == 0) {
		strcpy(result_buf,"  √");
		col = RGB(0,0,0);
	}
	else {
		strcpy(result_buf,"  ╳");
		col = RGB(255,0,0);
	}
	int item_i;

	sprintf(addr_buf,"0x%4.4x",regaddr);
	if (w_value >= 0 && w_value < 0xffffffff) {
		sprintf(write_buf,"0x%8.8x",w_value);
		if ((log_flag & k7reg_log_f)) {
			FILE *fp = fopen("c://k7reg_log.txt", "a");
			if (reg_list_count == 0)
				fprintf(fp,newpropicture);
			fprintf(fp,"%s	%8s%16s%16s%s\n",time_buf,addr_buf,write_buf,"",result_buf);
			fclose(fp);
		}
	}
	if (r_value >= 0 && r_value < 0xffffffff) {
		sprintf(read_buf,"0x%8.8x",r_value);
		if ((log_flag & k7reg_log_f)) {
			FILE *fp = fopen("c://k7reg_log.txt", "a");
			if (reg_list_count == 0)
				fprintf(fp,newpropicture);
			fprintf(fp,"%s	%8s%16s%16s%s\n",time_buf,addr_buf,"",read_buf,result_buf);
			fclose(fp);
		}
	}

#ifdef MFC
	if ((log_flag & k7reg_log)) {
		reg_list_count ++;
		item_i = m_reg_list.InsertItem(reg_list_count,time_buf);
		m_reg_list.SetItemText(item_i,1,addr_buf);
		if (w_value >= 0 && w_value < 0xffffffff)
			m_reg_list.SetItemText(item_i,2,write_buf);
		if (r_value >= 0 && r_value < 0xffffffff)
			m_reg_list.SetItemText(item_i,3,read_buf);
		m_reg_list.SetItemText(item_i,4,result_buf);
		m_reg_list.SetItemColor(item_i,col);
		m_reg_list.EnsureVisible(item_i,FALSE);
	}
#endif

	return result;
}

int logger::Write_SPI_Log(int addr,int w_value,int r_value)
{
	if ((log_flag & spi_log_f)) {
		FILE *fp = fopen("c://spi_log.txt","a");
// 		if (message_list_count == 0 || reg_list_count == 0)
// 			fprintf(fp,newpropicture);
		if (w_value >= 0) {
			fprintf(fp,"0x%.3X	0x%.2X\n",addr,w_value);
			fclose(fp);
		}
		if (r_value >= 0) {
			fprintf(fp,"0x%.3X		  0x%.2x\n",addr,r_value);
			fclose(fp);
		}
	}
	return 0;
}

int logger::writetofile_float_1D(const char *filepath,float *data,int length)
{
	FILE *fp = fopen(filepath, "w");
	if (fp) {
		for (int i = 0; i < length; i++)
			fprintf(fp,"%f  \n",data[i]);
	}
	fclose(fp);
	return 0;
}

int logger::readfromfile_float_1D(const char *filepath,float *data,int length)
{
	FILE *fp = fopen(filepath, "r");
	if (fp) {
		for (int i= 0; i < length; i++) {
			fscanf(fp,"%f",&data[i]);
		}
	}
	fclose(fp);
	return 0;
}

int logger::writetofile_float_2D(const char *filepath, float *data, int size1D, int size2D) {
	FILE *fp = fopen(filepath, "w");
	if (fp) {
		for (int i = 0; i < size1D; i++) {
			for (int j = 0; j < size2D; j++) {
				fprintf(fp, "%10.6f ", data[i * size2D + j]);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}
	return 0;
}

int logger::readfromfile_float_2D(const char *filepath, float *data, int size1D, int size2D) {
	FILE *fp = fopen(filepath, "r");
	if (fp) {
		for (int i = 0; i < size1D; i++) {
			for (int j = 0; j < size2D; j++) {
				fscanf(fp, "%f ", &data[i * size2D + j]);
			}
		}
		fclose(fp);
	}
	return 0;
}

int logger::writetofile_double_2D(const char *filepath,double *data,int size1D,int size2D)
{
	FILE *fp = fopen(filepath, "w");
	if (fp) {
		for (int i = 0; i < size1D; i++) {
			for (int j = 0; j < size2D; j++) {
				fprintf(fp, "%f ", data[i * size2D + j]);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}
	return 0;
}