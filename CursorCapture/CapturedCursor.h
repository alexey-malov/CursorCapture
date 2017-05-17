#pragma once
#include "CapturedCursor_fwd.h"
#include "DIBitmap.h"

namespace mousecapture
{

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
	ICONINFO m_info;
	WTL::CBitmap m_bmMask;
	WTL::CBitmap m_bmColor;
};



class CCursorImage
{
public:
	CCursorImage() = default;

	CCursorImage(HCURSOR cursor, UINT frameIndex);

	CCursorImage(CCursorImage && other) = default;
	CCursorImage(const CCursorImage&) = delete;

	CCursorImage& operator=(CCursorImage && rhs) = default;
	CCursorImage& operator=(const CCursorImage&) = delete;

	POINT GetHotspot()const { return m_hotspot; }

	unsigned GetWidth()const;
	unsigned GetHeight()const;

	const CDIBitmap& GetMask()const { return m_maskBitmap; }
	const CDIBitmap& GetColor()const { return m_colorBitmap; }
private:
	POINT m_hotspot = {0, 0};
	CDIBitmap m_maskBitmap;
	CDIBitmap m_colorBitmap;
};

class CCapturedCursor
{
public:
	CCapturedCursor(const CCapturedCursor *prevCursor = nullptr);
	CCapturedCursor(const CCapturedCursor&) = delete;
	CCapturedCursor& operator=(const CCapturedCursor&) = delete;

	ULONGLONG GetCaptureTick()const { return m_cursorCaptureTick; }

	MouseButtonsState GetMouseButtonsState()const;
	POINT GetScreenPos()const { return m_screenPos; }
	const CCursorImage& GetImage()const;

	bool IsVisible()const;
private:
	POINT m_screenPos;
	bool m_isVisible = false;
	ULONGLONG m_cursorCaptureTick; // cursor capture tick
	ULONGLONG m_frameStartTick; // frame capture tick
	UINT m_frameIndex = 0;
	WTL::CCursorHandle m_cursor;
	CursorFrameInfo m_frameInfo;
	CCursorImage m_image;
	MouseButtonsState m_mouseButtons;
};

} // namespace mousecapture
