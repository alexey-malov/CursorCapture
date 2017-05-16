#pragma once
#include "CapturedCursor_fwd.h"
#include "DIBitmapData.h"

namespace mousecapture
{

struct CursorFrameDescription
{
	CursorFrameDescription() = default;
	CursorFrameDescription(
		const CDIBitmapData* colorBitmap,
		const CDIBitmapData* maskBitmap,
		const MouseButtonsState& mouseState,
		const POINT& screenPos,
		const POINT& hotspot
	);

	bool operator==(const CursorFrameDescription& rhs)const;
	bool operator!=(const CursorFrameDescription& rhs)const;

	const CDIBitmapData* colorBitmap = nullptr;
	const CDIBitmapData* maskBitmap = nullptr;
	MouseButtonsState mouseState;
	CPoint screenPos = {0, 0};
	CPoint hotspot = { 0, 0 };
};

class CMouseCapturer
{
public:
	using Timestamp = std::chrono::milliseconds;

	struct TimedCursorFrame
	{
		TimedCursorFrame(const CursorFrameDescription& descr, const Timestamp& timestamp);

		CursorFrameDescription description;
		Timestamp timestamp;
	};

	~CMouseCapturer();

	void CaptureCursor(const Timestamp& timestamp);

	void EnumerateImages(const std::function<void(const CDIBitmapData& img)>& fn) const;
	void EnumerateFrames(const std::function<void(const TimedCursorFrame& frame)>& fn) const;
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

	CursorFrameDescription CreateCursorFrameDescriptor(const CCapturedCursor& cursor);
	CDIBitmapData* RegisterImage(const CDIBitmap& dib);

	std::unique_ptr<CCapturedCursor> m_prevCursor;
	ImageStorage m_images;
	std::deque<TimedCursorFrame> m_cursorFrames;
};

} // namespace mousecapture
