#include "stdafx.h"
#include "CapturedCursor.h"
#include <array>
#include <boost/iterator/transform_iterator.hpp>

using namespace std;

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
			auto result = m_getCursorFrameInfo(cursor, 0, 0, &frameInfo.displayRateInJiffies, &frameInfo.totalFrames);
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
	return{ {LoadCursor(nullptr, resources[Is])...} };
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

}



CCapturedCursor::CCapturedCursor(const CCapturedCursor *prevCursor)
	: m_cursorCaptureTick(GetTickCount64())
	, m_frameStartTick(m_cursorCaptureTick)
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

	// Calculate animation frames
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
				m_frameIndex = (m_frameIndex + framesSincePreviousFrame) % m_frameInfo.totalFrames;
				m_frameStartTick = m_frameStartTick + framesSincePreviousFrame * frameDuration;
			}
		}
	}
}

bool CCapturedCursor::IsVisible() const
{
	return m_isVisible;
}

CCapturedCursor::~CCapturedCursor()
{
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

CCursorImage::CCursorImage(CCursorImage && other)
{
	*this = std::move(other);
}

CCursorImage::CCursorImage(HCURSOR cursor, UINT frameIndex)
{
	CIconInfo iconInfo(cursor);
	auto mask = iconInfo.GetMask();
	auto color = iconInfo.GetMask();

}

bool CCursorImage::IsEmpty() const
{
	return !m_color || !m_mask;
}

CCursorImage& CCursorImage::operator=(CCursorImage && rhs)
{
	if (this != std::addressof(rhs))
	{
		m_color.Attach(rhs.m_color.Detach());
		m_mask.Attach(rhs.m_color.Detach());
	}
	return *this;
}

CIconInfo::CIconInfo(HICON icon) : m_info{ sizeof(m_info) }
{
	if (!GetIconInfoEx(icon, &m_info))
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
