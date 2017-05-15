#pragma once
#include "CapturedCursor_fwd.h"

namespace mousecapture
{

class CCapturedCursor;
class CDIBitmap;

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
	explicit CHashedImage(const CDIBitmap& bitmap, const DIBitmapID& id);
	CHashedImage(const CHashedImage&) = default;
	CHashedImage(CHashedImage&&) = default;

	CHashedImage& operator=(const CHashedImage&) = default;
	CHashedImage& operator=(CHashedImage&&) = default;

	unsigned GetWidth()const { return m_id.width; }
	unsigned GetHeight()const { return m_id.height; }

private:
	std::vector<uint32_t> m_bits;
	DIBitmapID m_id;
};

struct CursorFrameDescription
{
	boost::optional<DIBitmapID> colorBitmapId;
	boost::optional<DIBitmapID> maskBitmapId;
	MouseButtonsState mouseState;
	POINT screenPos = {0, 0};
	POINT hotspot = { 0, 0 };
};

class CMouseCapturer
{
public:
	CMouseCapturer() = default;
	~CMouseCapturer();

	void CaptureCursor();
private:
	struct DIBitmapIDHasher
	{
		std::size_t operator()(const DIBitmapID& k)const;
	};
	using ImageStorage = std::unordered_map<DIBitmapID, CHashedImage, DIBitmapIDHasher>;
	using Timestamp = std::chrono::milliseconds;


	ImageStorage::iterator RegisterImage(const CDIBitmap& dib);

	std::unique_ptr<CCapturedCursor> m_prevCursor;
	ImageStorage m_images;
	
	std::map<Timestamp, CursorFrameDescription> m_cursorFrames;
};

} // namespace mousecapture
