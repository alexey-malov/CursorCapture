// CursorCaptureView.cpp : implementation of the CCursorCaptureView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "CursorCaptureView.h"
#include "CapturedCursor.h"

namespace 
{
template <typename Fn>
void AutoSelectObject(HDC dc, HGDIOBJ obj, Fn&& fn)
{
	auto oldObject = SelectObject(dc, obj);
	fn();
	SelectObject(dc, oldObject);
}
} // namespace 


using namespace mousecapture;

CCursorCaptureView::~CCursorCaptureView()
{

}

BOOL CCursorCaptureView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT CCursorCaptureView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

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
