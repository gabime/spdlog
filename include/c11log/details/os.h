#pragma once
#include<string>
#include<cstdio>
#include<ctime>

namespace c11log
{
	namespace details
	{
		namespace os
		{			
			std::tm localtime(const std::time_t &time_t);			
			std::tm localtime(); 
			
		}		
	}
}
