#ifndef ____INC___STRT_H____CNI_____
#define ____INC___STRT_H____CNI_____

/*
###############################################################################
Interface StrT
###############################################################################

*******************************************************************************
	STR* c_ptr (void)            // not necessarily NUL terminated!
	Liefert einen Zeiger auf den Puffer, ohne die Terminierung zu beachten
*******************************************************************************
	STR const* c_str (void)    // return char*
	Liefert terminierten String
*******************************************************************************
	long length (void)
*******************************************************************************

*******************************************************************************
	long Len (void)
*******************************************************************************

//*****************************************************************************
	long Size (void)		// Größe des belegten Puffers
*******************************************************************************

*******************************************************************************
	void ReserveBuffer(long ReqSize)	// Puffergröße direkt modifizieren
*******************************************************************************




//*****************************************************************************
// Constructor's of different kind
//*****************************************************************************
	StrT(void)
	StrT(STR* s)
	StrT(STR const* s)
	StrT(STR const * s, long len)
	StrT(STR c)
	StrT(unsigned long val)
	StrT(int val)
	StrT(StrT  & s)


*******************************************************************************
	StrT& Set(StrT& s)		// Überträgt Inhalt von anerem String
*******************************************************************************
	StrT & Clear(void)		// Löscht String
*******************************************************************************
	// Exported, but for use with very good care!!!
	void __SetLen(long len)		// !!! Vorsicht, direkte Längenmanipulation!!!!
*******************************************************************************


*******************************************************************************
	// "=" - Operators
*******************************************************************************
	StrT & operator= (StrT const & s)
	StrT & operator= (STR const * s)
	StrT & operator= (STR c)
	StrT & operator= (long n)
	StrT & operator= (unsigned long n)

*******************************************************************************
	// Additionsoperatoren
*******************************************************************************
	StrT& operator+=(const StrT& s)

*******************************************************************************
// String abschneiden
*******************************************************************************
	long TruncateLeft(long nElem)
	long TruncateRight(long nElem)

*******************************************************************************
// copy the data into a external buffer
*******************************************************************************
	int	Copy(STR * dst, long maxlen)

//*****************************************************************************
// Compare two strings
//*****************************************************************************
	int Compare (StrT & s)
	int Compare (STR const * s)

//*****************************************************************************
// Offset eines Substrings im String finden
//*****************************************************************************
	long StrStr (StrT  & s)
	long StrStr (STR const * s)

//*****************************************************************************
// Zeichen / String einfügen an Position 
//*****************************************************************************
	long InsertElem (long pos, STR c)
	long Insert (long pos, STR const * s, long len = -1)

//*****************************************************************************
// Stück aus String entfernen
//*****************************************************************************
	long Remove (long pos, long len)

//*****************************************************************************
// Ersetzen eines Teilstrings
//*****************************************************************************
	long Replace (long pos, StrT & s, long clen =-1)
	long Replace (long pos, STR c, long clen =-1)
	long Replace (long pos, STR const * s, long clen = -1, long len = -1)

//*****************************************************************************
//*****************************************************************************
	StrT & Left (long len, STR padch)
	StrT & Right (long len, STR padch)
	StrT & Mid (long pos, long len, STR padch)


//*****************************************************************************
//*****************************************************************************
	StrT Substr(long start, long len)

//*****************************************************************************
//*****************************************************************************
	int _concat (StrT & s)
	int _concat (STR ch)
	int _concat (const STR * s, long len = -1)

//*****************************************************************************
// Vergleichs-Operatoren
//*****************************************************************************
	bool operator== (StrT  & s)
	bool operator== (STR * s) 
	bool operator!= (StrT & s)
	bool operator!= (STR * s) 
	bool operator< (StrT & s)
	bool operator< (STR * s) 
	bool operator<= (StrT & s)
	bool operator<= (STR  * s)
	bool operator> (StrT & s)
	bool operator> (STR * s)
	bool operator>= (StrT & s) 
	bool operator>= (STR * s) 


//*************************************************************************
// Konvertierungsoperatoren char + wchar
//*************************************************************************
	StrT & operator<< (const char* s)
	StrT & operator<< (const wchar_t* s)
	StrT & operator<< (StrT  & s)
	StrT & operator<< (int val)
	StrT& operator<< (unsigned int val)
	StrT & operator<< (short val)
	StrT & operator<< (unsigned short val)
	StrT& operator<< (long val)
	StrT & operator<< (unsigned long val)
	StrT& operator<< (C_StreamIoManipulator& manip)

	STR const & operator[] (long pos) const
	STR & operator[] (long pos)

//*****************************************************************************
//*****************************************************************************
	int _compare(StrT s)
	int __compare(StrT const s) const

//*****************************************************************************
//*****************************************************************************
	long _strstr(StrT  s)
//*****************************************************************************
//*****************************************************************************
	long RemoveCh (STR const * clist)

//*****************************************************************************
//*****************************************************************************
	long CountCh (STR const * clist)

//*****************************************************************************
//*****************************************************************************
	StrT& toUpper(void)
	StrT& toLower(void)


//*****************************************************************************
//*****************************************************************************
	StrT& Trim(unsigned long mode, STR c)

//*****************************************************************************
//*****************************************************************************
	unsigned long CreateChecksum(void)

//*****************************************************************************
//*****************************************************************************
	void ltos(const long n, int radix = -1)
	void ultos(const unsigned long n, int radix = -1)

//*****************************************************************************
//*****************************************************************************
	long& Value(long& n, int radix = -1)
	unsigned long& Value(unsigned long& n, int radix = -1)

//*****************************************************************************
//*****************************************************************************
	int GetNumberConvertBase(void)
	void SetNumberConvertBase(int base)

//*****************************************************************************
// Wenn CaseSens TRUE ist, werden Vergleiche case-sensitiv durchgeführt.
//*****************************************************************************
	void SetCompareCase(bool CaseSens)

//*****************************************************************************
// Index: liefert den Offset im String nach AWK-Manier
//*****************************************************************************
	long Index(const STR* strCharset)
	long IndexBin(long start,STR* chr,long nPat)

//*****************************************************************************
// Erstes Vorkommen von str finden. Die Variable pos wird initialisiert und 
// wird bei nächsten Such-Durchläuifen werwendet. 
//*****************************************************************************
	long& FindFirst(const STR* str,long& pos)
	long& FindFirst(const StrT& str,long& pos)

//*****************************************************************************
// Find the next occurrence of substring 'findStr' in the string
// pos ist die von FindFirst initialisierte Variable zum Zwischenspeichern
// des aktuellen Ergebnisses.
//*****************************************************************************
	long& FindNext(const STR* str,long& pos)
	long& FindNext( StrT& str,long& pos)
	
*******************************************************************************
*******************************************************************************

###############################################################################
Interface StrTArray
###############################################################################
typedef StrTArray<StrW,StrWLlEntry, wchar_t> StrWArray;
typedef StrTArray<StrC,StrCLlEntry, char> StrCArray;
*******************************************************************************
	STRINGOBJ& push_back(STRINGOBJ& ob)					// String einfügen
	bool GetString(STRINGOBJ& str,unsigned long idx)	// Auf String per Index zugr.
	STRINGOBJ* GetString(unsigned long idx)
	void Clear(void)									// Array leermachen
*******************************************************************************
	// StrX in das Array aufsplitten, jedes gefundene Trennzeichen (charset)
	// erzeugt einen neuen Arrayeintrag. 
	unsigned long Split(STRINGOBJ& str,STR* charset,long nChr)
*******************************************************************************
*******************************************************************************


*/

#include <stdlib.h>
#include <wchar.h>
//#include "Snip_str.h"
#include "V_Strx_Iomanip.h"
#include "LlistT.h"
#include "V_MemoryAllocator.h"
#include "StrBBuffer.h"
#include "StrTBuffer.h"

# define	STDLEN			32
//#define		BLOCKSIZE		64

// Define to 0 for much smaller class
#define VIRTUAL_DESTRUCTOR  0
#if VIRTUAL_DESTRUCTOR
#define __VIRTUAL   virtual
#else
#define __VIRTUAL
#endif

char *stristr(const char *String,const char *Pattern);
wchar_t *wcsistr(const wchar_t *String, const wchar_t *Pattern);

/*
enum 
{ 
	LEFT   = 0,
	CENTER = 1,
	RIGHT  = 2,
	NOCLIP = 0,
	NOTRIM = 0,
	CLIP   = 1,
	TRIM   = 2,
	WHITESPACE = 0 
}STRING_T_MODES;
*/

const unsigned int	STRTTMODE_LEFT			=	0;
const unsigned int	STRTTMODE_CENTER		=	1;
const unsigned int	STRTTMODE_RIGHT			=	2;

const unsigned int	STRTTMODE_NOCLIP		=	0;
const unsigned int	STRTTMODE_NOTRIM		=	0;
const unsigned int	STRTTMODE_CLIP			=	1;
const unsigned int	STRTTMODE_TRIM			=	2;

const unsigned int	STRTTMODE_WHITESPACE	=	0;

// Die ersten zwei Bits repräsentieren den Zahlen-Konvertierungs-
// Modus. Macht natürlich nur einer der drei Sinn.
const unsigned int	STRTTFLAG_DEC			=	0x00;
const unsigned int	STRTTFLAG_HEX			=	0x01;
const unsigned int	STRTTFLAG_OCT			=	0x02;
const unsigned int	STRTTFLAG_CASE			=	0x04;

const unsigned int	STRTT_DEFAULT_FLAG		=	STRTTFLAG_DEC;

// Globale Funtionsprototypen
char* StrT_WideCharToCharString(const wchar_t* txt);
wchar_t* StrT_CharToWideString(const char* txt);


template<class STR> 
class WIDECHAR_SUPPORT
{
	public:
	// Ersatz-Funktionen zum Mapping der unterschiedlichen OS-Funktionen
	// gemäß dem Template-Typ (wchar/char)
	size_t __strlen(const wchar_t* str){return wcslen(str);};
	STR* __ultoa(unsigned long val,wchar_t* str,int radix){return _ultow(val,str,radix);};
	STR* __itoa(int val,wchar_t* str,int radix){return _itow(val,str,radix);};
	int __stricmp(const wchar_t* s1,const wchar_t* s2){return _wcsicmp(s1,s2);};
	int __strcmp(const wchar_t* s1,const wchar_t* s2){return wcscmp(s1,s2);};
	STR* __strstr(const wchar_t* str,const wchar_t* strCharset){return (STR*) wcsstr(str,strCharset);};
	STR* __stristr(const wchar_t* str,const wchar_t* strCharset){return (STR*) wcsistr(str,strCharset);};
	STR* __strchr(const wchar_t* str,int c){return wcschr(str,c);};
	STR* __strupr(wchar_t* str){return _wcsupr(str);};
	STR* __strlwr(wchar_t* str){return _wcslwr(str);};
	long __strtol(const wchar_t* str,wchar_t** endptr,int base){return wcstol(str,endptr,base);};
	unsigned long __strtoul(const wchar_t* str,wchar_t** endptr,int base){return wcstoul(str,endptr,base);};
	size_t __strcspn(const wchar_t* str,const wchar_t* strCharSet){return wcscspn(str,strCharSet);};

	// Funktionen zur Konvertierung von Standardstrings in STR-Ketten
	// Nach Gebrauch ist die entsprechende Free-Funktion aufzurufen. Auf keinen
	// Fall selbst freigeben!
	STR* __csconvert(const char* str){return StrT_CharToWideString(str);};
	void __csconvert_free(STR* data){if(data) delete data;};
	STR* __wcsconvert(const wchar_t* str){return (STR*)str;};
	void __wcsconvert_free(STR* data){};

	// Funktion für Checksummen-Wert eines Elementes (bei char und WCHAR einfach der 
	// Zeichenwert)
	unsigned long __get_element_checksum_value(wchar_t ch){return (unsigned long) ch;};

	// Ersatz-Funktionen zur Puffermanipulation
	STR* __block_move(wchar_t* dest,const wchar_t* src, size_t count){return (STR*)memmove(dest,src,count*sizeof(wchar_t));};
	STR* __block_copy(wchar_t* dest,const wchar_t* src, size_t count){return (STR*)memcpy(dest,src,count*sizeof(wchar_t));};
	STR* __block_set(wchar_t* dest,wchar_t val,size_t count)
	{
		for(size_t i=0;i<count;i++)
			dest[i] = val;
		return (STR*) dest;
	};

	// Konstanten wie z.B. für NULL-Terminierung 
	STR __std_terminating_element(void){return L'\0';};
	STR __std_whitespace_element(void){return L' ';};
	STR* __std_eol_element(void){return L"\r\n";};

};


template<class STR>
class CHAR_SUPPORT
{
	public:
	// Ersatz-Funktionen zum Mapping der unterschiedlichen OS-Funktionen
	// gemäß dem Template-Typ (wchar/char)
	size_t __strlen(const char* str){return strlen(str);};
	STR* __ultoa(unsigned long val,char* str,int radix){return _ultoa(val,str,radix);};
	STR* __itoa(int val,char* str,int radix){return _itoa(val,str,radix);};
	int __stricmp(const char* s1,const char* s2){return _stricmp(s1,s2);};
	int __strcmp(const char* s1,const char* s2){return strcmp(s1,s2);};
	STR* __strstr(const char* str,const char* strCharset){return (char*)strstr(str,strCharset);};
	STR* __stristr(const char* str,const char* strCharset){return stristr(str,strCharset);};
	STR* __strchr(const char* str,int c){return strchr(str,c);};
	STR* __strupr(char* str){return _strupr(str);};
	STR* __strlwr(char* str){return _strlwr(str);};
	long __strtol(const char* str,char** endptr,int base){return strtol(str,endptr,base);};
	unsigned long __strtoul(const char* str,char** endptr,int base){return strtoul(str,endptr,base);};
	size_t __strcspn(const char* str,const char* strCharSet){return strcspn(str,strCharSet);};

	// Funktionen zur Konvertierung von Standardstrings in STR-Ketten
	// Nach Gebrauch ist die entsprechende Free-Funktion aufzurufen. Auf keinen
	// Fall selbst freigeben!
	STR* __csconvert(const char* str){return (STR*)str;};
	void __csconvert_free(STR* data){};
	STR* __wcsconvert(const wchar_t* str){return StrT_WideCharToCharString(str);};
	void __wcsconvert_free(STR* data){if(data) delete data;};


	// Funktion für Checksummen-Wert eines Elementes (bei char und WCHAR einfach der 
	// Zeichenwert)
	unsigned long __get_element_checksum_value(char ch){return (unsigned long) ch;};


	// Auch die Memory-Operationen sollten anpassbar sein (z.B. tut memset nur bei 
	// char's veernünftig)
	STR* __block_move(char* dest,const char* src, size_t count){return (STR*)memmove(dest,src,count);};
	STR* __block_copy(char* dest,const char* src, size_t count){return (STR*)memcpy(dest,src,count*sizeof(char));};
	STR* __block_set(char* dest,int val,size_t count){return (STR*)memset(dest,val,count);};


	STR __std_terminating_element(void){return '\0';};
	STR __std_whitespace_element(void){return ' ';};
	STR* __std_eol_element(void){return "\r\n";};

};



#pragma pack(push, 1)
template<typename STR,
typename SUPPORT,
typename ALLOC>
class StrT : public StrBBuffer<STR>
{
	private:
//	StrTBuffer<STR, ALLOC>	m_Str;
//	StrBBuffer<STR>	m_Str;
	SUPPORT			SupportFn;
	unsigned long	m_Flag;	

	void InitPrivateData(void)
	{
		m_Flag = STRTT_DEFAULT_FLAG;
	}
	

	// Private Member
	//*****************************************************************************
	// private initialisation functions
	//*****************************************************************************
	void _strinit (STR const * s, long len, /*long siz,*/ unsigned long flgs)
	{
		m_Flag = flgs;
		_strinit(s,len);
	}

	void _strinit (STR const * s, long len /*long siz, unsigned long flgs*/)
	{
	if (len < 0)
		len = (long) ((s) ? SupportFn.__strlen (s) : 0);
	if(s==NULL)
		StrBBuffer<STR>::clear();
	else
		StrBBuffer<STR>::Set((STR*)s,len);

	};



	void _strinit_n (unsigned long val, int radix)
	{
		if(radix==-1)
			radix = GetNumberConvertBase();

		STR  buf[32], * p = buf;

		SupportFn.__ultoa(val, buf, radix);

		_strinit(buf, -1);
	}

	void _strinit_n (int  val, int radix)
	{
		STR  buf[32];

		if(radix==-1)
			radix = GetNumberConvertBase();

		SupportFn.__itoa(val, buf, radix);
		_strinit(buf, -1);
	}



	void __strinit (STR const * s =0, long slen =0,
					unsigned long flgs = 0)
	{
		_strinit(s,slen,flgs);
	};



	//*****************************************************************************
	// to access the data directly over PTR: internally used
	//*****************************************************************************
	public:
	STR* c_ptr (void)            // not necessarily NUL terminated!
	{                                                // Use with caution...
		return StrBBuffer<STR>::Get();
	}


	//*****************************************************************************
	//*****************************************************************************
	public:
	STR const* c_str (void)    // return char*
	{

		return StrBBuffer<STR>::Get(SupportFn.__std_terminating_element(),NULL);
	}


	//*****************************************************************************
	// length returns len of string
	//*****************************************************************************
	public:
	long length (void)
	{
		// Null-Terminator zählt nicht und muß vorher weg!
		STR sTerm = SupportFn.__std_terminating_element();
		StrBBuffer<STR>::TruncateChr(sTerm,false,true);

		return StrBBuffer<STR>::Len();
	}

	long Len (void)
	{
		// Null-Terminator zählt nicht und muß vorher weg!
		STR sTerm = SupportFn.__std_terminating_element();
		StrBBuffer<STR>::TruncateChr(sTerm,false,true);

		return StrBBuffer<STR>::Len();
	}

	//*****************************************************************************
	// size returns the buffer's size
	//*****************************************************************************
	long Size (void)
	{
		// Null-Terminator zählt nicht und muß vorher weg!
		STR sTerm = SupportFn.__std_terminating_element();
		StrBBuffer<STR>::TruncateChr(sTerm,false,true);

		return (long)StrBBuffer<STR>::size();
	}

	void ReserveBuffer(long ReqSize,STR& FillElem)
	{
		StrBBuffer<STR>::ReserveBuffer(ReqSize,FillElem);
	}



public:

	//*****************************************************************************
	// Constructor's of different kind
	//*****************************************************************************
	StrT(void)
	{
//		OutputDebugString("Constructor void\r\n");
		InitPrivateData();
		_strinit(NULL,-1);
	};

	StrT(STR* s)
	{
//		OutputDebugString("Constructor STR*\r\n");
		InitPrivateData();
		_strinit(s,-1);
	};

	StrT(STR const* s)
	{
//		OutputDebugString("Constructor STR*\r\n");
		InitPrivateData();
		_strinit(s,-1);
	};


	StrT(STR const * s, long len)
	{
//		OutputDebugString("Constructor STR* + len\r\n");
		InitPrivateData();
		_strinit(s, len);
	};


	StrT(STR c)
	{
//		OutputDebugString("Constructor STR\r\n");
		InitPrivateData();
		_strinit (&c, 1);
	}

	StrT(unsigned long val)
	{
		InitPrivateData();
//		OutputDebugString("Constructor long\r\n");
		_strinit_n(val,-1);
	};

	StrT(int val)
	{
//		OutputDebugString("Constructor int\r\n");
		InitPrivateData();
		_strinit_n(val,-1);
	};

	StrT(StrT  & s)
//	  : strdata(s.strdata)
	{
		// Übernahme des Flags vom Anderen
		m_Flag = s.m_Flag;
		_strinit(s.c_ptr(),s.length());
//		OutputDebugString("Constructor const StrT: ref++\r\n");
	}


	//*****************************************************************************
	// Destructor
	//*****************************************************************************
	~StrT(void)
	{
//		OutputDebugString("Destructor\r\n");
		InitPrivateData();
	}

	StrT& Set(StrT& s)
	{
		StrBBuffer<STR>::Set(s.c_ptr(),s.length());
		return *this;
	}

	StrT& Set(const STR* s,long nCh)
	{
		if(s)
			StrBBuffer<STR>::Set(s,nCh);
		return *this;
	}


	//*****************************************************************************
	// deletes the data or decrements the buffer's reference counter
	//*****************************************************************************
	StrT & Clear(void)
	{
		StrBBuffer<STR>::Clear();
		return *this;
	}

	//*****************************************************************************
	// Exported, but for use with very good care!!!
	//*****************************************************************************
	void __SetLen(long len)
	{
		StrBBuffer<STR>::SetLen(len);
	}

	//*****************************************************************************
	// Checks the buffer and does CopyOnWrite operation to duplicate buffer
	//*****************************************************************************
private:

	//*****************************************************************************
	// "=" - Operators
	//*****************************************************************************
public:
	//StrT & operator= (StrT const & s)
	//{
	//	StrBBuffer<STR>:: = s; //.m_Str;

	//	return *this;
	//}


	StrT & operator= (STR const * s)
	{
		_strinit(s,-1);

		return *this;
	}

	StrT & operator= (STR c)
	{
		_strinit(&c,-1);
		return *this;
	}

	StrT & operator= (long n)
	{
		ltos(n);
		return *this;
	}

	StrT & operator= (unsigned long n)
	{
		ultos(n);
		return *this;
	}

	//*************************************************************************
	// Additionsoperatoren
	//*************************************************************************
	StrT& operator+=(const StrT& s)
	{
		_concat((StrT<STR,SUPPORT,ALLOC>&)s);
//		m_Str += s.m_Str;
		return *this;
	}

	//*****************************************************************************
	//*****************************************************************************
	long TruncateLeft(long nElem)
	{
		return StrBBuffer<STR>::TruncateLeft(nElem);
	}

	long TruncateRight(long nElem)
	{
		// Null-Terminator zählt nicht und muß vorher weg!
		STR sTerm = SupportFn.__std_terminating_element();
		StrBBuffer<STR>::TruncateChr(sTerm,false,true);

		return StrBBuffer<STR>::TruncateRight(nElem);
	}

	//*****************************************************************************
	// copy the data into a external buffer
	//*****************************************************************************
	int	Copy(STR * dst, long maxlen)
	{
		if(dst==NULL)
			return 0;
		if (maxlen == -1)
			maxlen = long(length() + 1);
		long len = long(maxlen - 1);
		if (len > length())
			len = length();
		if (len > 0)
			SupportFn.__block_copy(dst, c_ptr(), len);
		if (len >= 0)
			dst[len] = SupportFn.__std_terminating_element();
		return len;
	}


	//*****************************************************************************
	// Compare two strings
	//*****************************************************************************
	int Compare (StrT & s)
	{
		return _compare(s);
	}

	int Compare (StrT & s, bool CompareCase)
	{
		return Compare(s.c_str(),CompareCase);
	}


	int Compare (STR const * s)
	{
		// Wenn NULL-Vergleich, ist String immer größer als Vergl.Str.
		if(s==NULL)
			return 1;
		
		if(m_Flag&STRTTFLAG_CASE)
			return SupportFn.__stricmp(c_str(), s);
		return SupportFn.__strcmp(c_str(), s);

	}

	int Compare (STR const * s, bool CompareCase)
	{
		// Wenn NULL-Vergleich, ist String immer größer als Vergl.Str.
		if(s==NULL)
			return 1;
		
		if(CompareCase)
			return SupportFn.__strcmp(c_str(), s);
		return SupportFn.__stricmp(c_str(), s);

	}


private:
	int __compare_int(STR const* s1, STR const* s2)
	{

		if(m_Flag&STRTTFLAG_CASE)
			return SupportFn.__stricmp(s1, s1);
		return SupportFn.__strcmp(s1, s2);
	}

public:
	//*****************************************************************************
	// Offset eines Substrings im String finden
	//*****************************************************************************
	long StrStr (StrT  & s)
	{
		return _strstr(s);
	}

	long StrStr (STR const * s)
	{
		return _strstr(s);
	}



	//*****************************************************************************
	// String einfügen an Position 
	//*****************************************************************************
/*    long insert (long pos,  StrT & s)
	{
		return insert (pos, s.c_ptr(), s.length());
	}
*/
	long InsertElem (long pos, STR c)
	{
		return Insert (pos, &c, 1);
	}


/*
	long insert (long pos, const STR&  s, long len)
	{
		insert(pos,&s,len);
	}
*/


	long Insert (long pos, STR const * s, long len = -1)
	{
		if(s==0)
			return length();
		
		if (len < 0)
			len = (long) SupportFn.__strlen (s);

		return StrBBuffer<STR>::Insert(pos,s,len);
	}



	//*****************************************************************************
	// Stück aus String entfernen
	//*****************************************************************************
	long Remove (long pos, long len)
	{
		return StrBBuffer<STR>::remove(pos,len);
	}

	//*****************************************************************************
	// Ersetzen eines Teilstrings
	//*****************************************************************************
	long Replace (long pos, StrT & s, long clen =-1)
	{
		return Replace (pos, s.c_ptr(), clen, s.length());
	}
/*    
	long replace (long pos, STR const * s, long clen =-1, long len =-1)
	{
		return replace (pos, s, clen, len);
	}
*/
	long Replace (long pos, STR c, long clen =-1)
	{
		return Replace (pos, &c, clen, 1);
	}


	long Replace (long pos, STR const * s, long clen = -1, long len = -1)
	{
		if(s==NULL)
			return StrBBuffer<STR>::Len();

		if (len < 0)
			len = (long) SupportFn.__strlen (s);

		return StrBBuffer<STR>::replace(pos,s,clen,len);
	}


	//*****************************************************************************
	//*****************************************************************************
	StrT & Left (long len, STR padch)
	{
		StrBBuffer<STR>::left(len,padch);

		return *this;
	}

	//*****************************************************************************
	//*****************************************************************************
	StrT & Right (long len, STR padch)
	{
		StrBBuffer<STR>::right(len,padch);

		return *this;
	}

	//*****************************************************************************
	//*****************************************************************************
	StrT & Mid (long pos, long len, STR padch)
	{
		StrBBuffer<STR>::mid(pos,len,padch);

		return *this;
	}


	//*****************************************************************************
	//*****************************************************************************
	StrT Substr(long start, long len)
	{
		if (start < 0)
			start = long(length() + (start));
		if (start < 0 || start >= StrBBuffer<STR>::Len())
			return StrT();   // Empty
		if (len < 0 || (long(start + len) > StrBBuffer<STR>::Len()))
			len = long(StrBBuffer<STR>::Len() - start);
		return StrT(c_ptr() + (start), len);

	}


	//*****************************************************************************
	//*****************************************************************************
	int _concat (StrT & s)
	{
		return _concat (s.c_ptr(), s.length());
	}

	int _concat (STR ch)
	{
		return _concat (&ch, 1);
	}


	int _concat (const STR * s, long len = -1)
	{
		if(s==NULL)
			return length();

		if (len < 0)
			len = (long) SupportFn.__strlen (s);
		if (len > 0)
		{
			StrBBuffer<STR>::Add(s,len,SupportFn.__std_terminating_element());
		}

		return length();
	}




//    bool operator== (StrT const & s) const
	bool operator== (StrT  & s)
	{
		return (_compare(s) == 0) ? true : false;
	}

	bool operator== (STR /*const*/ * s) /*const*/
	{
		return (_compare(s) == 0) ? true : false;
	}


	bool operator!= (StrT /*const*/ & s) /*const*/
	{
		return (_compare(s) != 0) ? true : false;
	}

	bool operator!= (STR /*const*/ * s) /*const*/
	{
		return (_compare(s) != 0) ? true : false;
	}

	bool operator< (StrT /*const*/ & s) /*const*/
	{
		return (_compare(s) < 0) ? true : false;
	}

	bool operator< (STR /*const*/ * s) /*const*/
	{
		return (_compare(s) < 0) ? true : false;
	}


	bool operator<= (StrT /*const*/ & s) /*const*/
	{
		return (_compare(s) <= 0) ? true : false;
	}

	bool operator<= (STR /*const*/ * s) /*const*/
	{
		return (_compare(s) <= 0) ? true : false;
	}


	bool operator> (StrT /*const*/ & s) /*const*/
	{
		return (_compare(s) > 0) ? true : false;
	}

	bool operator> (STR /*const*/ * s) /*const*/
	{
		return (_compare(s) > 0) ? true : false;
	}

	bool operator>= (StrT /*const*/ & s) /*const*/
	{
		return (_compare(s) >= 0) ? true : false;
	}

	bool operator>= (STR /*const*/ * s) /*const*/
	{
		return (_compare(s) >= 0) ? true : false;
	}

/*
	StrT & operator<< (STR const * s)    // concatenate
	{
		_concat (s);
		return *this;
	}
*/
	//*************************************************************************
	// Stream-in-Operatoren
	//*************************************************************************

	//*************************************************************************
	// Konvertierungsoperatoren char + wchar
	//*************************************************************************
	StrT & operator<< (const char* s)
	{
		STR* lpStr = SupportFn.__csconvert(s);
		if(lpStr)
		{
			_concat (lpStr);
			SupportFn.__csconvert_free(lpStr);
		}
		return *this;
	}

	StrT & operator<< (const wchar_t* s)
	{
		STR* lpStr = SupportFn.__wcsconvert(s);
		if(lpStr)
		{
			_concat (lpStr);
			SupportFn.__wcsconvert_free(lpStr);
		}
		return *this;
	}
	

	StrT & operator<< (StrT  & s)
	{
		_concat (s.c_ptr(),s.length());
		return *this;
	}

	StrT & operator<< (int val)
	{
		StrT<STR,SUPPORT,ALLOC> ts;
		ts.ltos(val,GetNumberConvertBase());
		_concat(ts);
//		_concat (StrT(val));
		return *this;
	}

	StrT& operator<< (unsigned int val)
	{
		StrT<STR,SUPPORT,ALLOC> ts;
		ts.ultos(val,GetNumberConvertBase());
		_concat(ts);
//		_concat (StrT(val));
		return *this;
	}

	StrT & operator<< (short val)
	{
		StrT<STR,SUPPORT,ALLOC> ts;
		ts.ltos(val,GetNumberConvertBase());
		_concat(ts);
//		_concat (StrT(val));
		return *this;
	}

	StrT & operator<< (unsigned short val)
	{
		StrT<STR,SUPPORT,ALLOC> ts;
		ts.ultos(val,GetNumberConvertBase());
		_concat(ts);
//		_concat (StrT(val));
		return *this;
	}

	StrT& operator<< (long val)
	{
		StrT<STR,SUPPORT,ALLOC> ts;
		ts.ltos(val,GetNumberConvertBase());
		_concat(ts);
//		_concat (StrT(val));
		return *this;
	}

	StrT & operator<< (unsigned long val)
	{
		StrT<STR,SUPPORT,ALLOC> ts;
		ts.ultos(val,GetNumberConvertBase());
		_concat(ts);
//		_concat (StrT(val));
		return *this;
	}

	StrT& operator<< (C_StreamIoManipulator& manip)
	{
		switch(manip.g_ManipId())
		{
		case IOMID_clear:
			// Löschen des Strings 
			Clear();
		  break;

		case IOMID_endl:
			// Endline-Zeichen einbringen
			Insert(-1,SupportFn.__std_eol_element(),-1);
			
//			AddStr("\r\n");
		  break;

		case IOMID_flag:
			switch(manip.g_Manip())
			{
			case IOM_hex:
				SetNumberConvertBase(16);
				break;

			case IOM_dec:
				SetNumberConvertBase(10);
				break;

			case IOM_oct:
				SetNumberConvertBase(8);
				break;

			case IOM_comp_case:
				SetCompareCase(true);
				break;

			case IOM_comp_nocase:
				SetCompareCase(false);
				break;

			};

		  break;
		};


	return *this;
		
	}

/*
	StrT & operator<< (const STR c)
	{
		_concat (c);
		return *this;
	}

	StrT & operator<< (unsigned char c)
	{
		_concat (c);
		return *this;
	}
*/


	//*****************************************************************************
	// String is never modified in this version
	//*****************************************************************************
	STR const & operator[] (long pos) const
	{
		if (pos < 0)            // Negative index addresses from eos
			pos = long(StrBBuffer<STR>::Len() + pos);
		if (pos >= m_Data.Len())
		{
			STR * buf = c_ptr() + length();
			*buf = 0;
			return *buf;
		}
		return c_ptr()[pos];
	}

		// ... but here it may be


	//*****************************************************************************
	//*****************************************************************************
	STR & operator[] (long pos)
	{
		if (pos < 0)                       // Negative index addresses from eos
			pos = long(StrBBuffer<STR>::Len() + pos);
		if (pos < 0)                     // Any cleaner way without exceptions?
			pos = StrBBuffer<STR>::Len();
		if (pos < StrBBuffer<STR>::Len())
		{
			// Nix zu tun.
		}
		else
		{
			for(long i=StrBBuffer<STR>::Len();i<pos;i++)
				StrBBuffer<STR>::AddElem(SupportFn.__std_whitespace_element());
		}
		return c_ptr()[pos];
	}

	//*****************************************************************************
	//*****************************************************************************
	int _compare(StrT /*const*/ s)
	{
		if((m_Flag&STRTTFLAG_CASE)||(s.m_Flag&STRTTFLAG_CASE))
			return SupportFn.__stricmp(c_str(), s.c_str());
		return SupportFn.__strcmp(c_str(), s.c_str());
	}

	int __compare(StrT const s) const
	{
		if((m_Flag&STRTTFLAG_CASE)||(s.m_Flag&STRTTFLAG_CASE))
			return SupportFn.__stricmp(c_str(), s.c_str());
		return SupportFn.__strcmp(c_str(), s.c_str());
	}

	//*****************************************************************************
	//*****************************************************************************
	long _strstr(StrT /*const*/ s)
	{
		STR    *p;

		if((m_Flag&STRTTFLAG_CASE)||(s.m_Flag&STRTTFLAG_CASE))
			p = SupportFn.__strstr(c_str(), s.c_str());
		else
			p = SupportFn.__stristr(c_str(), s.c_str());
		return long((p) ? (p - StrBBuffer<STR>::Get(NULL)) : -1);
	}

	long AddChOnce(STR ch)
	{
		// Entfernen des Terminators
		STR sTerm = SupportFn.__std_terminating_element();
		StrBBuffer<STR>::TruncateChr(sTerm,false,true);
		long dwSize = StrBBuffer::Len();
		if(dwSize>0)
		{
			STR* MyStr = StrBBuffer::Get();
			if((MyStr)&&(MyStr[dwSize-1]==ch))
				return dwSize;
		}

		StrBBuffer::AddElem(ch);
		
		return StrBBuffer::Len();
	}

	//*****************************************************************************
	//*****************************************************************************
	long RemoveCh (STR const * clist)
	{

		long result = 0;
		if (*clist)
		{
			// Kopie des Puffers anlegen und Puffer löschen
			StrTBuffer<STR> ts((StrTBuffer<STR>)this/*m_Str*/);
			StrBBuffer<STR>::Clear();

			STR * buf;
			long nLen = 0;
			buf = ts.Get(&nLen);
			
			for (long i = 0; i < nlen; ++i)
			{
				// Char nicht im Charset? Dann zum String hinzufügen.
				if (SupportFn.__strchr (clist, buf[i]) != 0)
					StrBBuffer<STR>::AddElem(buf[i],SupportFn.__std_terminating_element);
				else
					result++;

			}
		}

		return result;
	}

	//*****************************************************************************
	//*****************************************************************************
	long CountCh (STR const * clist)
	{
		long result = 0;
		if (*clist)
		{
			long nLen;
			STR * buf = StrBBuffer<STR>::Get(&nLen);
			for (long i = 0; i < nlen; ++i, ++buf)
				if (SupportFn.__strchr (clist, *buf) != 0)
					++result;
		}
		return result;
	}


	//*****************************************************************************
	// Funktionen zur Kompatibilität mit V_String-Klasse
	//*****************************************************************************

	//*****************************************************************************
	//*****************************************************************************
	StrT& toUpper(void)
	{
		SupportFn.__strupr((STR*)c_str());		
		return *this;
	};

	//*****************************************************************************
	//*****************************************************************************
	StrT& toLower(void)
	{
		SupportFn.__strlwr((STR*)c_str());		
		return *this;
	};


	//*****************************************************************************
	//*****************************************************************************
	StrT& Trim(unsigned long mode, STR c)
	{

		if(mode == STRTTMODE_RIGHT)
		{
			// Bevor es rechts funktioieren kann, muß der Terminator weg!
			STR sTerm = SupportFn.__std_terminating_element();
			StrBBuffer<STR>::TruncateChr(sTerm,false,true);
			StrBBuffer<STR>::TruncateChr(c,false,true);
		}
		else if(mode == STRTTMODE_LEFT)
			StrBBuffer<STR>::TruncateChr(c,true,false);
		else if(mode == STRTTMODE_CENTER)
		{
			// Bevor es rechts funktioieren kann, muß der Terminator weg!
			STR sTerm = SupportFn.__std_terminating_element();
			StrBBuffer<STR>::TruncateChr(sTerm,false,true);
			StrBBuffer<STR>::TruncateChr(c,true,true);
		}


			
		return *this;
	};

	//*****************************************************************************
	//*****************************************************************************
	unsigned long CreateChecksum(void)
	{
		unsigned long chs = 0;
		long nChr = 0;
		STR * buf = StrBBuffer<STR>::Get(&nChr);
		for(long i=0;i<nChr;i++)
		{
			chs = __HashKey(chs,SupportFn.__get_element_checksum_value(buf[i]));
		};
		return chs;
	};

	private:
	unsigned long __HashKey(unsigned long key, unsigned long nHash)
	{
		return ((nHash<<5) + nHash + key);
	};

	//*****************************************************************************
	//*****************************************************************************
	public:
	void ltos(const long n, int radix = -1)
	{
		if(radix==-1)
			radix = GetNumberConvertBase();
		_strinit_n(n,radix);
	};

	//*****************************************************************************
	//*****************************************************************************
	void ultos(const unsigned long n, int radix = -1)
	{
		if(radix==-1)
			radix = GetNumberConvertBase();
		_strinit_n(n,radix);
	};

	//*****************************************************************************
	//*****************************************************************************
	long& Value(long& n, int radix = -1)
	{
		if(radix==-1)
			radix = GetNumberConvertBase();

		const STR * buf = c_str();
		n = SupportFn.__strtol(buf,NULL,radix);
		return n;
	};

	unsigned long& Value(unsigned long& n, int radix = -1)
	{
		if(radix==-1)
			radix = GetNumberConvertBase();

		const STR * buf = c_str();
		n = SupportFn.__strtoul(buf,NULL,radix);
		return n;
	};


	//*****************************************************************************
	//*****************************************************************************
	int GetNumberConvertBase(void)
	{
		// Bit-Flags sind für octal + hex, wenn keins gesetzt ist, ist's dezimal
		if(m_Flag&STRTTFLAG_OCT)
			return 8;
		else if(m_Flag&STRTTFLAG_HEX)
			return 16;
		else
			return 10;

		return 10;
	};

	//*****************************************************************************
	//*****************************************************************************
	void SetNumberConvertBase(int base)
	{
		// Löschen der Flags
		m_Flag &= ~(STRTTFLAG_HEX|STRTTFLAG_OCT);
		
		switch(base)
		{
		case 8:
			m_Flag |= STRTTFLAG_OCT;
			break;

		case 16:
			m_Flag |= STRTTFLAG_HEX;
			break;

		}
	}

	//*****************************************************************************
	// Wenn CaseSens TRUE ist, werden Vergleiche case-sensitiv durchgeführt.
	//*****************************************************************************
	void SetCompareCase(bool CaseSens)
	{
		if(CaseSens)
		{
			m_Flag |= STRTTFLAG_CASE;
		}
		else
		{
			m_Flag &= ~STRTTFLAG_CASE;
		}
	}


	//*****************************************************************************
	// Index: liefert den Offset im String nach AWK-Manier
	//*****************************************************************************
	long Index(const STR* strCharset)
	{
		if(strCharset==NULL)
			return -1;


		STR    *p;
		STR* buf;
		buf = (STR*)c_str();
		if(m_Flag&STRTTFLAG_CASE)
			p = SupportFn.__strstr(buf, strCharset);
		else
			p = SupportFn.__stristr(buf, strCharset);
		return long((p) ? (p - buf) : -1);

	};


	//*****************************************************************************
	//*****************************************************************************
	long FindFirstElemInSetBin(long start,STR* chr,long nPat)
	{
		return StrBBuffer<STR>::FindFirstElemInSet(start,chr,nPat);
	}

	//*****************************************************************************
	//*****************************************************************************
	long IndexBin(long start,STR* chr,long nPat)
	{

		return StrBBuffer<STR>::Index(start,chr,nPat);

		//if((chr==NULL)||(nPat<0)||(start>m_Str.Len()))
		//	return -1;

		//if(start<0)
		//	start = 0;


		//STR* lpDat = m_Str.Get();
		//long Len = m_Str.Len() - nPat;
		//for(long i = start;i<Len;i++)
		//{
		//	if(memcmp(lpDat+i,chr,nPat*sizeof(STR))==0)
		//	{
		//		return i;
		//	}
		//}

		//return -1;
	}

	//*****************************************************************************
	// Erstes Vorkommen von str finden. Die Variable pos wird initialisiert und 
	// wird bei nächsten Such-Durchläuifen werwendet. 
	//*****************************************************************************
	long& FindFirst(const STR* str,long& pos)
	{

		pos = Index(str);	// do the search

		return pos;
	};

	long& FindFirst(const StrT& str,long& pos)
	{
		const STR * buf = str.c_str();
		return FindFirst(buf,pos);
	}

	//*****************************************************************************
	// Find the next occurrence of substring 'findStr' in the string
	// pos ist die von FindFirst initialisierte Variable zum Zwischenspeichern
	// des aktuellen Ergebnisses.
	//*****************************************************************************
	long& FindNext(const STR* str,long& pos)
	{
					
		long leng = StrBBuffer<STR>::Len();
		const STR * buf = c_str();

		if(pos>=leng)
			pos = -1;
		else
		{
			// Wahrscheinlich ist pos noch ein Hinweis auf einen
			// gefundenen str. Wenn ja, weiterschalten.
			if(__compare_int(buf+pos,str)>=0)
				pos += SupportFn.__strlen(str);

			STR* lpStr = SupportFn.__strstr(buf+pos,str);
			if(lpStr)
				pos = (lpStr-buf);
			else
				pos = -1;
		};

		return pos;
	};


	long& FindNext( StrT& str,long& pos)
	{
		const STR * buf = str.c_str();
		return FindNext(buf,pos);

	}









	

};
#pragma pack(pop)



typedef StrT<wchar_t, WIDECHAR_SUPPORT<wchar_t>, C_MemoryAllocatorRtl >	StrW;
typedef StrT<char, CHAR_SUPPORT<char>, C_MemoryAllocatorRtl >	StrC;

typedef StrT<wchar_t, WIDECHAR_SUPPORT<wchar_t>, C_MemoryAllocatorHeap >	StrHW;
typedef StrT<char, CHAR_SUPPORT<char>, C_MemoryAllocatorHeap >	StrHC;

#ifdef _UNICODE
#define _tStr	StrW
#else
#define _tStr	StrC
#endif

typedef LinkedListEntryT<StrW> StrWLlEntry;
typedef LinkedListEntryT<StrC> StrCLlEntry;

template<typename STRINGOBJ, typename LLIST, typename STR>
class StrTArray
{
	private:
	LLIST*	lpLinkedList;

	public:
		StrTArray(){lpLinkedList = NULL;}

		~StrTArray()
		{
			if(lpLinkedList!=NULL) delete lpLinkedList;
			lpLinkedList = NULL;
		}

		STRINGOBJ& push_back(STRINGOBJ& ob)
		{
//			STRINGOBJ* lpNew = new STRINGOBJ(ob);
			if(lpLinkedList==NULL)
				lpLinkedList = new LLIST(&ob);
			else
				lpLinkedList->Add(&ob);
//			return *lpNew;
			return ob;
		}

		bool GetString(STRINGOBJ& str,unsigned long idx)
		{
			STRINGOBJ* lpRetVal = GetString(idx);
			if(lpRetVal)
			{
				str.Set(*lpRetVal);
				return true;
			}
//		str = *lpRetVal;
/*			if(lpLinkedList)
			{
				 lpLinkedList->Get(idx,str);
			}*/
			return false;
		}

		STRINGOBJ* GetString(unsigned long idx)
		{
			if(lpLinkedList)
				return lpLinkedList->GetPtr(idx);
			return NULL;
		}

		void Clear(void)
		{
			if(lpLinkedList)
				delete lpLinkedList;
			lpLinkedList = NULL;
		}


		// Hackt einen String dort in Stücke, wo die komplette Sequenz SplitStr 
		// gefunden wird. 
		unsigned long SplitStr(STRINGOBJ& str,STR* SplitStr,long nChr)
		{
			unsigned long SegCount = 0;

			Clear();

//			unsigned long i,j;
			long LastPos = 0;
			long pos = 0;
			
			STRINGOBJ seg;			
			
			while((pos = str.IndexBin(pos,SplitStr,nChr))!=-1)
			{
				// Segment liegt zwischen LastPos und Pos
				seg = str.Substr(LastPos,pos-LastPos);
				push_back(seg);
				SegCount++;
				pos += nChr;
				LastPos = pos;
				
			}
/*			
			while( (pos = str.FindNext(charset,pos))!=-1)
			{
				// Segment liegt zwischen LastPos und Pos
				seg = str.Substr(LastPos,pos-LastPos);
				push_back(seg);
				SegCount++;
				LastPos = pos + charset.length();
			};
*/
			seg = str.Substr(LastPos,-1);
			if(seg.length()!=0)
			{
				push_back(seg);
				SegCount++;
			};

			return SegCount;

		}


		// Trennt einen String in Segmente bei jedem Vorkommen eines
		// Elementes von charset
		unsigned long Split(STRINGOBJ& str,STR* charset,long nChr)
		{
			unsigned long SegCount = 0;

			Clear();

//			unsigned long i,j;
			long LastPos = 0;
			long pos = 0;
			
			STRINGOBJ seg;			
			
//			while((pos = str.IndexBin(pos,charset,nChr))!=-1)
			while((pos=str.FindFirstElemInSetBin(pos,charset,nChr))!=-1)
			{
				// Segment liegt zwischen LastPos und Pos
				seg = str.Substr(LastPos,pos-LastPos);
				push_back(seg);
				SegCount++;
				pos++;				// Überspringen des Separators
				LastPos = pos;
				
			}
/*			
			while( (pos = str.FindNext(charset,pos))!=-1)
			{
				// Segment liegt zwischen LastPos und Pos
				seg = str.Substr(LastPos,pos-LastPos);
				push_back(seg);
				SegCount++;
				LastPos = pos + charset.length();
			};
*/
			seg = str.Substr(LastPos,-1);
			if(seg.length()!=0)
			{
				push_back(seg);
				SegCount++;
			};

			return SegCount;
		}

};


typedef StrTArray<StrW,StrWLlEntry, wchar_t> StrWArray;
typedef StrTArray<StrC,StrCLlEntry, char> StrCArray;


//*****************************************************************************
// Prototypen für globale Funktionen 
//*****************************************************************************
int StrWPrintf(StrW& buffer,long siz,const wchar_t* format, ...);
int StrCPrintf(StrC& buffer,long siz,const char* format, ...);

unsigned long StrCGetPrivateProfileString(StrC AppName,StrC KeyName,StrC Default,StrC& ReturnedString,long MaxSize,StrC FileName);


#endif // ____INC___STRT_H____CNI_____
