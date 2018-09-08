#pragma once
#ifndef _WIN_HELPER_H_
#define _WIN_HELPER_H_

#include <windows.h>

class WinHelper {
public:
	static void SetWindow(HWND wd) {
		Get().hWnd = wd;
	}
	static bool DisapplyAssertBox(const char * msg, const char* file, const int line);
private:
	static WinHelper& Get();
	HWND hWnd;
};

#endif