#include "stdafx.h"
#include "CapturedCursor.h"
#include "Utils.h"

#include "resource.h"


namespace mousecapture
{

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
std::array<HCURSOR, N> ResourceIdsToHCursors(const LPCWSTR(&resources)[N], std::index_sequence<Is...>)
{
	return{ { LoadCursor(nullptr, resources[Is])... } };
}
template <size_t N, size_t... Is>
std::array<HCURSOR, N> ResourceIdsToHCursors(const LPCWSTR(&resources)[N])
{
	return ResourceIdsToHCursors(resources, std::make_index_sequence<N>());
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
		return std::find(m_builtinCursors.begin(), m_builtinCursors.end(), cursor) != m_builtinCursors.end();
	}
private:
	std::array<HCURSOR, std::extent<decltype(STANDARD_CURSOR_IDS)>::value> m_builtinCursors;
};

bool IsBuiltinCursorHandle(HCURSOR cursor)
{
	static const CBuiltInCursorHandles builtInCursors;
	return builtInCursors.IsBuiltinCursorHandle(cursor);
}

CDIBitmap CreateDIBitmapFromBitmap(HDC hSrcDC, HDC hDstDC, HBITMAP srcBitmap, int x, int y, unsigned width, unsigned height)
{
	CDCHandle dstDC(hDstDC);
	CDIBitmap dib(hSrcDC, width, height);
	AutoSelectObject(dstDC, dib.GetBitmap(), [&] {
		CDCHandle srcDC(hSrcDC);
		AutoSelectObject(srcDC, srcBitmap, [&] {
			ATLVERIFY(dstDC.BitBlt(0, 0, width, height, srcDC, x, y, SRCCOPY));
		});
	});

	return dib;
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
		std::swap(mbs.leftPressed, mbs.rightPressed);
	}
	return mbs;
}

} // anonymous namespace


using namespace std;
using boost::optional;

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
	m_cursor = 
		/*LoadCursor(_Module.m_hInst, MAKEINTRESOURCE(IDC_COLORED_CURSOR));*/
		ci.hCursor;//*/

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
		throw std::runtime_error("Failed to get icon info");
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

//////////////////////////////////////////////////////////////////////////

CCursorImage::CCursorImage(HCURSOR cursor, UINT frameIndex)
{
	CIconInfo iconInfo(cursor);
	auto mask = iconInfo.GetMask();
	if (!mask)
	{
		throw std::runtime_error("Cursor must have a mask");
	}

	m_hotspot = { iconInfo.GetXHotspot(), iconInfo.GetYHotspot() };

	auto color = iconInfo.GetColor();

	bool useMaskDrawing = !color;

	WTL::CWindowDC desktopDC(nullptr);
	WTL::CDC srcDC;
	ATLVERIFY(srcDC.CreateCompatibleDC(desktopDC));
	WTL::CDC dstDC;
	ATLVERIFY(dstDC.CreateCompatibleDC(desktopDC));

	BITMAP bmMask;
	ATLVERIFY(mask.GetBitmap(&bmMask));
	auto width = bmMask.bmWidth;
	auto height = color ? bmMask.bmHeight : bmMask.bmHeight / 2;

	auto makeDibFromCursor = [&dstDC, width, height, frameIndex, &cursor](UINT flags, bool makeOpaque) {
		CDIBitmap dib(dstDC, width, height);
		AutoSelectObject(dstDC, dib.GetBitmap(), [&] {
			ATLVERIFY(dstDC.DrawIconEx(0, 0, cursor, 0, 0, frameIndex, nullptr, flags));
		});
		if (makeOpaque)
		{
			for (auto & pixel : dib.GetData())
			{
				pixel |= 0xff000000;
			}
		}
		return dib;
	};

	if (color)
	{
		ATLASSERT(!useMaskDrawing);

		BITMAP bmColor;
		ATLVERIFY(color.GetBitmap(bmColor));
		if (bmColor.bmBitsPixel == 32)
		{
			auto maskDib = CreateDIBitmapFromBitmap(srcDC, dstDC, mask, 0, 0, width, height);
			useMaskDrawing = boost::algorithm::all_of_equal(maskDib.GetData(), RGB(255, 255, 255));
		}
	}

	if (useMaskDrawing)
	{
		m_mask = makeDibFromCursor(DI_MASK, true);
		m_color = makeDibFromCursor(DI_IMAGE, true);
	}
	else // Draw using DrawIconEx
	{
		m_color = makeDibFromCursor(DI_IMAGE, false);
	}

}

bool CCursorImage::IsEmpty() const
{
	return !m_color || !m_mask;
}

unsigned CCursorImage::GetWidth() const
{
	assert(m_color);
	return m_color.GetWidth();
}

unsigned CCursorImage::GetHeight() const
{
	assert(m_color);
	return m_color.GetHeight();
}

//////////////////////////////////////////////////////////////////////////

} // namespace mousecapture
