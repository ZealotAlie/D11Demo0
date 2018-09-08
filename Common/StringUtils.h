#pragma once
#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <iostream>

class StringDetails {
	enum
	{
		STR_BUFFER_SIZE = 1024
	};
	__declspec(noinline) static char (&GetTempString())[STR_BUFFER_SIZE]{
		static char s_TempString[STR_BUFFER_SIZE];
		return s_TempString;
	}
	friend static const char* FormatString( const char *format, ... );
};

static const char* FormatString( const char *format, ... )
{
	va_list		argptr;
	int			len;
	auto& buf = StringDetails::GetTempString();

	va_start( argptr, format );
	len = _vsnprintf_s( buf, sizeof( buf ), format, argptr );
	buf[sizeof( buf ) - 1] = '\0';
	va_end( argptr );

	return buf;
}

/*static const wchar_t* CharToWChar(const char* ch)
{
	size_t length = strlen(ch);
	size_t converted = 0;
	static wchar_t* pTempWChar = nullptr;
	if (pTempWChar)
	{
		delete[] pTempWChar;
	}
	pTempWChar = new wchar_t[length + 1];

	mbstowcs_s(&converted, pTempWChar, length + 1, ch, length);
	pTempWChar[length] = '\0';
	return pTempWChar;
}*/

#endif