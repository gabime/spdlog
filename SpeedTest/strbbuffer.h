#pragma once
#include <string>

#define STRBBUFFER_NO_MYPTR

template<typename STR>
class StrBBuffer : public std::basic_string<STR>
{
public:
	StrBBuffer(){};
	StrBBuffer(const STR* s,long len)
	{
		Set(s,len);
	}

	StrBBuffer(const STR c)
	{
		Set(&c,1);
	}

	~StrBBuffer(void){}

	long FindFirstElemInSet(long start, STR* set,long nPat)
	{
	
		if((set==NULL)||(nPat<0)||(start>long(std::basic_string<STR>::length())))
			return -1;

		long RetVal = -1;

		size_type ActPos = npos;
		
		for(long i=0;i<nPat;i++)
		{
			ActPos=std::basic_string<STR>::find(set[i],start);
			if(ActPos!=npos)
			{
				if((RetVal==-1)||(long(ActPos)<RetVal))
					RetVal = (long)ActPos;
			}
		}


		return RetVal;
	}

	long Index(long start,STR* chr,long nPat)
	{
		if((chr==NULL)||(nPat<0)||(start>std::basic_string<STR>::length()))
			return -1;

		if(start<0)
			start = 0;

		size_type RetVal = std::basic_string<STR>::find(chr,start,nPat);
		if(RetVal==npos)
			return -1;
		return RetVal;

	}


	STR* Set(const STR* s,long len)
	{
		if((s==NULL)||(len<0))
			return NULL;

		std::basic_string<STR>::assign(s,len);

		#ifdef STRBBUFFER_NO_MYPTR
		return (STR*) std::basic_string<STR>::data();
		#else
		return std::basic_string<STR>::_Myptr();
		#endif
	};

	STR& Set(STR& s,long len)
	{
		std::basic_string<STR>::assign(&s,len);

		#ifdef STRBBUFFER_NO_MYPTR
		STR* lps = (STR*) std::basic_string<STR>::data();
		return lps[0];
		#else
		return *std::basic_string<STR>::_Myptr();
		#endif
	};

	// Zuweisung von nChr Elementen s an den Puffer
	STR& Set(long nChr,STR& s)
	{
		std::basic_string<STR>::assign(nChr,s);

		#ifdef STRBBUFFER_NO_MYPTR
		const STR* lps = std::basic_string<STR>::data();
		return (STR&)lps[0];
		#else
		return *std::basic_string<STR>::_Myptr();
		#endif
	};


	// Reserviert einen Puffer von einer bestimmten Länge

	void ReserveBuffer(long ReqSize,STR& FillElem)
	{
//		std::basic_string<STR>::resize(
		if(long(std::basic_string<STR>::length())<ReqSize)
		{
			std::basic_string<STR>::resize(ReqSize,FillElem);
//			BlockSet(std::basic_string<STR>::length(),FillElem,ReqSize-std::basic_string<STR>::length());
		}
	}

	void SetLen(long len)
	{
		std::basic_string<STR>::resize(len);
	}

	
	void StripTrailing(const STR& chr)
	{
		long nPos = -1;
		nPos = (long)std::basic_string<STR>::find_last_not_of(chr,nPos);
		if(nPos>0)
			std::basic_string<STR>::resize(nPos+1);
	}

	long Add(const STR* s,long len, const STR& TermElem)
	{
		StripTrailing(TermElem);
		return Add(s,len);
	}

	long Add(const STR* s,long len)
	{
		if((s!=NULL)&&(len>=0))
		{
			std::basic_string<STR>::append(s,len);
		}
		
		return (long)std::basic_string<STR>::length();
	}


	long AddElem(const STR& c,const STR& TermElem)
	{
		StripTrailing(TermElem);
		return AddElem(c);
	}

	long AddElem(const STR& c)
	{
		std::basic_string<STR>::operator +=(c);
		return (long)std::basic_string<STR>::length();
	}

	long Insert (long pos, STR const * s, long len)
	{
		if (len)
		{
			long leng = (long)std::basic_string<STR>::length();
			if (pos < 0 || pos > leng)
				pos = leng;
			
			std::basic_string<STR>::insert(pos,s,len);

		}

		return (long) std::basic_string<STR>::length();
	}


	long remove (long pos, long len)
	{
		if (pos >= 0 && pos < long(std::basic_string<STR>::length()))
		{
			if(len==-1)
				len = long(std::basic_string<STR>::length()) - pos;
			std::basic_string<STR>::erase(pos,len);
		}
		return (long)std::basic_string<STR>::length();
		
	}



	//*************************************************************************
	// Replace a part or the whole buffer.
	// If the buffer is not big enough, it is grown to the size that fits 
	// for the resulting string.
	// pos = Position, ab welcher ersetzt werden soll
	// s = Source-Puffer
	// clen = Länge des zu ersetzenden Abschnittes im Originalstring
	// len = Länge des einzufügenden Puffers (s)
	// InitChr = character to initialize the growing sequency
	//*************************************************************************
	long ReplaceAndGrow(long pos, STR const * s, long clen, long len,STR& InitChr)
	{
		if((s==NULL)||(len<0)||(pos<0)||(clen<0))
			return std::basic_string<STR>::length();

		// Wachsen angesagt?
		long leng = std::basic_string<STR>::length();
		if((pos+clen)>leng)
			ReserveBuffer(pos+clen,InitChr);
		
		return replace(pos,s,clen,len);
	}

	//*************************************************************************
	// Replace a part of the buffer
	// pos = Position, ab welcher ersetzt werden soll
	// s = Source-Puffer
	// clen = Länge des zu ersetzenden Abschnittes im Originalstring
	// len = Länge des einzufügenden Puffers (s)
	//*************************************************************************
	long replace (long pos, STR const * s, long clen, long len)
	{
		if((s==NULL)||(len<0)||(pos<0))
			return (long)std::basic_string<STR>::length();

		long leng = long(std::basic_string<STR>::length());

		if ((pos >= 0)&&(pos < leng))
		{
			// Wenn ungültig, gehe davon aus, daß ganzer String
			// zu ersetzen ist.
			if (clen < 0)
				clen = (long)leng - pos;

			if((pos+clen)>leng)
				clen = long(leng-pos);

			//if (clen < 0 || (pos + clen) > leng)
			//	clen = long(leng - pos);
			//if (pos > leng)
			//	pos = leng;

			std::basic_string<STR>::replace(pos,len,s,clen);

		}

		return long(std::basic_string<STR>::length());

	}

	StrBBuffer & left (long len, STR padch)
	{
		if (len < 0)
			return right (long(-len), padch);
		long leng = std::basic_string<STR>::length();

		if (len != leng)
		{
			if (len > leng)
				BlockSet(leng,padch,len - leng);
			else
				std::basic_string<STR>::erase(len);
		}
		return *this;
	}

	StrBBuffer & right (long len, STR padch)
	{
		if (len < 0)
			return left(-1, padch);

		long leng = std::basic_string<STR>::length();
		if (len != leng)
		{

			if (len > leng)
			{
				#ifdef STRBBUFFER_NO_MYPTR
				std::basic_string<STR> _buf =  std::basic_string<STR>::data();
				#else
				std::basic_string<STR> _buf = this->_Myptr();
				#endif

				STR* _ptr = (STR*)_buf.c_str();
				std::basic_string<STR>::erase();
				BlockSet(0,padch,len-leng);
				std::basic_string<STR>::append(_ptr);
			}
			else
			{
				// String muß schrumpfen, also herunterkopieren der Daten

				#ifdef STRBBUFFER_NO_MYPTR
				std::basic_string<STR> _buf =  std::basic_string<STR>::data();
				#else
				std::basic_string<STR> _buf = this->_Myptr();
				#endif
				STR* _ptr = (STR*)_buf.c_str();
				std::basic_string<STR>::assign(_ptr+(leng-len),len);
												
			};
		}
		return *this;
	}

	//*****************************************************************************
	//*****************************************************************************
	StrBBuffer & mid (long pos, long len, STR padch)
	{
		if (pos <= 0)
			return left(len, padch);

		long leng = std::basic_string<STR>::length();
		if (pos > leng)
			pos = leng;
		
		if (leng < len)         // Are we padding?
		{
			long nlen = long((len - (leng - pos)) / 2);
			if (nlen > 0)
			{
				#ifdef STRBBUFFER_NO_MYPTR
				std::basic_string<STR> _buf =  std::basic_string<STR>::data();
				#else
				std::basic_string<STR> _buf = this->_Myptr();
				#endif
				STR* _ptr = (STR*)_buf.c_str();

				std::basic_string<STR>::assign(_ptr + pos,leng - pos);
				BlockSet((leng - pos),padch,nlen);
			}
		}
		else if(leng > len)
		{
			// String muß schrumpfen: Block aus der Mitte heraus nach
			// links kopieren und Länge anpassen.

			#ifdef STRBBUFFER_NO_MYPTR
			std::basic_string<STR> _buf =  std::basic_string<STR>::data();
			#else
			std::basic_string<STR> _buf = this->_Myptr();
			#endif
			STR* _ptr = (STR*)_buf.c_str();
			std::basic_string<STR>::assign(_ptr + (pos-1),len);
		};
		return *this;
	}


	//*************************************************************************
	//*************************************************************************
	STR* BlockSet(long DestOffs,STR& val,long cnt)
	{
//		if(DestOffs>m_Len)
//			return m_Data;

		// Wachsen angesagt?
		if(long(std::basic_string<STR>::length())<(DestOffs+cnt))
		{
			LONG lGrow = (DestOffs+cnt) - long(std::basic_string<STR>::length());
			std::basic_string<STR>::append(lGrow,val);
		};

	
		#ifdef STRBBUFFER_NO_MYPTR
		return (STR*) std::basic_string<STR>::data();
		#else
		return std::basic_string<STR>::_Myptr();
		#endif
	}

	long Len(void){return (long)std::basic_string<STR>::length();};

	void Clear(void)
	{
		std::basic_string<STR>::clear();
	}

		// Liefert nur den Zeiger
	STR* Get(void)
	{
		#ifdef STRBBUFFER_NO_MYPTR
		return (STR*) std::basic_string<STR>::data();
		#else
		return std::basic_string<STR>::_Myptr();
		#endif
	}


	// Liefert den Zeiger und die Anzahl von Elementen ohne Terminierung
	STR* Get(long* nElem)
	{
		if(nElem)
			*nElem = long(std::basic_string<STR>::length());
		#ifdef STRBBUFFER_NO_MYPTR
		return (STR*) std::basic_string<STR>::data();
		#else
		return std::basic_string<STR>::_Myptr();
		#endif
	}

	// Liefert einen terminierten String. 
	STR* Get(const STR& TermElem,long* nElem)
	{
		UINT  len = (UINT) std::basic_string<STR>::length();
		if(len>0)
		{
			#ifdef STRBBUFFER_NO_MYPTR
			STR* lps = (STR*) std::basic_string<STR>::data();
			#else
			STR* lps = std::basic_string<STR>::_Myptr();
			#endif
			if(lps[len-1]!=TermElem)
				std::basic_string<STR>::operator +=(TermElem);
//				std::basic_string<STR>::resize(len+1,TermElem);
		}
		else
			std::basic_string<STR>::operator +=(TermElem);

//		std::basic_string<STR>::append(TermElem,1);
//		STR* lpStr = std::basic_string<STR>::_Myptr();
//		long len = std::basic_string<STR>::

//		std::basic_string<STR>::operator +=(TermElem);
//		this += TermElem;		

		if(nElem)
			*nElem = (long)std::basic_string<STR>::length();

		#ifdef STRBBUFFER_NO_MYPTR
		return (STR*) std::basic_string<STR>::data();
		#else
		return std::basic_string<STR>::_Myptr();
		#endif
	}


	//*************************************************************************
	// Schneidet den String ab
	//*************************************************************************
	long TruncateLeft(long nElem)
	{
		if(nElem>=long(std::basic_string<STR>::length()))
			std::basic_string<STR>::clear();
		else
		{
			std::basic_string<STR>::erase(0,nElem);
		};

		return long(std::basic_string<STR>::length());
	}


	long TruncateRight(long nElem)
	{
		if(nElem>=long(std::basic_string<STR>::length()))
			std::basic_string<STR>::clear();
		else
		{
			std::basic_string<STR>::erase(std::basic_string<STR>::length()-nElem,nElem);
		}
		return (long)std::basic_string<STR>::length();
	}


	//long TruncateChr(STR& chr,bool left,bool right)
	//{
	//	return TruncateChrc(chr,left,right);
	//}


	long TruncateChr(STR chr,bool left,bool right)
	{
		if(std::basic_string<STR>::length()==0) 
			return (unsigned long)std::basic_string<STR>::length();

		unsigned long start,end;
		start = 0; end = 0;
		if(right)
		{
			unsigned long nRemove = 0;

			#ifdef STRBBUFFER_NO_MYPTR
			STR* _mp = (STR*) std::basic_string<STR>::data();
			#else
			STR* _mp = std::basic_string<STR>::_Myptr();
			#endif
			for(start = (unsigned long)std::basic_string<STR>::length(); (start>0)&&(_mp[start-1]==chr); start--, nRemove++)
			{
				DWORD dwRes = 0;
			}
			std::basic_string<STR>::erase(start,nRemove);

		}

		if(left)
		{
			end = (unsigned long)std::basic_string<STR>::find_first_not_of(chr);
			std::basic_string<STR>::erase(0,end);

		}
		
		//if(end)
		//	TruncateRight(end);
		//if(start)
		//	TruncateLeft(start);

		return (unsigned long) std::basic_string<STR>::length();
			
	}


};
