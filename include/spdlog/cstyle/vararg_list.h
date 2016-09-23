#pragma once
// Provides a typesafe way to transfer a variable list of arguments over a C style dll export interface. 
// Problem: there is no typesafe way to transfer argument lists with a variable count of parameters over 
// a interface like this. There are lists of variants known from OLE, but the handling is not easy.

#include "fmt\fmt.h"
#include "common_types.h"

//#include "vararg_list_item.h"


class vararg_sstring
{
public:
	const char*		string;
	size_t			size;
	vararg_sstring(const char* s, size_t _size)
	{
		string = s; size = _size;
	}
};

class vararg_ustring
{
public:
	const unsigned char*	string;
	size_t					size;
	vararg_ustring(const unsigned char* s, size_t _size)
	{
		string = s; size = _size;
	}
};

class vararg_wstring
{
public:
	const wchar_t*		string;
	size_t				size;
	vararg_wstring(const wchar_t* s, size_t _size)
	{
		string = s; size = _size;
	}
};


const unsigned long DEFAULT_VARARG_ARGUMENT_COUNT = 10;


class vararg_list
{

private:


	fmt::internal::Value::Type*		_type;
	fmt::internal::Value*			_value;
	size_t					_size;
	size_t					_allocated;

	inline void check_space_and_grow(size_t n_args)
	{
		if (_allocated < (_size + n_args))
		{
			fmt::internal::Value::Type* lptTp = _type;
			fmt::internal::Value*		lptVal = _value;
//			fmt::internal::Arg* lpTmp = _data;
			_allocated = _size + n_args;
			_type = new fmt::internal::Value::Type[_allocated];
			_value = new fmt::internal::Value[_allocated];
//			_data = new fmt::internal::Arg[_allocated];
			memcpy(_type, lptTp, _size * sizeof(fmt::internal::Value::Type));
			memcpy(_value, lptVal, _size * sizeof(fmt::internal::Value));
//			memcpy(_data, lpTmp, _size * sizeof(fmt::internal::Arg));
			delete[] lptTp;
			delete[] lptVal;
//			delete[] lpTmp;
		}
	}

public:



	vararg_list()
	{
		_allocated = DEFAULT_VARARG_ARGUMENT_COUNT;
		_type = new fmt::internal::Value::Type[_allocated];
		_value = new fmt::internal::Value[_allocated];
		_size = 0;
	}

	vararg_list(size_t DefaultSize)
	{
		_size = 0;
		_allocated = DefaultSize;
		_type = new fmt::internal::Value::Type[_allocated];
		_value = new fmt::internal::Value[_allocated];
	}


	//vararg_list(fmt::internal::Arg* lpi, unsigned long size)
	//{
	//	if ((lpi == NULL) || (size == 0))
	//	{
	//		_data = NULL; _size = 0;
	//		return;
	//	}
	//	_data = new fmt::internal::Arg[size];
	//	// Caution: just memcpy, no constructor involved!
	//	memcpy(_data, lpi, size * sizeof(fmt::internal::Arg));
	//	_size = size;
	//}

	~vararg_list()
	{
		if (_value)
			delete[] _value;
		if (_type)
			delete[] _type;
//		if (_data)
//			delete[] _data;
		_allocated = 0;
		_size = 0;
	}

	inline void add_spdlog_string(spdlog::log_char_t* data)
	{
		check_space_and_grow(1);
		#if defined(_WIN32) && defined(SPDLOG_WCHAR_LOGGING)
		_type[_size] = fmt::internal::Arg::WSTRING;
		if (data)
			_value[_size].wstring.size = wcslen(data);
		else
			_value[_size].wstring.size = 0;
		_value[_size++].wstring.value = data;
		#else
		_type[_size] = fmt::internal::Arg::CSTRING;
		if (data)
			_value[_size].string.size = strlen(data);
		else
			_value[_size].string.size = 0;
		_value[_size++].string.value = data;
		#endif
	}

	inline void add_bool(bool data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::BOOL;
		_value[_size++].uint_value = data;
	}

	inline void add_char(char data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::CHAR;
		_value[_size++].int_value = data;
	}

	inline void add_wstring(vararg_wstring& data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::WSTRING;
		_value[_size].wstring.size = data.size;
		_value[_size++].wstring.value = data.string;

	}

	inline void add_wstring(const wchar_t* data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::WSTRING;
		if (data)
			_value[_size].wstring.size = wcslen(data);
		else
			_value[_size].wstring.size = 0;
		_value[_size++].wstring.value = data;

	}

	inline void add_ustring(vararg_ustring& data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::STRING;
		_value[_size].ustring.size = data.size;
		_value[_size++].ustring.value = data.string;

	}

	inline void add_ustring(const unsigned char* data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::STRING;
		if (data)
			_value[_size].ustring.size = strlen((char*)data);
		else
			_value[_size].ustring.size = 0;
		_value[_size++].ustring.value = data;
	}

	inline void add_sstring(vararg_sstring& data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::CSTRING;
		_value[_size].sstring.size = data.size;
		_value[_size++].sstring.value = (const signed char*)data.string;
	}


	inline void add_sstring(const char* data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::CSTRING;
		if (data)
			_value[_size].sstring.size = strlen(data);
		else
			_value[_size].sstring.size = 0;
		_value[_size++].sstring.value = (signed char*)data;
	}

	inline void add_pointer(const void* data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::POINTER;
		_value[_size++].pointer = data;
	}

	inline void add_long_double(long double data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::LONG_DOUBLE;
		_value[_size++].long_double_value = data;
	}

	inline void add_double(double data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::DOUBLE;
		_value[_size++].double_value = data;
	}

	inline void add_ulonglong(unsigned long long data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::ULONG_LONG;
		_value[_size++].ulong_long_value = data;
	}

	inline void add_longlong(long long data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::LONG_LONG;
		_value[_size++].long_long_value = data;

	}

	inline void add_uint(unsigned int data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::UINT;
		_value[_size++].uint_value = data;
	}

	inline void add_int(int data)
	{
		check_space_and_grow(1);
		_type[_size] = fmt::internal::Arg::INT;
		_value[_size++].int_value = data;
	}

	inline void add_arg(fmt::internal::Arg& data)
	{
		check_space_and_grow(1);
		_value[_size++] = data;
	}

	size_t size() { return _size; }

//	fmt::internal::Arg* data() { return _data; }
	fmt::internal::Value::Type* ValueTypeArray(void) { return _type; }
	fmt::internal::Value* ValueArray(void) { return _value; }

	void clear(void) { _size = 0; }

	inline uint64_t GetCompressedType(void)
	{
		// The compressed type field is a int64 that uses 4 bits for each entry, up to MAX_PACKED_ARGS.
		uint64_t RetVal = 0;
		unsigned long nToGo = _size<fmt::ArgList::MAX_PACKED_ARGS?_size: fmt::ArgList::MAX_PACKED_ARGS;
		// Mask is not required, just for safety. The type field has to be set to values between 0x00 and 0x0f, all other 
		// values are errors at all.
		uint64_t mask = 0xf;
		for (unsigned long i = 0; i < nToGo; i++)
		{
			RetVal |= ((_type[i] & mask) << (i * 4));
		}
		return RetVal;

	}



};

