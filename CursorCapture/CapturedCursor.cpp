#include "stdafx.h"
#include "CapturedCursor.h"

namespace
{

/*
typedef HCURSOR(WINAPI* GET_CURSOR_FRAME_INFO)(HCURSOR, LPCWSTR, DWORD, DWORD*, DWORD*);
GET_CURSOR_FRAME_INFO fnGetCursorFrameInfo = 0;

HMODULE libUser32 = LoadLibraryA("user32.dll");
if (!libUser32)
{
return false;
}

fnGetCursorFrameInfo = reinterpret_cast<GET_CURSOR_FRAME_INFO>(GetProcAddress(libUser32, "GetCursorFrameInfo"));
if (!fnGetCursorFrameInfo)
{
return false;
}

DWORD displayRate, totalFrames;
fnGetCursorFrameInfo(hcursor, L"", 0, &displayRate, &totalFrames);*/

struct CursorFrameInfo
{
	DWORD displayRate;
	DWORD totalFrames;
};

class CCursorFrameInfoGetter
{
	typedef HCURSOR(WINAPI* GET_CURSOR_FRAME_INFO)(HCURSOR, DWORD reserved, DWORD iStep, DWORD* rateJiffies, DWORD* numSteps);
public:
	CCursorFrameInfoGetter()
	{
		HMODULE libUser32 = LoadLibrary(L"user32.dll");
		if (libUser32)
		{
			m_getCursorFrameInfo = reinterpret_cast<GET_CURSOR_FRAME_INFO>(GetProcAddress(libUser32, "GetCursorFrameInfo"));
			if (!m_getCursorFrameInfo)
			{
			}
		}
	}
	
	CursorFrameInfo GetCursorFrameInfo(CCursorHandle cursor)const
	{
		if (m_getCursorFrameInfo)
		{
			CursorFrameInfo frameInfo;
			m_getCursorFrameInfo(cursor, 0, 0, &frameInfo.displayRate, &frameInfo.totalFrames);
		}
		else
		{
			return {1, 1};
		}
	}
private:
	GET_CURSOR_FRAME_INFO m_getCursorFrameInfo = nullptr;
};

}

CCapturedCursor::CCapturedCursor()
{
	if (!GetCursorPos(&m_screenPos))
	{
		
	}
}


CCapturedCursor::~CCapturedCursor()
{
}
