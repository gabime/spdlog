#ifndef ____INC___V_MEMORYALLOCATOR_H___CNI____
#define ____INC___V_MEMORYALLOCATOR_H___CNI____

#include <stdio.h>

// #define ____VMEMALLOC_DEBUG

/*
class C_GlobalMemorySource
{
private:
	HANDLE hGlobalHeap;

public:
	C_GlobalMemorySource();
	BYTE* Allocate(SIZE_T dwSize);
	void Free(BYTE* lpMem);
};


extern C_GlobalMemorySource MaterieQuelle;
*/

class C_MemoryAllocatorHeap
{
private:
	HANDLE hGlobalHeap;

public:
	C_MemoryAllocatorHeap()
	{
		hGlobalHeap = GetProcessHeap();
	};


	BYTE* Alloc(SIZE_T size)
	{
#ifdef ____VMEMALLOC_DEBUG
		BYTE* lpRetVal = (BYTE*)HeapAlloc(hGlobalHeap,HEAP_ZERO_MEMORY,size);
		char Buffer[151];
		_snprintf(Buffer,150,"Alloc (H) %x Byte at %x,this = %x\r\n",size,lpRetVal,this);
		OutputDebugString(Buffer);
		return lpRetVal;
#else
		return (BYTE*)HeapAlloc(hGlobalHeap,HEAP_ZERO_MEMORY,size);
#endif
	};
	
	BYTE* ReAlloc(BYTE* lpMem,SIZE_T size)
	{
#ifdef ____VMEMALLOC_DEBUG
		BYTE* lpRetVal = (BYTE*)HeapReAlloc(hGlobalHeap,HEAP_ZERO_MEMORY,lpMem,size);
		char Buffer[151];
		_snprintf(Buffer,150,"ReAlloc (H) %x Byte at %x,this = %x\r\n",size,lpRetVal,this);
		OutputDebugString(Buffer);
		return lpRetVal;
#else
		return (BYTE*)HeapReAlloc(hGlobalHeap,HEAP_ZERO_MEMORY,lpMem,size);
#endif
	};

	void Free(BYTE* lpMem)
	{
	#ifdef ____VMEMALLOC_DEBUG
		char Buffer[151];
		_snprintf(Buffer,150,"Free (H) block %x,this = %x\r\n",lpMem,this);
		OutputDebugString(Buffer);
	#endif
		if(lpMem)
			HeapFree(hGlobalHeap,0,lpMem);
	};
};

class C_GlobalMemAlloc
{
public:
	void* operator new(size_t size);
	void operator delete(void *ptr,size_t size);
};

class C_MemoryAllocatorRtl
{
private:

public:

	BYTE* Alloc(SIZE_T size)
	{
#ifdef ____VMEMALLOC_DEBUG
		BYTE* lpRetVal = (BYTE*)malloc(size);
		char Buffer[151];
		_snprintf(Buffer,150,"Alloc (R) %x Byte at %x,this = %x\r\n",size,lpRetVal,this);
		OutputDebugString(Buffer);
		return lpRetVal;
#else
		return (BYTE*)(BYTE*)malloc(size);;
#endif
	};
	
	BYTE* ReAlloc(BYTE* lpMem,SIZE_T size)
	{
#ifdef ____VMEMALLOC_DEBUG
		BYTE* lpRetVal = (BYTE*)realloc(lpMem,size);
		char Buffer[151];
		_snprintf(Buffer,150,"ReAlloc (R) %x Byte at %x,this = %x\r\n",size,lpRetVal,this);
		OutputDebugString(Buffer);
		return lpRetVal;
#else
		return (BYTE*)realloc(lpMem,size);
#endif
	};

	void Free(BYTE* lpMem)
	{
	#ifdef ____VMEMALLOC_DEBUG
		char Buffer[151];
		_snprintf(Buffer,150,"Free (R) block %x,this = %x\r\n",lpMem,this);
		OutputDebugString(Buffer);
	#endif
		if(lpMem)
			free(lpMem);
	};
};


#endif ____INC___V_MEMORYALLOCATOR_H___CNI____
