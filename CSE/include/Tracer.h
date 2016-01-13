#ifndef _TRACER_H_
#define _TRACER_H_

#include <winsock.h>
#include <fstream>
#include "stdint.h"

using namespace std;

class CTracer {
public:
	enum TracerType { FileTracer = 1, SocketTracer = 2, ConsoleTracer = 4 };
	CTracer(uint8_t tracerFlags) : m_tracerFlags(tracerFlags), m_hFile(NULL), m_hConsole(NULL) { };
	~CTracer() {
		Close();
	}
	virtual bool Init() {
		bool fileRet = true, socketRet = true, consoleRet = true;
		if (m_tracerFlags & FileTracer)    fileRet    = initFile();
		if (m_tracerFlags & SocketTracer)  socketRet  = initSocket();
		if (m_tracerFlags & ConsoleTracer) consoleRet = initConsole();
		return (fileRet & socketRet & consoleRet);
	}
	virtual bool Write(LPCTSTR format, ...) {
		char buffer[1024];
		ZeroMemory(buffer, 1024);
		va_list ap;
		va_start(ap, format);
		vsprintf(buffer, format, ap);
		va_end(ap);

		bool fileRet = true, socketRet = true, consoleRet = true;
		if (m_tracerFlags & FileTracer)    fileRet    = writeFile(buffer);
		if (m_tracerFlags & SocketTracer)  socketRet  = writeSocket(buffer);
		if (m_tracerFlags & ConsoleTracer) consoleRet = writeConsole(buffer);
		return (fileRet & socketRet & consoleRet);
	}
	virtual bool SetFilePath(LPCTSTR filePath) {
		if (filePath == NULL) 
			return false;
		m_filePath = filePath;
		CIniFile::GetCurPath(m_filePath);
		return true;
	}
	virtual bool SetSocketParam(LPCTSTR rmtIpAddr, int port) { return false; }
	virtual void Close() {
		if (m_tracerFlags & FileTracer && m_hFile != NULL)       closeFile();
		if (m_tracerFlags & SocketTracer && m_hSocket != NULL)   closeSocket();
		if (m_tracerFlags & ConsoleTracer && m_hConsole != NULL) closeConsole();
	}
protected:
	virtual bool initFile() {
		if (m_hFile != NULL) {
			closeFile();
		}
		m_hFile = fopen(m_filePath, "w");
		uint16_t pn, sn;
		g_pSP5064->GetModuleInfo(pn, sn);
		Write("SP5064R%d_%d\n", pn, sn);
		return true;
	}
	virtual bool writeFile(LPCTSTR str) {
		fwrite(str, sizeof(char), strlen(str), m_hFile);
		fflush(m_hFile);
		return true;
	}
	virtual bool closeFile() {
		fclose(m_hFile);
		m_hFile = NULL;
		return true; 
	}
	
	virtual bool initSocket()              { return false; }
	virtual bool writeSocket(LPCTSTR str)  { return false; }
	virtual bool closeSocket()             { return false; }
	
	virtual bool initConsole()             { AllocConsole(); m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE); return true; }
	virtual bool writeConsole(LPCTSTR str) { WriteConsole(m_hConsole, str, strlen(str), NULL, NULL); return true; }
	virtual bool closeConsole()            { if (!FreeConsole()) { return false; } return true; }
protected:
	uint8_t     m_tracerFlags;
	FILE       *m_hFile;
	HANDLE      m_hConsole;
	int         m_hSocket;
	CString     m_filePath;
	sockaddr_in m_rmtSocketParam;
};

#endif