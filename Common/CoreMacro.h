#pragma once

#ifndef _CORE_MACRO_H_
#define _CORE_MACRO_H_

#include <iostream>
#include "WinHelper.h"
#include "StringUtils.h"

#ifdef _DEBUG
static char __DebugMsgBuffer[1024];
#define Assert(Expr, Msg, ...) \
	do{\
		if(!(Expr))\
		{\
			sprintf_s(__DebugMsgBuffer, _countof(__DebugMsgBuffer),Msg,__VA_ARGS__);\
			strcpy_s(__DebugMsgBuffer,_countof(__DebugMsgBuffer),FormatString("Assert Failed: %s\n %s",#Expr,__DebugMsgBuffer));\
			if(WinHelper::DisapplyAssertBox(__DebugMsgBuffer,__FILE__,__LINE__))\
			{\
				DebugBreak(); \
			}\
		}\
	} while (0)

#define DxAssert(Hr,Msg, ...) \
	Assert(SUCCEEDED(Hr),Msg,__VA_ARGS__)

#else
#define Assert(Expr, Msg, ...)	;
#define DxAssert(Hr,Msg, ...)	;
#endif

#endif