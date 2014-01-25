#pragma once

namespace c11log{
	namespace details{
		struct null_mutex
		{
			void lock()
			{}
			void unlock()
			{}
			bool try_lock()
			{
				return true;
			}
		};
	}
}