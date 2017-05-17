#pragma once
#include "CapturedCursor_fwd.h"
#include "DIBitmapData.h"

namespace mousecapture
{

struct CursorState
{
	CursorState() = default;
	CursorState(
		const CDIBitmapData* colorBitmap,
		const CDIBitmapData* maskBitmap,
		const MouseButtonsState& mouseState,
		const POINT& screenPos,
		const POINT& hotspot
	);

	bool operator==(const CursorState& rhs)const;
	bool operator!=(const CursorState& rhs)const;

	const CDIBitmapData* colorBitmap = nullptr;
	const CDIBitmapData* maskBitmap = nullptr;
	MouseButtonsState mouseState;
	CPoint screenPos = {0, 0};
	CPoint hotspot = { 0, 0 };
};

struct TimedCursorState
{
	using Timestamp = std::chrono::milliseconds;

	TimedCursorState(const CursorState& state, const Timestamp& timestamp);

	CursorState state;
	Timestamp timestamp;
};

class ICapturedCursorProvider
{
public:

	virtual ~ICapturedCursorProvider() = default;

	virtual void EnumerateImages(const std::function<void(const CDIBitmapData& img)>& fn) const = 0;
	virtual void EnumerateStates(const std::function<void(const TimedCursorState& state)>& fn) const = 0;
};

class CMouseCapturer : public ICapturedCursorProvider
{
public:
	~CMouseCapturer();

	void CaptureCursor(const TimedCursorState::Timestamp& timestamp);

	void EnumerateImages(const std::function<void(const CDIBitmapData& img)>& fn) const override;
	void EnumerateStates(const std::function<void(const TimedCursorState& state)>& fn) const override;
private:
	// Bitmap id
	struct DIBitmapID
	{
		explicit DIBitmapID(const CDIBitmap& dib);

		bool operator==(const DIBitmapID& rhs)const;
		bool operator!=(const DIBitmapID& rhs)const;

		uint64_t crc64;
		unsigned width;
		unsigned height;
	};

	struct DIBitmapIDHasher
	{
		std::size_t operator()(const DIBitmapID& k)const;
	};

	// Cursor frame description with timestamp
	using ImageStorage = std::unordered_map<DIBitmapID, CDIBitmapData, DIBitmapIDHasher>;

	CursorState CreateCursorFrameDescriptor(const CCapturedCursor& cursor);
	CDIBitmapData* RegisterImage(const CDIBitmap& dib);

	std::unique_ptr<CCapturedCursor> m_prevCursor;
	ImageStorage m_images;
	std::deque<TimedCursorState> m_cursorStates;
};

} // namespace mousecapture
