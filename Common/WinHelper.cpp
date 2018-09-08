
#include "WinHelper.h"
#include "StringUtils.h"

bool WinHelper::DisapplyAssertBox(const char * msg, const char * file, const int line)
{
	OutputDebugStringA(FormatString("[Engine Message]%s%s Line:%d", msg, file, line));
	return MessageBoxA(
		nullptr, 
		FormatString("%s%s Line:%d", msg, file, line),
		"Engine Message", 
		MB_ABORTRETRYIGNORE | MB_ICONWARNING) 
		== IDABORT;
}

WinHelper & WinHelper::Get()
{
	static WinHelper s_helper;
	return s_helper;
}
