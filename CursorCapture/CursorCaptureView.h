// CursorCaptureView.h : interface of the CCursorCaptureView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CCapturedCursor;

class CCursorCaptureView : public CWindowImpl<CCursorCaptureView>
{
public:
	~CCursorCaptureView();
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CCursorCaptureView)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CREATE(OnCreate)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
private:
	static constexpr UINT_PTR IDT_TIMER1 = 1;
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnTimer(UINT_PTR nIDEvent);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	UINT m_animationStepIndex = 0;

	std::unique_ptr<CCapturedCursor> m_capturedCursor;
};
