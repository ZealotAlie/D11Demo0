// WinMain.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <crtdbg.h>
#include "InstancingDemoApp.h"

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	InstancingDemoApp myApp(hInstance);

	if(!myApp.Init())
	{
		return 0;
	}

	return myApp.Run();
}