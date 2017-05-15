// CursorCaptureView.cpp : implementation of the CCursorCaptureView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "CursorCaptureView.h"
#include "CapturedCursor.h"

using boost::optional;

template <typename Fn>
void AutoSelectObject(HDC dc, HGDIOBJ obj, Fn&& fn)
{
	auto oldObject = SelectObject(dc, obj);
	fn();
	SelectObject(dc, oldObject);
}

CCursorCaptureView::~CCursorCaptureView()
{

}

BOOL CCursorCaptureView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

void ComposeIcon(const CIconInfo& ii, WTL::CDC& dc, int x, int y)
{
	WTL::CDC srcDC;
	WTL::CWindowDC desktopDC(nullptr);
	srcDC.CreateCompatibleDC(desktopDC);

	auto color = ii.GetColor();
	auto mask = ii.GetMask();

	optional<BITMAP> bmColor;
	if (color)
	{
		BITMAP b;
		ATLVERIFY(color.GetBitmap(&b));
		bmColor = b;
	}

	bool isInvertingCursor = false;

	optional<BITMAP> bmMask;
	if (mask)
	{
		BITMAP b;
		ATLVERIFY(mask.GetBitmap(&b));
		bmMask = b;

		auto w = bmMask->bmWidth;
		auto h = !color ? bmMask->bmHeight / 2 : bmMask->bmHeight;


		CBitmap dib;
		BITMAPINFO bmi = { 0 };
		BITMAPINFOHEADER & bih = bmi.bmiHeader;
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = w;
		bih.biHeight = h;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		LPVOID bits = nullptr;


		auto oldBitmap = srcDC.SelectBitmap(mask);

		CDC memDC;
		memDC.CreateCompatibleDC(desktopDC);
		ATLVERIFY(dib.CreateDIBSection(desktopDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0));
		auto ooo = memDC.SelectBitmap(dib);
		ATLVERIFY(memDC.BitBlt(0, 0, w, h, srcDC, 0, 0, SRCCOPY));
		std::vector<uint32_t> buffer(w * h);
		ATLVERIFY(dib.GetDIBits(memDC, 0, h, buffer.data(), &bmi, DIB_RGB_COLORS));
		memDC.SelectBitmap(ooo);

		srcDC.SelectBitmap(oldBitmap);

		isInvertingCursor = boost::algorithm::all_of_equal(buffer, RGB(255, 255, 255));
	}

	if (mask && (isInvertingCursor || !color))
	{
		auto oldBitmap = srcDC.SelectBitmap(mask);
		auto w = bmMask->bmWidth;
		bool bwCursor = !bmColor;
		auto h = bwCursor ? bmMask->bmHeight / 2 : bmMask->bmHeight;
		dc.BitBlt(x, y, w, h, srcDC, 0, 0, SRCAND);
		

		if (bwCursor)
		{
			CBitmap dib;
			BITMAPINFO bmi = { 0 };
			BITMAPINFOHEADER & bih = bmi.bmiHeader;
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biWidth = w;
			bih.biHeight = h;
			bih.biPlanes = 1;
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			LPVOID bits = nullptr;

			CDC memDC;
			memDC.CreateCompatibleDC(desktopDC);
			ATLVERIFY(dib.CreateDIBSection(desktopDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0));
			auto ooo = memDC.SelectBitmap(dib);
			ATLVERIFY(memDC.BitBlt(0, 0, w, h, srcDC, 0, h, SRCCOPY));

			dc.BitBlt(x, y, w, h, memDC, 0, 0, SRCINVERT);
			memDC.SelectBitmap(ooo);
		}
		else
		{
			ATLASSERT(color);
			srcDC.SelectBitmap(color);
			dc.BitBlt(x, y, w, h, srcDC, 0, 0, SRCINVERT);
		}
		srcDC.SelectBitmap(oldBitmap);
	}
	else
	{
		ATLASSERT(bmColor);
		auto oldBitmap = srcDC.SelectBitmap(color);

		BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		ATLVERIFY(dc.AlphaBlend(x, y, bmColor->bmWidth, bmColor->bmHeight, srcDC, 0, 0, bmColor->bmWidth, bmColor->bmHeight, bf));

		srcDC.SelectBitmap(oldBitmap);
	}
}

LRESULT CCursorCaptureView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	//TODO: Add your drawing code here
	CURSORINFO ci{ sizeof(CURSORINFO)};
	if (!GetCursorInfo(&ci))
	{
		return 0;
	}
	{
		CRect rc(200, 200, 250, 250);
		dc.SetBkColor(RGB(0, 255, 0));
		ATLVERIFY(dc.ExtTextOutW(0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));

	}

	m_capturedCursor = std::make_unique<CCapturedCursor>(m_capturedCursor.get());
	auto & img = m_capturedCursor->GetImage();
	CDC srcDC;
	srcDC.CreateCompatibleDC();
	if (img.GetMask())
	{
		AutoSelectObject(srcDC, img.GetMask().GetBitmap(), [&] {
			ATLVERIFY(dc.BitBlt(195, 195, img.GetWidth(), img.GetHeight(), srcDC, 0, 0, SRCAND));
		});
		AutoSelectObject(srcDC, img.GetColor().GetBitmap(), [&] {
			ATLVERIFY(dc.BitBlt(195, 195, img.GetWidth(), img.GetHeight(), srcDC, 0, 0, SRCINVERT));
		});
	}
	else
	{
		AutoSelectObject(srcDC, img.GetColor().GetBitmap(), [&] {
			BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

			ATLVERIFY(dc.AlphaBlend(195, 195, img.GetWidth(), img.GetHeight(), srcDC, 0, 0, img.GetWidth(), img.GetHeight(), bf));
		});
	}
	
	return 0;


	//CCursor cursorIcon = reinterpret_cast<HCURSOR>(CopyImage(ci.hCursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
	CIconHandle cursorIcon = ci.hCursor;
	//CIconHandle cursorIcon = LoadCursor(NULL, IDC_IBEAM);
	//CIconHandle cursorIcon = LoadCursor(_Module.m_hInst, MAKEINTRESOURCE(IDC_EDIT_CURSOR));
	//CIconHandle cursorIcon = LoadCursor(_Module.m_hInst, MAKEINTRESOURCE(IDC_BEAM_I));
	//CIconHandle cursorIcon = LoadCursor(NULL, IDC_ARROW);
	//CIconHandle cursorIcon = LoadCursor(_Module.m_hInst, MAKEINTRESOURCE(IDC_ARROW_I));
	//CIconHandle cursorIcon = LoadCursor(_Module.m_hInst, MAKEINTRESOURCE(IDC_EDIT_CURSOR));
	//CIconHandle cursorIcon = LoadCursor(NULL, IDC_CROSS);
	//CIconHandle cursorIcon = LoadCursor(NULL, IDC_WAIT);

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
				WTL::CWindowDC desktopDC(nullptr);
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

			{
				CRect rc(originX, originY + 10, originX + 32, originY + 32);
				dc.SetBkColor(RGB(255, 0, 0));
				ATLVERIFY(dc.ExtTextOutW(0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));

			}

			auto drawnSuccessfully = draw(originX, originY);
			if (drawnSuccessfully)
			{
				{
					CRect rc(originX + 90, originY - 10, originX + 100 + 42, originY + 32 + 10);
					dc.SetBkColor(RGB(0, 120, 90));
					ATLVERIFY(dc.ExtTextOutW(0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));
				}
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
