#include "stdafx.h"
#include "CapturedCursor.h"
#include "Utils.h"

namespace mousecapture
{
using namespace std;
using boost::optional;
using boost::algorithm::all_of;

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

	CursorFrameInfo GetCursorFrameInfo(HCURSOR cursor)const
	{
		if (m_getCursorFrameInfo)
		{
			CursorFrameInfo frameInfo;
			ATLVERIFY(m_getCursorFrameInfo(cursor, 0, 0, &frameInfo.displayRateInJiffies, &frameInfo.totalFrames));
			return frameInfo;
		}
		else
		{
			return{ 0, 1 };
		}
	}
private:
	GET_CURSOR_FRAME_INFO m_getCursorFrameInfo = nullptr;
};

CursorFrameInfo GetCursorFrameInfo(HCURSOR cursor)
{
	static const CCursorFrameInfoGetter getter;
	return getter.GetCursorFrameInfo(cursor);
}

static const LPCWSTR STANDARD_CURSOR_IDS[] = {
	IDC_APPSTARTING,
	IDC_ARROW,
	IDC_CROSS,
	IDC_HAND,
	IDC_HELP,
	IDC_IBEAM,
	IDC_NO,
	IDC_SIZEALL,
	IDC_SIZENESW,
	IDC_SIZENS,
	IDC_SIZENWSE,
	IDC_SIZEWE,
	IDC_UPARROW,
	IDC_WAIT,
};

template <size_t N, size_t... Is>
array<HCURSOR, N> ResourceIdsToHCursors(const LPCWSTR(&resources)[N], index_sequence<Is...>)
{
	return{ { LoadCursor(nullptr, resources[Is])... } };
}
template <size_t N, size_t... Is>
array<HCURSOR, N> ResourceIdsToHCursors(const LPCWSTR(&resources)[N])
{
	return ResourceIdsToHCursors(resources, make_index_sequence<N>());
}

class CBuiltInCursorHandles
{
public:
	CBuiltInCursorHandles()
		: m_builtinCursors(ResourceIdsToHCursors(STANDARD_CURSOR_IDS))
	{

	}
	CBuiltInCursorHandles(const CBuiltInCursorHandles&) = delete;
	CBuiltInCursorHandles& operator=(const CBuiltInCursorHandles&) = delete;

	bool IsBuiltinCursorHandle(HCURSOR cursor)const
	{
		return find(m_builtinCursors.begin(), m_builtinCursors.end(), cursor) != m_builtinCursors.end();
	}
private:
	array<HCURSOR, extent<decltype(STANDARD_CURSOR_IDS)>::value> m_builtinCursors;
};

bool IsBuiltinCursorHandle(HCURSOR cursor)
{
	static const CBuiltInCursorHandles builtInCursors;
	return builtInCursors.IsBuiltinCursorHandle(cursor);
}

MouseButtonsState CaptureMouseButtonState()
{
	auto isButtonPressed = [](auto key) { return (0 != (GetAsyncKeyState(key) & 0x8000)); };

	MouseButtonsState mbs;
	mbs.leftPressed = isButtonPressed(VK_LBUTTON);
	mbs.rightPressed = isButtonPressed(VK_RBUTTON);
	mbs.middlePressed = isButtonPressed(VK_MBUTTON);
	if (GetSystemMetrics(SM_SWAPBUTTON))
	{
		swap(mbs.leftPressed, mbs.rightPressed);
	}
	return mbs;
}

bool BitmapHasInvertingPixels(const CDIBitmap &maskBitmap, const CDIBitmap &colorBitmap)
{
	bool invPixelFound = false;
	auto checkForInversion = [&](uint32_t maskPixel, uint32_t colorPixel) {
		if (maskPixel && colorPixel) invPixelFound = true;
	};
	boost::for_each(maskBitmap.GetData(), colorBitmap.GetData(), checkForInversion);
	return invPixelFound;
}

void GenerateAlphaChannelFromMask(const CDIBitmap &maskBitmap, CDIBitmap &colorBitmap)
{
	auto maskBits = maskBitmap.GetData();
	auto colorBits = colorBitmap.GetData();
	auto maskIt = maskBits.begin();
	auto colorIt = colorBits.begin();
	for (; maskIt != maskBits.end() && colorIt != colorBits.end(); ++maskIt, ++colorIt)
	{
		assert(!(*maskIt && *colorIt) && "Inversion pixels are not supported");
		*colorIt = *maskIt ? 0 : *colorIt | 0xff000000;
	}
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////////

bool operator==(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs)
{
	return
		lhs.displayRateInJiffies == rhs.displayRateInJiffies &&
		lhs.totalFrames == rhs.totalFrames;
}

bool operator!=(const CursorFrameInfo& lhs, const CursorFrameInfo& rhs)
{
	return !(lhs == rhs);
}

CCapturedCursor::CCapturedCursor(const CCapturedCursor *prevCursor)
	: m_cursorCaptureTick(GetTickCount64())
	, m_frameStartTick(m_cursorCaptureTick)
{
	CURSORINFO ci {sizeof(CURSORINFO)};
	if (!GetCursorInfo(&ci))
	{
		return;
	}

	m_screenPos = ci.ptScreenPos;
	m_cursor = ci.hCursor;

	m_isVisible = (ci.flags == CURSOR_SHOWING);
	if (!m_isVisible)
	{
		return;
	}

	m_mouseButtons = CaptureMouseButtonState();

	m_frameInfo = GetCursorFrameInfo(m_cursor);

	// Calculate animation frame
	{
		// TODO: handle custom cursor
		bool cursorIsSame = prevCursor &&
			(prevCursor->m_cursor == m_cursor) &&
			(prevCursor->m_isVisible) &&
			(m_frameInfo == prevCursor->m_frameInfo) &&
			IsBuiltinCursorHandle(m_cursor);

		if (cursorIsSame)
		{
			if (m_frameInfo.IsAnimated())
			{
				auto msSinceFrameStart = m_cursorCaptureTick - prevCursor->m_frameStartTick;
				auto frameDuration = m_frameInfo.GetFrameDurationInMs();
				auto framesSincePreviousFrame = msSinceFrameStart / frameDuration;
				m_frameIndex = (prevCursor->m_frameIndex + framesSincePreviousFrame) % m_frameInfo.totalFrames;
				m_frameStartTick = prevCursor->m_frameStartTick + framesSincePreviousFrame * frameDuration;
			}
		}
	}

	// Capture cursor image
	m_image = CCursorImage(m_cursor, m_frameIndex);
}

MouseButtonsState CCapturedCursor::GetMouseButtonsState() const
{
	return m_mouseButtons;
}

const CCursorImage& CCapturedCursor::GetImage() const
{
	return m_image;
}

bool CCapturedCursor::IsVisible() const
{
	return m_isVisible;
}

bool CursorFrameInfo::IsAnimated() const
{
	return displayRateInJiffies > 0 && totalFrames > 1;
}

DWORD CursorFrameInfo::GetFrameDurationInMs() const
{
	assert(IsAnimated());
	return displayRateInJiffies * 1000 / 60;
}

CIconInfo::CIconInfo(HICON icon) : m_info{ sizeof(m_info) }
{
	if (!GetIconInfo(icon, &m_info))
	{
		throw runtime_error("Failed to get icon info");
	}
	m_bmMask = m_info.hbmMask;
	m_bmColor = m_info.hbmColor;
}

WTL::CBitmapHandle CIconInfo::GetMask() const
{
	return m_bmMask.m_hBitmap;
}

WTL::CBitmapHandle CIconInfo::GetColor() const
{
	return m_bmColor.m_hBitmap;
}

int CIconInfo::GetXHotspot() const
{
	return m_info.xHotspot;
}

int CIconInfo::GetYHotspot() const
{
	return m_info.yHotspot;
}

CCursorImage::CCursorImage(HCURSOR cursor, UINT frameIndex)
{
	CIconInfo iconInfo(cursor);
	auto iconMask = iconInfo.GetMask();
	if (!iconMask)
	{
		assert(!"Cursor must have a mask");
		throw runtime_error("Cursor must have a mask");
	}

	m_hotspot = { iconInfo.GetXHotspot(), iconInfo.GetYHotspot() };

	auto iconColor = iconInfo.GetColor();

	WTL::CWindowDC desktopDC(nullptr);
	WTL::CDC dstDC;
	ATLVERIFY(dstDC.CreateCompatibleDC(desktopDC));

	BITMAP bmMask;
	ATLVERIFY(iconMask.GetBitmap(&bmMask));
	auto width = bmMask.bmWidth;
	auto height = iconColor ? bmMask.bmHeight : bmMask.bmHeight / 2;

	auto makeDibFromCursor = [&dstDC, width, height, frameIndex, &cursor](UINT flags) {
		CDIBitmap dib(dstDC, width, height);
		AutoSelectObject(dstDC, dib.GetBitmap(), [&] {
			ATLVERIFY(dstDC.DrawIconEx(0, 0, cursor, 0, 0, frameIndex, nullptr, flags));
		});
		return dib;
	};

	auto maskDib = makeDibFromCursor(DI_MASK);

	m_colorBitmap = makeDibFromCursor(DI_IMAGE);
	bool useMask = !iconColor;
	if (iconColor)
	{
		ATLASSERT(!useMask);

		BITMAP bmColor;
		ATLVERIFY(iconColor.GetBitmap(bmColor));

		auto isTransparent = [](uint32_t pixel) { return (pixel & 0xff000000) == 0; };
		useMask = all_of(m_colorBitmap.GetData(), isTransparent);
	}

	if (useMask)
	{
		if (BitmapHasInvertingPixels(maskDib, m_colorBitmap))
		{
			m_maskBitmap = move(maskDib);
			MakeOpaque(m_maskBitmap);
			MakeOpaque(m_colorBitmap);
		}
		else
		{
			GenerateAlphaChannelFromMask(maskDib, m_colorBitmap);
		}
	}

	assert(m_colorBitmap);
}

unsigned CCursorImage::GetWidth() const
{
	assert(m_colorBitmap);
	return m_colorBitmap.GetWidth();
}

unsigned CCursorImage::GetHeight() const
{
	assert(m_colorBitmap);
	return m_colorBitmap.GetHeight();
}

} // namespace mousecapture
