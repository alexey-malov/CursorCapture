// CursorCaptureView.cpp : implementation of the CCursorCaptureView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "CursorCaptureView.h"

BOOL CCursorCaptureView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

class CIconInfo
{
public:
	CIconInfo(HICON icon)
		: m_info{ sizeof(m_info) }
	{
		if (!GetIconInfoEx(icon, &m_info))
		{
			throw std::runtime_error("Failed to get icon info");
		}
		m_bmMask = m_info.hbmMask;
		m_bmColor = m_info.hbmColor;
	}
	CIconInfo(const CIconInfo&) = delete;
	CIconInfo& operator=(const CIconInfo&) = delete;

	CBitmapHandle GetMask()const
	{
		return m_bmMask.m_hBitmap;
	}
	CBitmapHandle GetColor()const
	{
		return m_bmColor.m_hBitmap;
	}
	int GetXHotspot()const
	{
		return m_info.xHotspot;
	}
	int GetYHotspot()const
	{
		return m_info.yHotspot;
	}
private:
	ICONINFOEX m_info;
	CBitmap m_bmMask;
	CBitmap m_bmColor;
};




void ComposeIcon(const CIconInfo& ii, WTL::CDC& dc, int x, int y)
{
	WTL::CDC srcDC;
	srcDC.CreateCompatibleDC(dc);
	int w = 32;
	int h = 32;
	{
		if (!ii.GetColor() && ii.GetMask())
		{
			auto oldBitmap = srcDC.SelectBitmap(ii.GetMask());
			BITMAP bmMask;
			ATLVERIFY(ii.GetMask().GetBitmap(&bmMask));
			ATLASSERT(h == bmMask.bmHeight / 2);
			dc.StretchBlt(x, y, w, h, srcDC, 0, 0, w, h, SRCAND);
			dc.StretchBlt(x, y, w, h, srcDC, 0, h, w, h, SRCINVERT);
			srcDC.SelectBitmap(oldBitmap);
		}
		else if (ii.GetColor())
		{
			auto oldBitmap = srcDC.SelectBitmap(ii.GetColor());

			BITMAP bmColor;
			ATLVERIFY(ii.GetColor().GetBitmap(&bmColor));

			BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
			ATLVERIFY(dc.AlphaBlend(x, y, w, h, srcDC, 0, 0, w, h, bf));

			srcDC.SelectBitmap(oldBitmap);
		}
	}
}

LRESULT CCursorCaptureView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	//TODO: Add your drawing code here
	CURSORINFO ci{ sizeof(CURSORINFO)};
	ATLVERIFY(GetCursorInfo(&ci));

	
	//CCursor cursorIcon = reinterpret_cast<HCURSOR>(CopyImage(ci.hCursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
	CIconHandle cursorIcon = ci.hCursor;//CopyIcon(ci.hCursor);
	
	if (ci.flags == CURSOR_SHOWING && cursorIcon)
	{
		CIconInfo ii(cursorIcon);


		auto drawCursor = [&dc, &ii, &cursorIcon](UINT stepIndex) {
			int originX = 10;
			int originY = 10;

			auto draw = [&cursorIcon, &dc, stepIndex](int x, int y, UINT flags = DI_NORMAL) {
				return dc.DrawIconEx(x, y, cursorIcon, 0, 0, stepIndex, nullptr, flags) != FALSE;
			};

			auto captureCursor = [&cursorIcon, stepIndex]()
			{
				auto desktopDC = ::GetDC(nullptr);
				WTL::CBitmap outBitmap;
				BITMAPINFO bi = { 0 };
				auto & bih = bi.bmiHeader;
				bih.biSize = sizeof(BITMAPINFOHEADER);
				bih.biBitCount = 32;
				bih.biWidth = 32;
				bih.biHeight = 32;
				bih.biCompression = BI_RGB;
				bih.biPlanes = 1;
				LPVOID bits = nullptr;
				ATLVERIFY(outBitmap.CreateDIBSection(desktopDC, &bi, DIB_RGB_COLORS, &bits, nullptr, 0));

				CDC memDC;
				memDC.CreateCompatibleDC(desktopDC);
				auto oldBitmap = memDC.SelectBitmap(outBitmap);
				memDC.DrawIconEx(0, 0, cursorIcon, 0, 0, stepIndex, nullptr, DI_IMAGE);
				memDC.SelectBitmap(oldBitmap);
				GdiFlush();
				return outBitmap.Detach();
			};

			auto drawnSuccessfully = draw(originX, originY);
			if (drawnSuccessfully)
			{
				draw(originX + 100, originY, DI_MASK);
				draw(originX + 100, originY + 50, DI_IMAGE);

				{
					CRect rc(originX + 200, originY, originX + 200 + 32, originY + 32);
					dc.SetBkColor(RGB(255, 0, 0));
					ATLVERIFY(dc.ExtTextOutW(0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));
					ComposeIcon(ii, dc, originX + 200, originY);
				}

				dc.SetPixel(originX + ii.GetXHotspot(), originY + ii.GetYHotspot(), RGB(255, 0, 0));

				{
					CBitmap cursorBitmap = captureCursor();
					WTL::CDC srcDC;
					srcDC.CreateCompatibleDC(dc);
					auto oldBitmap = srcDC.SelectBitmap(cursorBitmap);

					CRect rc(originX + 300, originY + 10, originX + 300 + 32, originY + 32 + 10);
					dc.SetBkColor(RGB(255, 0, 0));
					ATLVERIFY(dc.ExtTextOutW(0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));

					BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					ATLVERIFY(dc.AlphaBlend(originX + 300, originY, 32, 32, srcDC, 0, 0, 32, 32, bf));

					srcDC.SelectBitmap(oldBitmap);
				}
			}
			return drawnSuccessfully;
		};


		if (!drawCursor(UINT_MAX)) // try to draw static cursor
		{
			// cursor is animated
			if (!drawCursor(m_animationStepIndex++))
			{
				m_animationStepIndex = 0;
				ATLVERIFY(drawCursor(m_animationStepIndex++));
			}
		}
	}


	return 0;
}

void CCursorCaptureView::OnTimer(UINT_PTR /*nIDEvent*/)
{
	RedrawWindow();
}

int CCursorCaptureView::OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
{
	ATLVERIFY(SetTimer(IDT_TIMER1, 10));
	return 0;
}
