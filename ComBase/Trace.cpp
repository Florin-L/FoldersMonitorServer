
#include <Windows.h>
#include <cstdio>
#include <cstdarg>
#include "Trace.h"

static const int kBufferSize = 1024;

namespace com {

	//
	void Trace(const wchar_t *fmt, ...)
	{
		if (!g_enableTraces)
			return;

		wchar_t buff[kBufferSize + 1] = {0};

		va_list args;
		va_start(args, fmt);

		wvsprintf(buff, fmt, args);
		::OutputDebugString(buff);
		
		va_end(args);
	}

} // com
