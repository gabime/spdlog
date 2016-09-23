#pragma once

//*****************************************************************************
// String-Autopointer zum automatischen Verwalten von lokalen Puffern, die im
// Freispeicher angelegt werden. Der neu erzeugte Puffer wird initialisiert.
// Nach dem Verlassen des Gültigkeitsbereiches wird der Puffer automatisch 
// durch den Destruktor freigegeben. 
//*****************************************************************************
class STRING_AP
{
public:
	char* s;
	
	operator char*() {return s;}
	operator BYTE*() {return (BYTE*)s;}
	STRING_AP(DWORD size){s=NULL;s=new char[size+1];s[0]='\0';s[size]='\0';};
	~STRING_AP(){if(s) delete[] s; s = NULL;};
};


class WSTRING_AP
{
public:
	wchar_t* s;
	
	operator wchar_t*() {return s;}
	WSTRING_AP(DWORD size){s=NULL;s=new WCHAR[size+1];s[0]=L'\0';s[size]=L'\0';};
	~WSTRING_AP(){if(s) delete[] s; s = NULL;};
};

class STRINGP
{
public:
	const char* s;
	
//	operator char*() {if(s) return s;return "";}
//	operator BYTE*() {return (BYTE*)operator char*();}
	STRINGP(const char* str){s = "";if(str) s=(char*)str;};
	~STRINGP(){s = "";};
};


class WSTRINGP
{
public:
	const wchar_t* s;
	
//	operator wchar_t*() {if(s)return s;return L"";}
	WSTRINGP(const wchar_t* lps){s = L""; if(lps) s=(wchar_t*)lps;};
	~WSTRINGP(){s = NULL;};
};


#ifdef  UNICODE                     // r_winnt
	typedef WSTRING_AP	TSTRING_AP;
#else
	typedef STRING_AP TSTRING_AP;
#endif
