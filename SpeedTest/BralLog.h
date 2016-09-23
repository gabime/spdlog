#pragma once


#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
//#include "BrlDataPath.h"
#include "VStringAp.h"

// #define FILE_STAY_OPEN

const DWORD			LOG_BUF_SIZE	=	1024;
const DWORD			TIME_BUF_SIZE	=	20;

const DWORD		CF_ACCESS_FLAGS	=	GENERIC_WRITE;
const DWORD		CF_SHARE_MODE	=	FILE_SHARE_WRITE;
const DWORD		CF_CR_DISP		=	OPEN_ALWAYS;


typedef enum E_BRLLOG_FLAG
{
	// Globale Flags, die modulübergreifend ein- und ausgeschaltet 
	// werden und in jedem Modul reserviert sind.
	E_BRLLOG_GLOB_INIT					=	0x0001,
	E_BRLLOG_GLOB_PNP					=	0x0002,
	E_BRLLOG_GLOB_STATUS				=	0x0004,		// Status-Änderungen
	E_BRLLOG_GLOB_DATAFLOW				=	0x0008,		// Daten-Übertragung Cl -> Srv -> Drv ...
	E_BRLLOG_GLOB_ERROR					=	0x0010,
	E_BRLLOG_GLOB_ASSIGN				=	0x0020,		// Geräte-Zuweisungen etc
	E_BRLLOG_GLOB_IPC					=	0x0040,		// Interprozess-Kommunikation
	E_BRLLOG_BRL_PROCESS				=	0x0080,		// Bearbeitung der Brailledaten (Screen und Window)
	E_BRLLOG_BRL_PROCESS_DUMPDATA		=	0x0100,		// Dump der Daten bei der Bearbeitung (HexDump)
	E_BRLLOG_BDC_DEVMGMT				=	0x0200,		// DeviceManagment  der BralDriverConfig.dll
	E_BRLLOG_BTR_LIBLOUIS				=	0x0400,		// Logging von LibLouis einschalten (Kurzschriftübersetzer)
	E_BRLLOG_BRSV_TIMERS				=	0x0800,
	E_BRLLOG_BRALDRV_IPC				=	0x01000,	// Interprozesskommunikation BrailleDriver.exe

}E_BRLLOG_FLAG;

// Standard für Braille-Treiber
const DWORD			STD_DRIVER_LOGFLAG	=	0x8000FFFF;
const DWORD			STD_LOGFLAG			=	E_BRLLOG_GLOB_INIT|E_BRLLOG_GLOB_PNP|E_BRLLOG_GLOB_STATUS|E_BRLLOG_GLOB_ERROR|E_BRLLOG_GLOB_ASSIGN|E_BRLLOG_BRSV_TIMERS;

class CLoggingPath
{
public:
	std::wstring	Drive;
	std::wstring	Dir;
	std::wstring	FileName;
	std::wstring	FileExt;

	BOOL Build(std::wstring& wsRes)
	{
		WSTRING_AP Path(_MAX_PATH);
		_wmakepath(Path,Drive.c_str(),Dir.c_str(),FileName.c_str(),FileExt.c_str());
		wsRes = Path;
		return TRUE;
	}

};

class CBralLog
{
private:
	DWORD			m_IniFlag;
	DWORD			m_Flag;
	BOOL			m_OutputDebugString;
	DWORD			m_ProcessId;

//	CRITICAL_SECTION		m_cs;


	std::wstring	m_FileName;
	#ifdef BRLLOG_VFL
	LONG			m_hLog;
	#endif

#ifdef FILE_STAY_OPEN
	HANDLE			m_hFile;
#endif

//	wchar_t			m_LogBuf[LOG_BUF_SIZE+1];
		

	void InitPrivateData(void)
	{
		#ifdef BRLLOG_VFL
		m_hLog = -1;
		#endif
		m_Flag = 0; m_IniFlag = 0;
//		memset(m_LogBuf,0,sizeof(m_LogBuf));
		m_OutputDebugString=FALSE;
		m_ProcessId = GetCurrentProcessId();
	}

	wchar_t HexCharW(BYTE value)
	{
		value &= 0xF;
		if (value >= 0 && value <= 9) return (CHAR)(L'0' + value);
		else return (wchar_t)(L'A' + (value - 10));
	}
	
	#ifndef BRLLOG_VFL
	HANDLE MyCreateFile(DWORD dwCreationDisposition = CF_CR_DISP)
	{
#ifdef FILE_STAY_OPEN
		return m_hFile;
#else
		return CreateFileW(m_FileName.c_str(),CF_ACCESS_FLAGS,CF_SHARE_MODE,NULL,dwCreationDisposition,0,NULL);
#endif
	}
	#endif


public:
	
	const std::wstring& GetFileName(void){return m_FileName;}

private:
	void _Log(const wchar_t* str);
	void _LogS2(const wchar_t* s1,const wchar_t* s2,const wchar_t* str);
	void _vLogPrintf(const wchar_t* format,va_list parameter);
	void _vLogPrintfS2(const wchar_t* s1,const wchar_t* s2,const wchar_t* format,va_list parameter);
	void _LogHex8(const wchar_t* header,const void* data,DWORD DataSize);
	void _LogHex16(const wchar_t* header,const void* data,DWORD DataSize);
	void _LogPrintf(const wchar_t* format,...)
	{
		va_list   Next;
		va_start(Next, format);
		_vLogPrintf(format,Next);
	}

	void _LogRaw(const wchar_t* str,DWORD dwLen);

public:
	CBralLog(void);
	~CBralLog(void);
	//#ifdef BRAL_NO_GLOBAL_HELPERS
	//void Init(HINSTANCE hModule,CBrlDataPath& BrlDataPath);
	//#else
	//void Init(HINSTANCE hModule);
	//#endif
	void Init(const std::wstring& IniFileName,CLoggingPath& LogFileName,const std::wstring& ModuleName);
	void Free(void);

	BOOL GetIniFileNameBd(std::wstring& Result,HINSTANCE hDll);
	BOOL GetIniFileNameBs(std::wstring& Result,HINSTANCE hDll);
//	BOOL GetLoggingPathBs(CLoggingPath& Result,CBrlDataPath& BrlDataPath,HINSTANCE hDll);
	BOOL GetModuleNameBs(std::wstring& Result,HINSTANCE hInst);

	void LogControl(LONG LogEnable,LONG LogClear);


	const wchar_t* GetLogFileName(void)
	{
		return m_FileName.c_str();
	}


	void SetFlag(DWORD dwFl);
	DWORD GetFlag(void){return m_Flag;}

	void LogHex8(DWORD dwFlag,const wchar_t* header,const void* data,DWORD DataSize)
	{
		if (dwFlag&m_Flag)
		{
			_LogHex8(header, data, DataSize);
		}
	}

	void LogHex16(DWORD dwFlag,const wchar_t* header,const void* data,DWORD DataSize)
	{
		if (dwFlag&m_Flag)
		{
			_LogHex16(header, data, DataSize);
		}
	}

	// Bei diesen Funktionen haben wir evtl. eine Chance, dass der Compiler sie 
	// inline einbaut und daher schnell ausführt
	void LogRaw(DWORD dwFlag,const wchar_t* str,DWORD dwLen)
	{
		if (dwFlag&m_Flag)
		{
			_LogRaw(str, dwLen);
		}
	}
	void Log(DWORD dwFlag,const wchar_t* str)
	{
		if (dwFlag&m_Flag)
		{
			_Log(str);
		}
	}

	void LogPrintf(DWORD dwFlag,const wchar_t* format,...)
	{

		if(dwFlag&m_Flag)
		{
			va_list   Next;
			va_start(Next, format);
			_vLogPrintf(format,Next);
		}
	}

	void vLogPrintf(DWORD dwFlag,const wchar_t* format,va_list parameter)
	{
		if(dwFlag&m_Flag)
			_vLogPrintf(format,parameter);
	}
	
};

#ifndef BRAL_NO_GLOBAL_HELPERS
extern CBralLog BralLog;
#endif