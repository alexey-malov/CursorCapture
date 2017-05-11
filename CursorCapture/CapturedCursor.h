#pragma once

struct CursorFrameInfo
{
	bool IsAnimated()const;
	DWORD GetFrameDurationInMs()const;
	DWORD displayRateInJiffies;
	DWORD totalFrames;
};

bool operator==(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs);
bool operator!=(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs);


class CIconInfo
{
public:
	CIconInfo(HICON icon);
	CIconInfo(const CIconInfo&) = delete;
	CIconInfo& operator=(const CIconInfo&) = delete;

	WTL::CBitmapHandle GetMask()const;
	WTL::CBitmapHandle GetColor()const;
	int GetXHotspot()const;
	int GetYHotspot()const;
private:
	ICONINFOEX m_info;
	WTL::CBitmap m_bmMask;
	WTL::CBitmap m_bmColor;
};

class CCursorImage
{
public:
	CCursorImage() = default;

	CCursorImage(HCURSOR cursor, UINT frameIndex);

	CCursorImage(CCursorImage && other);
	CCursorImage(const CCursorImage&) = delete;

	CCursorImage& operator=(CCursorImage && rhs);
	CCursorImage& operator=(const CCursorImage&) = delete;

	bool IsEmpty()const;

private:
	WTL::CBitmap m_mask;
	WTL::CBitmap m_color;
};

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
	ULONGLONG m_cursorCaptureTick; // cursor capture tick
	ULONGLONG m_frameStartTick; // frame capture tick
	DWORD m_frameIndex = 0;
	WTL::CCursorHandle m_cursor;
	CursorFrameInfo m_frameInfo;

};

