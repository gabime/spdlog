#include "BralLog.h"
#include <shlobj.h>
#include <time.h>
#include <SYS\timeb.h>
#include "StrT.h"
//#include "BralSrvVersion.h"
//#include "BrlDataPath.h"
#ifdef BRLLOG_VFL
#include "..\vfl\vfl.h"
#endif



CBralLog::CBralLog(void)
{
	InitPrivateData();
} 

CBralLog::~CBralLog(void)
{
	InitPrivateData();
}

const wchar_t NEW_LINE[] = L"\r\n";
const wchar_t UNICODE_TXT_PREFIX = 0xFEFF;

void CBralLog::SetFlag(DWORD dwFl)
{
	if((m_Flag==0)&&(dwFl!=0))
	{

		#ifndef BRLLOG_VFL
		// Kopf schreiben!
		DWORD nSize = _MAX_DIR;
		SYSTEMTIME st;
		memset(&st,0,sizeof(st));
		GetLocalTime(&st);
		StrW ws;
		DWORD dwProcId = GetCurrentProcessId();
		DWORD dwSessionId = 0;
		ProcessIdToSessionId(dwProcId,&dwSessionId);
		ws.AddElem(UNICODE_TXT_PREFIX);
		ws << L"\r\n";
		ws << L"================================================================================\r\n";
		ws << L"--- BAUM LogInitialize ---\r\n";
		ws << VSTR_MANIP_DEC << L"Date " << st.wDay << L"." << st.wMonth << L"." << st.wYear << L" Time " << st.wHour << L":" << st.wMinute << L":" << st.wMilliseconds << NEW_LINE;
		ws << NEW_LINE;
		ws << L" ProcessId: " << dwProcId << L" SessionId: " << dwSessionId << NEW_LINE;
		ws << L"================================================================================\r\n";

		_LogRaw(ws.c_ptr(),ws.length());
		#endif
	}
	m_Flag = dwFl;
}

void CBralLog::Free(void)
{
	m_FileName.resize(0);
	#ifdef FILE_STAY_OPEN
	CloseHandle(m_hFile);
	#endif

//	DeleteCriticalSection(&m_cs);
}




// Der BrailleDriver hat entweder die "BralMiniServer.ini" aus dem gleichen 
// Verzeichnis am Wickel, oder die "Baum.ini" auch aus dem gleichen Verz.
// (unter COBRA bzw dem JAWS-Treiber)
BOOL CBralLog::GetIniFileNameBd(std::wstring& Result,HINSTANCE hDll)
{
	WSTRING_AP IniName(_MAX_PATH);
	WSTRING_AP Path(_MAX_PATH);
	WSTRING_AP Drive(_MAX_DRIVE);
	WSTRING_AP Dir(_MAX_DIR);
	WSTRING_AP Fname(_MAX_FNAME);

	Result = L"";

	GetModuleFileNameW(hDll,Path,_MAX_PATH);
	_wsplitpath(Path,Drive,Dir,Fname,NULL);
	
	_wmakepath(IniName,Drive,Dir,L"BralMiniServer",L"ini");

	// Gibt es die Datei? Wenn nein, versuche sie eine Ebene höher zu finden.
	if(GetFileAttributesW(IniName)!=INVALID_FILE_ATTRIBUTES)
	{
		// BralMiniServer.ini im gleichen Verzeichnis gefunden: nimm diese
		Result = IniName;
		return TRUE;
	}

	_wmakepath(IniName,Drive,Dir,L"baum",L"ini");
	if(GetFileAttributesW(IniName)!=INVALID_FILE_ATTRIBUTES)
	{
		// BralMiniServer.ini im gleichen Verzeichnis gefunden: nimm diese
		Result = IniName;
		return TRUE;
	}

	return FALSE;
}


// Die für das Log zuständige INI ist in der Regel im gleichen Verzeichnis
// wie die DLL selber und heisst gewöhnlich "BralMiniServer.ini". 
// uU befindet sich die DLL in einem Unterverzeichnis (z.B. "CIF20").
// Dann sollte das Logging die "BralMiniServer.ini" aus dem übergeordneten
// Verzeichnis nutzen, falls es keine INI im gleichen Verzeichnis gibt.
BOOL CBralLog::GetIniFileNameBs(std::wstring& Result,HINSTANCE hDll)
{
	WSTRING_AP IniName(_MAX_PATH);
	WSTRING_AP Path(_MAX_PATH);
	WSTRING_AP Drive(_MAX_DRIVE);
	WSTRING_AP Dir(_MAX_DIR);
	WSTRING_AP Fname(_MAX_FNAME);

	Result = L"";

	GetModuleFileNameW(hDll,Path,_MAX_PATH);
	_wsplitpath(Path,Drive,Dir,Fname,NULL);
	
	_wmakepath(IniName,Drive,Dir,L"BralMiniServer",L"ini");

	// Gibt es die Datei? Wenn nein, versuche sie eine Ebene höher zu finden.
	if(GetFileAttributesW(IniName)!=INVALID_FILE_ATTRIBUTES)
	{
		// BralMiniServer.ini im gleichen Verzeichnis gefunden: nimm diese
		Result = IniName;
		return TRUE;
	}
	else
	{
		DWORD dwOffs = wcslen(Dir)-1;
		// Abschließenden Backslash wegputzen, falls es ihn gibt (muss es geben)
		if(Dir.s[dwOffs]==L'\\')
		{
			Dir.s[dwOffs] = L'\0';
			dwOffs--;
		}

		for(;(dwOffs>0)&&(Dir.s[dwOffs]!=L'\\');dwOffs--)
			Dir.s[dwOffs] = L'\0';

		// EXE da?
		_wmakepath(IniName,Drive,Dir,L"BralMiniServer",L"exe");
		if(GetFileAttributesW(IniName)!=INVALID_FILE_ATTRIBUTES)
		{
			// INI da?
			_wmakepath(IniName,Drive,Dir,L"BralMiniServer",L"ini");
			if(GetFileAttributesW(IniName)!=INVALID_FILE_ATTRIBUTES)
			{
				// BralMiniServer.exe und ini im übergeordneten Verzeichnis: 
				// Fall, wo sich die DLL in einem dem BS untergeordneten 
				// Interface-Verzeichnis befindet.
				Result = IniName;
				return TRUE;
			}
		}
	}


	return FALSE;
}
//BOOL CBralLog::GetLoggingPathBs(CLoggingPath& Result,CBrlDataPath& BrlDataPath,HINSTANCE hDll)
//{
//	WSTRING_AP Path(_MAX_PATH);
//	WSTRING_AP Drive(_MAX_DRIVE);
//	WSTRING_AP Dir(_MAX_DIR);
//	WSTRING_AP Fname(_MAX_FNAME);
//
//	GetModuleFileNameW(hDll,Path,_MAX_PATH);
//	_wsplitpath(Path,NULL,NULL,Fname,NULL);
//
//	std::wstring pa = BrlDataPath.DataPath;
//	_wsplitpath(pa.c_str(),Drive,Dir,NULL,NULL);
////	_wmakepath(Path,Drive,Dir,Fname,L"Log");
//
//	Result.Drive = Drive;
//	Result.Dir = Dir;
//	Result.FileName = Fname;
//	Result.FileExt = L"Log";
//
////	Result = Path;
//
//	return TRUE;
//}

BOOL CBralLog::GetModuleNameBs(std::wstring& Result,HINSTANCE hInst)
{
	WSTRING_AP IniName(_MAX_PATH);
	WSTRING_AP Path(_MAX_PATH);
	WSTRING_AP Fname(_MAX_FNAME);

	Result = L"";

	GetModuleFileNameW(hInst,Path,_MAX_PATH);
	_wsplitpath(Path,NULL,NULL,Fname,NULL);
	Result = Fname;
	return TRUE;
}


//#ifdef BRAL_NO_GLOBAL_HELPERS
//void CBralLog::Init(HINSTANCE hModule,CBrlDataPath& BrlDataPath)
//#else
//void CBralLog::Init(HINSTANCE hModule)
//#endif
void CBralLog::Init(const std::wstring& IniFileName,CLoggingPath& LogFileName,const std::wstring& ModuleName)
{
	//WSTRING_AP IniName(_MAX_PATH);
	WSTRING_AP Path(_MAX_PATH);
	WSTRING_AP Drive(_MAX_DRIVE);
	WSTRING_AP Dir(_MAX_DIR);
	WSTRING_AP Fname(_MAX_FNAME);
	//WSTRING_AP LogFileName(_MAX_FNAME);
	WSTRING_AP LogPathSegment(_MAX_DIR);
	LONG lt = 0;
	const DWORD INVALID_FLAG_VALUE	=	0xFFFFFFFF;

	//std::wstring	ModuleName;
//	InitializeCriticalSection(&m_cs);

	//// INI befindet sich im gleichen Verzeichnis wie die aufrufende EXE-Datei
	//GetModuleFileNameW(NULL,Path,_MAX_PATH);
	//_wsplitpath(Path,Drive,Dir,Fname,NULL);


	//// INI-File "Baum.ini" hat Vorrang, ansonsten "<ModuName>.ini" oder "BralMiniServer.ini"
	//_wmakepath(IniName,Drive,Dir,L"baum",L"ini");
	//if(GetFileAttributesW(IniName)==INVALID_FILE_ATTRIBUTES)
	//	_wmakepath(IniName,Drive,Dir,Fname,L"ini");
	//if(GetFileAttributesW(IniName)==INVALID_FILE_ATTRIBUTES)
	//	_wmakepath(IniName,Drive,Dir,L"BralMiniServer",L"ini");

	//BOOL bValidPath = FALSE;

	//// Fname und ModuleName leiten sich nicht von der aufrufenden 'EXE her,
	//// sondern vom übergebenen Modul-Handle
	//GetModuleFileNameW(hModule,Path,_MAX_PATH);
	//_wsplitpath(Path,Drive,Dir,Fname,NULL);
	//ModuleName = Fname;


	//// Log-Pfad entspricht dem BrailleDataPfad.
	////SHGetFolderPathW(NULL,CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,Path);
	////// Muss einen Backslash drauf setzen, sonst denkt splitpath, es wäre ein Dateiname...
	////wcsncat(Path,L"\\",_MAX_PATH-wcslen(Path));	
	//std::wstring pa = BrlDataPath.DataPath;
	//_wsplitpath(pa.c_str(),Drive,Dir,NULL,NULL);

	// Einstellungen betreffs des Log-Filenamen und Subpath aus INI lesen.
	// Wenn es den Schlüssel "LogFileName" gibt, überschreibt dieser die Dateinamen-Komponente, 
	// die aus dem Modulnamen ermittelt wurde.
	GetPrivateProfileStringW(L"Logging",L"LogFileName",LogFileName.FileName.c_str(),Fname,_MAX_FNAME,IniFileName.c_str());
	LogFileName.FileName = Fname;

	// Wenn es den Schlüssel "LogSubPath" gibt, so wird der aus BrlDataPath ermittelte 
	// Pfad verworfen, der neue Pfad ist COMMON_APPDATA ergänzt um den INI-Pfad
	if(GetPrivateProfileStringW(L"Logging",L"LogSubPath",L"",LogPathSegment,_MAX_DIR,IniFileName.c_str())>0)
	{
		SHGetFolderPathW(NULL,CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,Path);
		// Backslash als Trenner erforderlich
		if(LogPathSegment.s[0]!=L'\\')
			wcsncat(Path,L"\\",_MAX_PATH-wcslen(Path));
		wcsncat(Path,LogPathSegment,_MAX_PATH-wcslen(Path));
		// Muss auf Backslash enden!
		if(Path.s[wcslen(Path.s)-1]!=L'\\')
			wcsncat(Path,L"\\",_MAX_PATH-wcslen(Path));
		_wsplitpath(Path,Drive,Dir,NULL,NULL);
		LogFileName.Dir = Dir;
		LogFileName.Drive = Drive;
	}


//	wcsncat(Dir,BRLDATA_ROOTPATH,_MAX_DIR-wcslen(Dir));
	//_wmakepath(Path,Drive,Dir,Fname,L"Log");
	//m_FileName = Path;
	LogFileName.Build(m_FileName);


	// Log-Flag aus INI lesen
	m_IniFlag = GetPrivateProfileIntW(L"Logging",L"Global",STD_LOGFLAG,IniFileName.c_str());
	lt = GetPrivateProfileIntW(L"Logging",Fname,INVALID_FLAG_VALUE,IniFileName.c_str());
	if(lt==0)
		m_IniFlag = 0;
	else if(lt!=INVALID_FLAG_VALUE)
		m_IniFlag |= lt;

	// Den General-Schalter überprüfen
	LONG iLog = GetPrivateProfileIntW(L"Logging",L"Logging",0,IniFileName.c_str());
	if(iLog!=0)
		m_Flag = m_IniFlag;


	// Nachschauen, ob die Ausgabe auch per OutputDebugString erfolgen soll.
	m_OutputDebugString = GetPrivateProfileIntW(L"Logging",L"OutputDebugString",m_OutputDebugString,IniFileName.c_str());
	if(m_OutputDebugString==FALSE)
	{
		// Okay, eigentlich nicht laut INI. Wenn aber jemand lauscht, machen wir ihm die Freude.
		OutputDebugStringA("TT\r\n");
		if(GetLastError()==0)
			m_OutputDebugString = TRUE;
	}


	// Kein Logging gewünscht -> keinen Kopf schreiben!
	if(m_Flag==0)
		return;

	#ifdef	BRLLOG_VFL
	VflCreateLoggingService((BSTR)m_FileName.c_str(),0xFFFFFFFF,(LONG)hModule,&m_hLog);
	if(m_OutputDebugString)
		VflActivateDebugStringOut(m_hLog,TRUE);
	#else

#ifdef FILE_STAY_OPEN
	m_hFile = CreateFileW(m_FileName.c_str(), CF_ACCESS_FLAGS, CF_SHARE_MODE, NULL, OPEN_ALWAYS, 0, NULL);
#endif

	// Kopf schreiben
	DWORD nSize = _MAX_DIR;
	GetUserNameW(Dir,&nSize);
	SYSTEMTIME st;
	memset(&st,0,sizeof(st));
	GetLocalTime(&st);
	StrW ws;
	DWORD dwProcId = GetCurrentProcessId();
	DWORD dwSessionId = 0;
	ProcessIdToSessionId(dwProcId,&dwSessionId);
	ws.AddElem(UNICODE_TXT_PREFIX);
	ws << L"\r\n";
	ws << L"================================================================================\r\n";
	ws << L"--- BAUM LogInitialize Module " << ModuleName.c_str() << L"---\r\n";
	ws << VSTR_MANIP_DEC << L"Date " << st.wDay << L"." << st.wMonth << L"." << st.wYear << L" Time " << st.wHour << L":" << st.wMinute << L":" << st.wMilliseconds << NEW_LINE;
	ws << NEW_LINE;
	ws << L"User: " << Dir << L" ProcessId: " << dwProcId << L" SessionId: " << dwSessionId << NEW_LINE;
	ws << L"================================================================================\r\n";

	_LogRaw(ws.c_ptr(),ws.length());

	#endif

//	#define MYLOGPRINTF(fl,s,...) LogPrintf(fl,s,__VA_ARGS__ )
//	MYLOGPRINTF(0xFF,L"Dies ist ein oller Mist!");

}


void CBralLog::LogControl(LONG LogEnable,LONG LogClear)
{
#ifndef FILE_STAY_OPEN

	if(LogClear>0)
	{
		#ifdef BRLLOG_VFL
		VflClearLog(m_hLog,NULL);
		#else
		// Log löschen
//		HANDLE hFile = CreateFileW(m_FileName.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
		HANDLE hFile = MyCreateFile(CREATE_ALWAYS);
		DWORD nwr = 0;
		WriteFile(hFile,&UNICODE_TXT_PREFIX,sizeof(UNICODE_TXT_PREFIX),&nwr,NULL);
		CloseHandle(hFile);
		#endif
	}

	if(LogEnable==0)
	{
		// Log ausschalten
		if(m_Flag!=0)
		{
			m_Flag = 0;

			#ifdef BRLLOG_VFL
			VflEnableLogging(m_hLog,FALSE,NULL);
			#else
			SYSTEMTIME st;
			memset(&st,0,sizeof(st));
			GetLocalTime(&st);
			StrW ws;
			ws.AddElem(UNICODE_TXT_PREFIX);
			ws << L"\r\n";
			ws << L"================================================================================\r\n";
			ws << L"--- Shutdown Logging ---\r\n";
			ws << VSTR_MANIP_DEC << L"Date " << st.wDay << L"." << st.wMonth << L"." << st.wYear << L" Time " << st.wHour << L":" << st.wMinute << L":" << st.wMilliseconds << NEW_LINE;
			ws << L"================================================================================\r\n";
			_LogRaw(ws.c_ptr(),ws.length());
			#endif
		}

	}
	else if(LogEnable>0)
	{
		if((m_Flag==0)&&(m_IniFlag!=0))
		{
			m_Flag = m_IniFlag;

			#ifdef BRLLOG_VFL
			VflEnableLogging(m_hLog,TRUE,NULL);
			#else

			SYSTEMTIME st;
			memset(&st,0,sizeof(st));
			GetLocalTime(&st);
			StrW ws;
			ws.AddElem(UNICODE_TXT_PREFIX);
			ws << L"\r\n";
			ws << L"================================================================================\r\n";
			ws << L"--- Activate Logging ---\r\n";
			ws << VSTR_MANIP_DEC << L"Date " << st.wDay << L"." << st.wMonth << L"." << st.wYear << L" Time " << st.wHour << L":" << st.wMinute << L":" << st.wMilliseconds << NEW_LINE;
			ws << L"================================================================================\r\n";
			_LogRaw(ws.c_ptr(),ws.length());
			#endif
		}
	}
#endif
}






void CBralLog::_LogHex8(const wchar_t* header,const void* data,DWORD len)
{

	if(data==NULL)
		return;

	WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);

	#ifndef BRLLOG_VFL
//	HANDLE hFile = CreateFileW(m_FileName.c_str(),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
	HANDLE hFile = MyCreateFile();
	if(hFile==INVALID_HANDLE_VALUE)
		return;

	SetFilePointer(hFile,0,NULL,FILE_END);
	#endif

	std::wstring	result;
	static const DWORD CHARS_SIZE = 78;


	// Header darstellen
	if(header==NULL)
		header = L"HEXDUMP";

	SYSTEMTIME st;
	DWORD lt;
	GetLocalTime(&st);
	LONG nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"%02u:%02u:%02u:%03u %s%s",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,header,NEW_LINE);
	#ifdef BRLLOG_VFL
	result.append(m_LogBuf);
//	VflLogCStringRaw(m_hLog,0x01,m_LogBuf);
	#else
	WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
	#endif

	wchar_t chars[CHARS_SIZE];
	wmemset(chars,L' ',CHARS_SIZE);
	// Dekorationen erzeugen
	chars[76]=L'\r';
	chars[77] = L'\n';
	
	const BYTE* bytes = (const BYTE*)data;

	for (DWORD i1 = 0; i1 < len; i1 += 16)
	{
		chars[0] = HexCharW(i1 >> 28);
		chars[1] = HexCharW(i1 >> 24);
		chars[2] = HexCharW(i1 >> 20);
		chars[3] = HexCharW(i1 >> 16);
		chars[4] = HexCharW(i1 >> 12);
		chars[5] = HexCharW(i1 >> 8);
		chars[6] = HexCharW(i1 >> 4);
		chars[7] = HexCharW(i1 >> 0);

		DWORD offset1 = 11;
		DWORD offset2 = 60;
			
		for (DWORD i2 = 0; i2 < 16; i2++)
		{
			if (i1 + i2 >= len)
			{
				chars[offset1] = L' ';
				chars[offset1 + 1] = L' ';
				chars[offset2] = L' ';
			}
			else
			{
				byte b = bytes[i1 + i2];
				chars[offset1] = HexCharW(b >> 4);
				chars[offset1 + 1] = HexCharW(b);
				chars[offset2] = (b < 32 ? L'·' : (wchar_t)b);
			}
			offset1 += (i2 == 8 ? 4 : 3);
			offset2++;
		}

		result.append(chars,CHARS_SIZE);
//		result.append(L"\r\n");
	}

//	result.append(L"\r\n");
	#ifdef	BRLLOG_VFL
	VflLogCStringRaw(m_hLog,0x01,result.c_str());
	#else
	DWORD nWritten = 0;
	WriteFile(hFile,result.data(),result.size()*sizeof(wchar_t),&nWritten,NULL);
#ifndef FILE_STAY_OPEN
	CloseHandle(hFile);
#endif
	#endif

}

void CBralLog::_LogHex16(const wchar_t* header,const void* data,DWORD RealLen)
{

	if(data==NULL)
		return;

	WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);

	#ifndef BRLLOG_VFL
//	HANDLE hFile = CreateFileW(m_FileName.c_str(),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
	HANDLE hFile = MyCreateFile();
	if(hFile==INVALID_HANDLE_VALUE)
		return;

	SetFilePointer(hFile,0,NULL,FILE_END);
	#endif

	std::wstring	result;
	static const DWORD CHARS_SIZE = 78;


	// Header darstellen
	if(header==NULL)
		header = L"HEXDUMP";

	SYSTEMTIME st;
	DWORD lt;
	GetLocalTime(&st);
	LONG nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"%02u:%02u:%02u:%03u %s%s",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,header,NEW_LINE);
	#ifdef BRLLOG_VFL
	result.append(m_LogBuf);
	#else
	WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
	#endif

	wchar_t chars[CHARS_SIZE];
	wmemset(chars,L' ',CHARS_SIZE);
	// Dekorationen erzeugen
	chars[76]=L'\r';
	chars[77] = L'\n';
	
	const WORD* bytes = (const WORD*)data;
	DWORD len = RealLen/sizeof(WORD);

	for (DWORD i1 = 0; i1 < len; i1 += 8)
	{
		chars[0] = HexCharW(i1 >> 28);
		chars[1] = HexCharW(i1 >> 24);
		chars[2] = HexCharW(i1 >> 20);
		chars[3] = HexCharW(i1 >> 16);
		chars[4] = HexCharW(i1 >> 12);
		chars[5] = HexCharW(i1 >> 8);
		chars[6] = HexCharW(i1 >> 4);
		chars[7] = HexCharW(i1 >> 0);

		DWORD offset1 = 11;
		DWORD offset2 = 60;
			
		for (DWORD i2 = 0; i2 < 8; i2++)
		{
			if (i1 + i2 >= len)
			{
				chars[offset1] = L' ';
				chars[offset1 + 1] = L' ';
				chars[offset1 + 2] = L' ';
				chars[offset1 + 3] = L' ';
				chars[offset2] = L' ';
			}
			else
			{
				WORD b = bytes[i1 + i2];
				chars[offset1] = HexCharW(b >> 12);
				chars[offset1 + 1] = HexCharW(b >> 8);
				chars[offset1 + 2] = HexCharW(b >> 4);
				chars[offset1 + 3] = HexCharW(b);
				chars[offset2] = (b < 32 ? L'·' : (wchar_t)b);
			}
			offset1 += (i2 == 3 ? 6 : 5);
			offset2++;
		}

		result.append(chars,CHARS_SIZE);
//		result.append(L"\r\n");
	}

//	result.append(L"\r\n");
	#ifdef BRLLOG_VFL
	VflLogCStringRaw(m_hLog,0x01,result.c_str());
	#else
	DWORD nWritten = 0;
	WriteFile(hFile,result.data(),result.size()*sizeof(wchar_t),&nWritten,NULL);
#ifndef FILE_STAY_OPEN
	CloseHandle(hFile);
#endif
#endif

}


void CBralLog::_vLogPrintf(const wchar_t* format,va_list parameter)
{

#ifdef BRLLOG_VFL
	VflvLogPrintf(m_hLog,0x01,format,parameter);
#else

	HANDLE hFile = MyCreateFile();
	if(hFile!=INVALID_HANDLE_VALUE)
	{

		WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);

		SYSTEMTIME st;
		DWORD lt;
		GetLocalTime(&st);
		DWORD nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"[%08x]%02u:%02u:%02u:%03u ",m_ProcessId,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
//		SetFilePointer(hFile,0,NULL,FILE_END);
//		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
		nWr += vswprintf(m_LogBuf+nWr,LOG_BUF_SIZE-nWr,format,parameter);
//		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
		wcsncpy(m_LogBuf+nWr,NEW_LINE,LOG_BUF_SIZE-nWr);
		nWr += ((sizeof(NEW_LINE)/sizeof(wchar_t))-1);
//		WriteFile(hFile,NEW_LINE,4,&lt,NULL);
		SetFilePointer(hFile,0,NULL,FILE_END);
		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
#ifndef FILE_STAY_OPEN
		CloseHandle(hFile);
#endif
		if(m_OutputDebugString)
			OutputDebugStringW(m_LogBuf);
	}
#endif
}

void CBralLog::_LogS2(const wchar_t* s1,const wchar_t* s2,const wchar_t* str)
{
#ifdef BRLLOG_VFL
	VflLogPrintf(m_hLog,0x01,L"%s%s %s",s1,s2,str);
#else

	HANDLE hFile = MyCreateFile();
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);

		SYSTEMTIME st;
		DWORD lt;
		GetLocalTime(&st);
		LONG nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"[%08x]%02u:%02u:%02u:%03u %s%s %s%s",m_ProcessId,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,s1,s2,str,NEW_LINE);
//		wcsncpy(m_LogBuf+nWr,str,LOG_BUF_SIZE-nWr);
		

		SetFilePointer(hFile,0,NULL,FILE_END);
		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
//		WriteFile(hFile,str,wcslen(str)*sizeof(wchar_t),&lt,NULL);
//		WriteFile(hFile,NEW_LINE,4,&lt,NULL);
#ifndef FILE_STAY_OPEN
		CloseHandle(hFile);
#endif
		if(m_OutputDebugString)
			OutputDebugStringW(m_LogBuf);
		
	}
#endif

}


void CBralLog::_Log(const wchar_t* str)
{

#ifdef BRLLOG_VFL
	VflLogCString(m_hLog,0x01,str);
#else
	HANDLE hFile = MyCreateFile();
	if(hFile!=INVALID_HANDLE_VALUE)
	{

		WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);
		SYSTEMTIME st;
		DWORD lt;
		GetLocalTime(&st);
		LONG nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"[%08x]%02u:%02u:%02u:%03u %s%s",m_ProcessId,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,str,NEW_LINE);
		SetFilePointer(hFile,0,NULL,FILE_END);
		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
//		WriteFile(hFile,str,wcslen(str)*sizeof(wchar_t),&lt,NULL);
//		WriteFile(hFile,NEW_LINE,4,&lt,NULL);
#ifndef FILE_STAY_OPEN
		CloseHandle(hFile);
#endif
		if(m_OutputDebugString)
			OutputDebugStringW(m_LogBuf);
		
	}
#endif

	
}


void CBralLog::_vLogPrintfS2(const wchar_t* s1,const wchar_t* s2,const wchar_t* format,va_list parameter)
{
#ifdef BRLLOG_VFL
	WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);
	LONG nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"%s%s",s1,s2);
	nWr += vswprintf(m_LogBuf+nWr,LOG_BUF_SIZE-nWr,format,parameter);
	VflLogCString(m_hLog,0x01,m_LogBuf);
#else
	HANDLE hFile = MyCreateFile();
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		WSTRING_AP	m_LogBuf(LOG_BUF_SIZE);
		SYSTEMTIME st;
		DWORD lt;
		GetLocalTime(&st);
		LONG nWr = _snwprintf(m_LogBuf,LOG_BUF_SIZE,L"[%08x]%02u:%02u:%02u:%03u %s%s",m_ProcessId,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,s1,s2);
		nWr += vswprintf(m_LogBuf+nWr,LOG_BUF_SIZE-nWr,format,parameter);
		if(nWr<LOG_BUF_SIZE)
		{
			wcsncpy(m_LogBuf+nWr,NEW_LINE,LOG_BUF_SIZE-nWr);
			nWr += (sizeof(NEW_LINE)/sizeof(wchar_t))-1;
		}

		SetFilePointer(hFile,0,NULL,FILE_END);
		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
//		WriteFile(hFile,m_LogBuf,nWr*sizeof(wchar_t),&lt,NULL);
//		WriteFile(hFile,NEW_LINE,4,&lt,NULL);
#ifndef FILE_STAY_OPEN
		CloseHandle(hFile);
#endif
		if(m_OutputDebugString)
			OutputDebugStringW(m_LogBuf);
	}
#endif
}


void CBralLog::_LogRaw(const wchar_t* str,DWORD dwLen)
{

#ifdef BRLLOG_VFL
	VflLogCStringRaw(m_hLog,0x01,str);
#else

	HANDLE hFile = MyCreateFile();
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD lt;
		SetFilePointer(hFile,0,NULL,FILE_END);
		WriteFile(hFile,str,dwLen*sizeof(wchar_t),&lt,NULL);
#ifndef FILE_STAY_OPEN
		CloseHandle(hFile);
#endif
	}
#endif

	
}


#ifndef BRAL_NO_GLOBAL_HELPERS
CBralLog BralLog;
#endif