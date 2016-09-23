#pragma once
#include "spdlog\fmt\fmt.h"

class vararg_list_item : public fmt::internal::Arg
{
public:

	vararg_list_item()
	{
		this->type = this->NONE;
	}

	vararg_list_item(int int_value)
	{
		this->type = this->INT;
		this->int_value = int_value;
	}
	vararg_list_item(unsigned uint_value)
	{
		this->type = this->UINT;
		this->uint_value = uint_value;
	}
	vararg_list_item(__int64 long_long_value)
	{
		this->type = this->LONG_LONG;
		this->long_long_value = long_long_value;
	}

	vararg_list_item(unsigned __int64  ulong_long_value)
	{
		this->type = this->ULONG_LONG;
		this->ulong_long_value = ulong_long_value;
	}

	vararg_list_item(double double_value)
	{
		this->type = this->DOUBLE;
		this->double_value = double_value;
	}
	vararg_list_item(long double long_double_value)
	{
		this->type = this->LONG_DOUBLE;
		this->long_double_value = long_double_value;
	}

	vararg_list_item(char* lpStr)
	{
		this->type = this->CSTRING;
		this->string.value = lpStr;
		this->string.size = strlen(lpStr);
	}

	vararg_list_item(const char* lpStr)
	{
		this->type = this->CSTRING;
		this->string.value = lpStr;
		this->string.size = strlen(lpStr);
	}

	vararg_list_item(wchar_t* lpStr)
	{
		this->type = this->WSTRING;
		this->wstring.value = lpStr;
		// At the moment no idea if this should be the length or the size based on bytes...
		this->wstring.size = wcslen(lpStr);
	}

	vararg_list_item(const wchar_t* lpStr)
	{
		this->type = this->WSTRING;
		this->wstring.value = lpStr;
		// At the moment no idea if this should be the length or the size based on bytes...
		this->wstring.size = wcslen(lpStr);
	}
	vararg_list_item(unsigned char* lpStr)
	{
		this->type = this->STRING;
		this->sstring.value = (signed char*)lpStr;
		this->sstring.size = strlen((const char*)lpStr);
	}

	vararg_list_item(const unsigned char* lpStr)
	{
		this->type = this->STRING;
		this->sstring.value = (signed char*)lpStr;
		this->sstring.size = strlen((const char*)lpStr);
	}

	vararg_list_item(char ch)
	{
		this->type = this->CHAR;
		this->int_value = ch;
	}

	vararg_list_item(bool bVal)
	{
		this->type = this->BOOL;
		this->uint_value = bVal;
	}


	~vararg_list_item()
	{

	}
};

