#pragma once

struct CursorFrameInfo
{
	DWORD displayRate;
	DWORD totalFrames;
};

bool operator==(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs);
bool operator!=(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs);

class CCapturedCursor
{
public:
	CCapturedCursor(const CCapturedCursor *prevCursor = nullptr);
	CCapturedCursor(const CCapturedCursor&) = delete;
	CCapturedCursor& operator=(const CCapturedCursor&) = delete;

	bool IsVisible()const;

	~CCapturedCursor();
private:
	POINT m_screenPos;
	bool m_isVisible;
	ULONGLONG m_captureTick;
	DWORD frameIndex = 0;
	WTL::CCursorHandle m_cursor;
	CursorFrameInfo m_frameInfo;

};

