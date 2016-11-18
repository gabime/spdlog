/*
Formatting library for C++ - std::ostream support

Copyright (c) 2012 - 2016, Victor Zverovich
All rights reserved.

For the license information refer to format.h.
*/

#include "ostream.h"

namespace fmt {

	namespace internal {
		FMT_FUNC void write(std::ostream &os, Writer &w)
		{
			const char *data = w.data();
			typedef internal::MakeUnsigned<std::streamsize>::Type UnsignedStreamSize;
			UnsignedStreamSize size = w.size();
			UnsignedStreamSize max_size =
				internal::to_unsigned((std::numeric_limits<std::streamsize>::max)());
			do {
				UnsignedStreamSize n = size <= max_size ? size : max_size;
				os.write(data, static_cast<std::streamsize>(n));
				data += n;
				size -= n;
			} while (size != 0);
		}
	}

	FMT_FUNC void print(std::ostream &os, CStringRef format_str, ArgList args)
	{
		MemoryWriter w;
		w.write(format_str, args);
		internal::write(os, w);
	}
}  // namespace fmt
