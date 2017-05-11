#include "stdafx.h"
#include "CapturedCursor.h"

using namespace std;

bool operator==(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs)
{
	return
		lhs.displayRate == rhs.displayRate &&
		lhs.totalFrames == rhs.totalFrames;
}

bool operator!=(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs)
{
	return !(lhs == rhs);
}

namespace
{



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
		}
	}
	
	CursorFrameInfo GetCursorFrameInfo(CCursorHandle cursor)const
	{
		if (m_getCursorFrameInfo)
		{
			CursorFrameInfo frameInfo;
			auto result = m_getCursorFrameInfo(cursor, 0, 0, &frameInfo.displayRate, &frameInfo.totalFrames);
			return frameInfo;
		}
		else
		{
			return {1, 1};
		}
	}
private:
	GET_CURSOR_FRAME_INFO m_getCursorFrameInfo = nullptr;
};

CursorFrameInfo GetCursorFrameInfo(CCursorHandle cursor)
{
	static CCursorFrameInfoGetter getter;
	return getter.GetCursorFrameInfo(cursor);
}

}

CCapturedCursor::CCapturedCursor(const CCapturedCursor *prevCursor)
	:m_captureTick(GetTickCount64())
{
	CURSORINFO ci {sizeof(CURSORINFO)};
	if (!GetCursorInfo(&ci))
	{
		throw runtime_error("Failed to get cursor info");
	}

	m_screenPos = ci.ptScreenPos;
	m_cursor = ci.hCursor;

	m_isVisible = (ci.flags == CURSOR_SHOWING);
	if (!m_isVisible)
	{
		return;
	}

	m_frameInfo = GetCursorFrameInfo(m_cursor);
	if (prevCursor && (prevCursor->m_cursor == m_cursor))
	{
		// It can be the same cursor
	}
}

bool CCapturedCursor::IsVisible() const
{
	return m_isVisible;
}

CCapturedCursor::~CCapturedCursor()
{
}
