#ifndef ____INC___LLISTT_H___CNI____
#define ____INC___LLISTT_H___CNI____


#pragma pack(push, 1)

template<class LENTRY> 
class LinkedListEntryT
{
	private:
	LinkedListEntryT*	Next;
	LENTRY				Data;

	public:
	LinkedListEntryT()
	{
		Next = NULL;
		memset(&Data,0,sizeof(Data));
	}

	LinkedListEntryT(LENTRY* en)
	{
		Next = NULL;
		Data = *en;
	}

	~LinkedListEntryT()
	{
		if(Next!=NULL)
			delete Next;
		Next = NULL;
	}

	void Add(LENTRY* en)
	{
		if(!en)
			return;
		if(Next)
			Next->Add(en);
		else
			Next = new LinkedListEntryT(en);
	}

	void Add(LENTRY en)
	{
		if(!en)
			return;
		if(Next)
			Next->Add(en);
		else
			Next = new LinkedListEntryT(&en);
	}

	LENTRY* GetPtr(unsigned long index)
	{
		if(index==0)
			return &Data;

		if(Next)
			return Next->GetPtr(index-1);

		return NULL;
	};


	LENTRY& Get(unsigned long index,LENTRY& val)
	{
		if(index==0)
			return Data;

		if(Next)
			return Next->Get(index-1,val);
		return val;
	};


};
#pragma pack(pop)


#endif // ____INC___LLISTT_H___CNI____
