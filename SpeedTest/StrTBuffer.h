#pragma once

#pragma pack(push, 1)
template<typename STR, typename ALLOC>
class StrTBuffer
{
	private:
	//*************************************************************************
	//*************************************************************************
	ALLOC			m_Alloc;
	STR*			m_Data;
	long			m_Len;
	long			m_Size;

	// Read-Only-Flag: ist 1, wenn der Puffer auf einen konstanten externen
	// STR verweist
//	unsigned long m_Ro : 1;


	//*************************************************************************
	// Zum Erreichen der Fähigkeit, in unterschiedlichen Runtime-Umgebungen 
	// (DLL's) Allozierungen vornehmen zu können, wird hier ein eigener 
	// Allocator benutzt: über einen Zeiger wird ein globales Objekt angesprochen,
	// welches in der Umgebung des Moduls liegt, welches das Objekt erzeugte.
	//*************************************************************************
	STR* MyStrNew(unsigned long nElem)
	{
//		return (STR*)malloc(nElem*sizeof(STR));
		return (STR*) m_Alloc.Alloc(nElem*sizeof(STR));
	};

	void MyStrDelete(STR* lpData)
	{
//		free(lpData);
		m_Alloc.Free((BYTE*)lpData);
	};

	STR* MyStrRealloc(STR* lpData,unsigned long nElem)
	{
//		return (STR*) realloc(lpData,nElem*sizeof(STR));
		return (STR*)m_Alloc.ReAlloc((BYTE*)lpData,nElem*sizeof(STR));
	};

	//*************************************************************************
	// Check if the buffer is able to hold nElem elements.
	// If the buffer is a constant one it mutates generally to dynamic allocated
	// memory because all functions that uses _chksize want to modify the data.
	// if bDup = TRUE the data is duplicated, if false the string is empty 
	// after reallocation (to avoid unnessecary memory copys)
	//*************************************************************************
	long _chksize (long nElem, bool bDup = true)
	{

		if((nElem==m_Len)&&(m_Size>=nElem))
			return 0;

//		if(nElem<STDLEN)
//			nElem = STDLEN;

//		if((m_Size==-1)||(m_Len!=nElem))
		if((m_Size==-1)||(m_Len<(nElem+1)))
		{
			//if(nElem<STDLEN)
			//	nElem = STDLEN;
			

			if(m_Size!=-1)	// Kein konstantes Objekt!
			{
				if(m_Data!=NULL)
					m_Data = MyStrRealloc(m_Data,nElem+1);
				else
				{
					if(nElem<STDLEN)
						nElem = STDLEN;

					m_Data = MyStrNew(nElem+1);
				}
			}
			else
			{
				// Konstantes Objekt wird kopiert
				STR* lpOld = m_Data;
				m_Data = MyStrNew(nElem+1);
				if((bDup)&&(lpOld))
					memcpy(m_Data,lpOld,m_Len*sizeof(STR));
	
			};

/*
			STR* lpOld = m_Data;
			
//			m_Data = new STR[nElem+1];
			m_Data = MyStrNew(nElem+1);

			if((bDup)&&(lpOld))
				memcpy(m_Data,lpOld,m_Len*sizeof(STR));

			// Delete old if it is dynamic allocated
			if((m_Size!=-1)&&(lpOld!=NULL))
				MyStrDelete(lpOld);
//				delete lpOld;
*/
			// Settig the size mark it as dynamically allocated
			m_Size = nElem;

			return 1;
			
		};

		return 0;
	}

	void InitPrivateData(void)
	{
		m_Data = NULL;
//		m_Ro = 0;
		m_Len = 0;
		m_Size = -1;
	}

	public:
	//*************************************************************************
	// Konstruktoren
	//*************************************************************************
	StrTBuffer(void){InitPrivateData();};

	StrTBuffer(const STR* s,long len)
	{
		InitPrivateData();
		SetC(s,len);
	}

	StrTBuffer(const STR c)
	{
		InitPrivateData();
		SetC(&c,1);
	}

	//*************************************************************************
	// Destruktor
	//*************************************************************************
	~StrTBuffer(void)
	{
		if((m_Size!=-1)&&(m_Data!=NULL))
			MyStrDelete(m_Data);
//			delete[] m_Data;
		InitPrivateData();
	}

	//*************************************************************************
	// Tauscht den Inhalt zweier Puffer ohne Kopieren aus.
	//*************************************************************************
	StrTBuffer & Swap(StrTBuffer & s)
	{
		STR*			td = s.m_Data;
		long			tl = s.m_Len;
		long			ts = s.m_Size;
		
		s.m_Data = m_Data;
		s.m_Len = m_Len;
		s.m_Size = m_Size;
		m_Data = td;
		m_Len = tl;
		m_Size = ts;

		return *this;
	}

	//*************************************************************************
	// Zuweisungsoperator
	//*************************************************************************
	StrTBuffer & operator= (StrTBuffer const & s)
	{
		if(s.m_Size==-1)
		{
			// Konstante übernehmen
			m_Size = -1;
			m_Len = s.m_Len;
			m_Data = s.m_Data;
		}
		else
		{
			if(s.m_Data)
				Set(s.m_Data,s.m_Len);
		}

		return *this;
	}
	
	//*************************************************************************
	// Additionsoperatoren
	//*************************************************************************
	StrTBuffer& operator+=(const StrTBuffer& s)
	{
		if(s.m_Data)
			Add(s.m_Data,s.m_Len);
		return *this;
	}

/*
	//*************************************************************************
	// Das funktioniert nicht, bringt Debug-Assert!!!!
	//*************************************************************************
	StrTBuffer& operator+(const StrTBuffer& s)
	{
		if(s.m_Data)
			Add(s.m_Data,s.m_Len);
		
		return *this;

	}
*/


	//*************************************************************************
	// Clear: löscht den Inhalt bzw. initialisiert den Puffer mit der Defaultlänge
	//*************************************************************************
	void Clear(void)
	{
		_chksize(0,false);
		m_Len = 0;
	}
	

	//*************************************************************************
	// Set-Funktionen: setzen die Daten des Puffers
	// Die SetC Funktionen setzen einen readonly-Verweis auf einen externen Puffer,
	// die Set-Funktionen belegen dynamischen Speicher.
	//*************************************************************************
	STR* SetC(const STR* s, long len)
	{

		if((s==NULL)||(len<0))
			return NULL;

	// Konstantes S: setzt den Zeiger und setzt Size auf -1
		SetC(*s,len);

		return m_Data;
	}

	STR* Set(STR* s,long len)
	{
		if((s==NULL)||(len<0))
			return NULL;

		Set(*s,len);

		return m_Data;
	};

	STR& SetC(const STR& s,long len)
	{
		// Konstantes S: setzt den Zeiger und setzt Size auf -1

		// Wenn der Puffer bisher keinen konstanten Inhalt hatte,
		// lösche ihn.
		if((m_Size!=-1)&&(m_Data!=NULL))
			MyStrDelete(m_Data);
//			delete m_Data;

		// Setzen des Strings
		m_Data = (STR*)&s;
		m_Size = -1;
		m_Len = len;

		return *m_Data;
	};

	STR& Set(STR& s,long len)
	{
		// s ist nicht konstant: String muß dynamisch angelegt werden.
		// _chksize sorgt für genügend dynamischen Speicher
		_chksize(len,false);

		if(m_Data)
		{
			memcpy(m_Data,&s,len*sizeof(STR));
			m_Len = len;
		};


		return *m_Data;
	};


	//*************************************************************************
	// Add-Funktionen fügen ein oder mehrere Elemente an den Puffer an.
	//*************************************************************************
	long Add(const STR* s,long len)
	{
		if((s!=NULL)&&(len>=0))
		{
			_chksize(m_Len+len);

			memcpy(m_Data+m_Len,s,len*sizeof(STR));
			m_Len += len;
		}
		
		return m_Len;
	}

/*
	long Add(const STR& s,long len)
	{
		if(len<0)
			return m_Len;
		
		_chksize(m_Len+len);

		memcpy(m_Data+m_Len,&s,len*sizeof(STR));
		m_Len += len;

		return m_Len;
	}
*/
	long AddElem(const STR& c)
	{
		_chksize(m_Len+1);
		m_Data[m_Len++] = c;
		return m_Len;

	}

	//*************************************************************************
	// Insert-Funktion
	//*************************************************************************
	long Insert (long pos, STR const * s, long len)
	{
		if (len)
		{
			long leng = m_Len;
			if (pos < 0 || pos > leng)
				pos = leng;
			_chksize (long(leng + len + 1));
			STR * buf = m_Data;
			if (pos < leng)
				memmove(buf + pos + len, buf + pos,(leng - pos)*sizeof(STR));
			memcpy (buf + pos, s, len * sizeof(STR));
			m_Len += len;
		}

		return m_Len;
	}


	//*************************************************************************
	// removes a part of the buffer
	//*************************************************************************
	long remove (long pos, long len)
	{
		if (pos >= 0 && pos < m_Len)
		{
			long leng = m_Len;
			if (len < 0 || (pos + len) > leng)
				len = long(leng - pos);
			if (len)
			{
				_chksize (0);
				memcpy (m_Data + pos, m_Data + pos + len, (leng - (pos + len))*sizeof(STR));
				m_Len -= len;
			}
		}
		return m_Len;
		
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
			return m_Len;

		if (pos >= 0)
		{
			long leng = m_Len;
			if (clen < 0 || (pos + clen) > leng)
				clen = long(leng - pos);
			if (pos > leng)
				pos = leng;
			_chksize (long(leng - clen + len + 1));
			if (clen != len && clen)
				memmove (m_Data + pos + len, m_Data + pos + clen,
						 (leng - (pos + clen - len))*sizeof(STR));
			if (len)
				memcpy (m_Data + pos, s, len * sizeof(STR));
			m_Len += long(len - clen);
		}
		return m_Len;
	}

	//*****************************************************************************
	//*****************************************************************************
	StrTBuffer & left (long len, STR padch)
	{
		if (len < 0)
			return right (long(-len), padch);
		long leng = m_Len;
		if (len != leng)
		{
			
			_chksize (long(len ));
			m_Len = len;
			if (len > leng)
				BlockSet(leng,padch,len - leng);
		}
		return *this;
	}

	//*****************************************************************************
	//*****************************************************************************
	StrTBuffer & right (long len, STR padch)
	{
		if (len < 0)
			return left(-1, padch);
		long leng = m_Len;
		if (len != leng)
		{
			_chksize (long(len ));
			if (len > leng)
			{

				memmove (m_Data + len - leng, m_Data, leng*sizeof(STR));
				BlockSet(0,padch,len-leng);
			}
			else
			{
				// String muß schrumpfen, also herunterkopieren der Daten
				memmove(m_Data,m_Data+(leng-len),len*sizeof(STR));
												
			};
			m_Len = len;
		}
		return *this;
	}

	//*****************************************************************************
	//*****************************************************************************
	StrTBuffer & mid (long pos, long len, STR padch)
	{
		if (pos <= 0)
			return left(len, padch);
		long leng = m_Len;
		if (pos > leng)
			pos = leng;
		
		_chksize (long(len ));
		if (leng < len)         // Are we padding?
		{
			long nlen = long((len - (leng - pos)) / 2);
			if (nlen > 0)
			{
				memmove (m_Data, m_Data + pos, (leng - pos)*sizeof(STR));
				BlockSet((leng - pos),padch,nlen);
				m_Len -= long(pos - nlen);
			}
		}
		else if(leng > len)
		{
			// String muß schrumpfen: Block aus der Mitte heraus nach
			// links kopieren und Länge anpassen.
			memmove(m_Data,m_Data+pos,len*sizeof(STR));
			m_Len = len;
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
		if(m_Len<(long)(DestOffs+cnt))
		{
			m_Len = DestOffs+cnt;
		};

		_chksize(m_Len);

		if(sizeof(STR)==1)
		{
			// Bescheuerte Konstruktion, weil STR nur unter der Bedingung
			// direkt als Parameter von memset benutzt werden kann, wenn 
			// es sich um chars oder WCHARS handelt. Unter anderen Bedingugnen
			// läuft dieser Code sowieso nicht. (Wie könnte man dies zur Compilezeit
			// festlegen??? Wäre besser für die Laufzeit!
			int* lpInt = (int*)&val;
			memset(m_Data+DestOffs,*lpInt,cnt);
		}
		else if(sizeof(STR)==2)
		{
			int* lpInt = (int*)&val;
			wmemset((wchar_t*)m_Data+DestOffs,*lpInt,cnt);
		}
		else
		{
			for(long i = 0;i<cnt;i++)
				m_Data[i+DestOffs] = val;
		};
	
		return m_Data;
	}


/*
	STR* BlockMove(long DestOffs,long SrcOffs,long cnt)
	{
		if((DestOffs>m_Len)||(ScrOffs>m_Len))
			return m_Data;

		_chksize(m_Len);

		if((DestOffs+cnt)>m_Len)
			cnt = m_Len - DestOffs;
		if((SrcOffs+cnt)>m_Len)
			cnt = m_Len - SrcOffs;

		memmove(m_Data+DestOffs,m_Data+SrcOffs,cnt*sizeof(STR));
	}
*/


	//*************************************************************************
	// Funktionen zum Ermitteln der Daten.
	//*************************************************************************
	// Liefert nur den Zeiger
	STR* Get(void)
	{
		return m_Data;
	}



	// Liefert den Zeiger und die Anzahl von Elementen ohne Terminierung
	STR* Get(long* nElem)
	{
		if(nElem)
			*nElem = m_Len;
		return m_Data;
	}


	// Liefert einen terminierten String. Ist der String ein Verweis auf 
	// einen konstanten Puffer, so mutiert dieser automatisch zu einem dynamischen
	// Puffer
	STR* Get(const STR& TermElem,long* nElem)
	{
		// Mutiert den Puffer, falls er const ist
		_chksize(m_Len);

		m_Data[m_Len] = TermElem;

		if(nElem)
			*nElem = m_Len;

		return m_Data;
	}
	

	// Reserviert einen Puffer von einer bestimmten Länge
	void ReserveBuffer(long ReqSize)
	{
		if(ReqSize>=0)
			_chksize(ReqSize);
	}

	void ReserveBuffer(long ReqSize,STR& FillElem)
	{
		if(m_Size<ReqSize)
		{
			BlockSet(m_Size,FillElem,ReqSize-m_Size);
		};
	}


	//*************************************************************************
	// Schneidet den String ab
	//*************************************************************************
	long TruncateLeft(long nElem)
	{
		if(nElem>=m_Len)
			Clear();
		else
		{
			_chksize(m_Len);
			memmove(m_Data,m_Data+nElem,(m_Len-nElem)*sizeof(STR));
			m_Len -= nElem;
		};

		return m_Len;
	}


	long TruncateRight(long nElem)
	{
		if(nElem>=m_Len)
			Clear();
		else
		{
			m_Len -= nElem;
			_chksize(m_Len-nElem);
		}
		return m_Len;
	}

	long TruncateChr(STR& chr,bool left,bool right)
	{
		if(m_Len==0) return m_Len;

		unsigned long start,end;
		start = 0; end = 0;
		if(right)
		{
			for(long i=m_Len-1;i>=0;i--)
			{
				if(m_Data[i]!=chr)
					break;
				else
					end++;
			}
		}

		if(left)
		{
			for(start=0;start<end;start++)
			{
				if(m_Data[start]!=chr)
					break;
			}
		}
		
		if(end)
			TruncateRight(end);
		if(start)
			TruncateLeft(start);

		return m_Len;
			
	}

	//*************************************************************************
	// Liefert Länge und Puffergröße
	//*************************************************************************
	long Len(void){return m_Len;};

	long Size(void){return m_Size;};

	//*************************************************************************
	// Zum Setzen der Länge, wenn der Puffer von außen modifiziert wurde 
	// (!!! Vorsicht - Size() beachten bzw. setzen !!!)
	//*************************************************************************
	void SetLen(long len)
	{
		if(len<0)
			return;
		if(len>m_Size)
			len = m_Size;
		m_Len = len;
	};

};
#pragma pack(pop)





typedef StrTBuffer<wchar_t, C_MemoryAllocatorRtl>		StrTBufferW;
typedef StrTBuffer<char, C_MemoryAllocatorRtl>			StrTBufferC;
