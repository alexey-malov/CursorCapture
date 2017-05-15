#pragma once
#include "CapturedCursor_fwd.h"

namespace mousecapture
{

struct DIBitmapID
{
	explicit DIBitmapID(const CDIBitmap& dib);

	bool operator==(const DIBitmapID& rhs)const;
	bool operator!=(const DIBitmapID& rhs)const;

	uint64_t crc64;
	unsigned width;
	unsigned height;
};

class CHashedImage
{
public:
	explicit CHashedImage(const CDIBitmap& bitmap);
	CHashedImage(const CHashedImage&) = default;
	CHashedImage(CHashedImage&&) = default;

	CHashedImage& operator=(const CHashedImage&) = default;
	CHashedImage& operator=(CHashedImage&&) = default;

	unsigned GetWidth()const { return m_width; }
	unsigned GetHeight()const { return m_height; }

private:
	std::vector<uint32_t> m_bits;
	unsigned m_width;
	unsigned m_height;
};

struct CursorFrameDescription
{
	bool operator==(const CursorFrameDescription& rhs)const;
	bool operator!=(const CursorFrameDescription& rhs)const;

	const CHashedImage* colorBitmap = nullptr;
	const CHashedImage* maskBitmap = nullptr;
	MouseButtonsState mouseState;
	CPoint screenPos = {0, 0};
	CPoint hotspot = { 0, 0 };
};


class CMouseCapturer
{
public:
	using Timestamp = std::chrono::milliseconds;

	CMouseCapturer() = default;
	~CMouseCapturer();

	void CaptureCursor(const Timestamp& timestamp);
private:
	struct DIBitmapIDHasher
	{
		std::size_t operator()(const DIBitmapID& k)const;
	};
	using ImageStorage = std::unordered_map<DIBitmapID, CHashedImage, DIBitmapIDHasher>;

	CursorFrameDescription CreateCursorFrameDescriptor(const CCapturedCursor& cursor);
	CHashedImage& RegisterImage(const CDIBitmap& dib);

	std::unique_ptr<CCapturedCursor> m_prevCursor;
	ImageStorage m_images;
	struct TimedCursorFrame
	{
		TimedCursorFrame(const CursorFrameDescription& descr, const Timestamp& timestamp)
			: description(descr), timestamp(timestamp)
		{}

		CursorFrameDescription description;
		Timestamp timestamp;
	};
	std::deque<TimedCursorFrame> m_cursorFrames;
};

} // namespace mousecapture
