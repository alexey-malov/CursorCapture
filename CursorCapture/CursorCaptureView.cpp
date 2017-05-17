// CursorCaptureView.cpp : implementation of the CCursorCaptureView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "CursorCaptureView.h"
#include "CapturedCursor.h"
#include "Utils.h"

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
	CDC srcDC;
	srcDC.CreateCompatibleDC();

	if (m_textureAtlas.GetBitmap())
	{
		auto & atlasDib = m_textureAtlas.GetBitmap();
		AutoSelectObject(srcDC, atlasDib.GetBitmap(), [&] {
			ATLVERIFY(dc.BitBlt(0, 0, atlasDib.GetWidth(), atlasDib.GetHeight(), srcDC, 0, 0, SRCCOPY));
		});
		return 0;
	}

	{
		CRect rc(200, 200, 250, 250);
		dc.SetBkColor(RGB(0, 255, 0));
		ATLVERIFY(dc.ExtTextOutW(0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));
	}

	m_capturedCursor = std::make_unique<CCapturedCursor>(m_capturedCursor.get());
	auto & img = m_capturedCursor->GetImage();
	if (img.GetMask())
	{
		AutoSelectObject(srcDC, img.GetMask().GetBitmap(), [&] {
			ATLVERIFY(dc.BitBlt(195, 195, img.GetWidth(), img.GetHeight(), srcDC, 0, 0, SRCAND));
		});
		AutoSelectObject(srcDC, img.GetColor().GetBitmap(), [&] {
			ATLVERIFY(dc.BitBlt(195, 195, img.GetWidth(), img.GetHeight(), srcDC, 0, 0, SRCINVERT));
		});
	}
	else if (img.GetColor())
	{
		AutoSelectObject(srcDC, img.GetColor().GetBitmap(), [&] {
			BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

			ATLVERIFY(dc.AlphaBlend(195, 195, img.GetWidth(), img.GetHeight(), srcDC, 0, 0, img.GetWidth(), img.GetHeight(), bf));
		});
	}
	
	return 0;
}

void CCursorCaptureView::OnCaptureStop(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	KillTimer(IDT_TIMER1);

	m_textureAtlas = BuildTextureAtlas([this](auto && callback) {
		m_capturer.EnumerateImages(callback);
	});

	RedrawWindow();
}

void CCursorCaptureView::OnTimer(UINT_PTR /*nIDEvent*/)
{
	auto tick = GetTickCount64();
	if (!m_startTick)
	{
		m_startTick = tick;
	}
	m_capturer.CaptureCursor(TimedCursorState::Timestamp(tick - *m_startTick));

	RedrawWindow();
}

int CCursorCaptureView::OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
{
	ATLVERIFY(SetTimer(IDT_TIMER1, 10));
	return 0;
}
