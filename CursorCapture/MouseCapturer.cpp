#include "stdafx.h"
#include "MouseCapturer.h"
#include "CapturedCursor.h"
#include "crc64.h"

namespace mousecapture
{


DIBitmapID::DIBitmapID(const CDIBitmap& dib)
{
	auto data = dib.GetData();
	auto byteSpan = gsl::make_span(
		reinterpret_cast<const uint8_t*>(data.data()),
		reinterpret_cast<const uint8_t*>(&data.end()[0])
	);
	crc64 = utils::Crc64(byteSpan);
	width = dib.GetWidth();
	height = dib.GetHeight();
}

bool DIBitmapID::operator!=(const DIBitmapID& rhs) const
{
	return !(*this == rhs);
}

bool DIBitmapID::operator==(const DIBitmapID& rhs)const
{
	return crc64 == rhs.crc64 && width == rhs.width && height == rhs.height;
}

CHashedImage::CHashedImage(const CDIBitmap& bitmap, const DIBitmapID& id)
	: m_bits(bitmap.GetWidth() * bitmap.GetHeight())
	, m_id(id)
{
	auto srcData = bitmap.GetData();
	auto dstData = gsl::make_span(m_bits);
	assert(srcData.size() == dstData.size());
	const auto w = GetWidth();
	const auto h = GetHeight();
	for (unsigned scanline = 0; scanline < h; ++scanline)
	{
		auto srcScanline = srcData.subspan((h - 1 - scanline) * w, w);
		auto dstScanline = dstData.subspan(scanline * w, w);
		assert(srcScanline.length_bytes() == dstScanline.length_bytes());
		memcpy(dstScanline.data(), srcScanline.data(), dstScanline.length_bytes());
	}
}

CMouseCapturer::~CMouseCapturer() = default;

void CMouseCapturer::CaptureCursor()
{
	auto newCursor = std::make_unique<CCapturedCursor>(m_prevCursor.get());

	CursorFrameDescription currentCursorDesc;

	if (newCursor->IsVisible())
	{
		auto& cursorImage = newCursor->GetImage();
		auto& colorImage = cursorImage.GetColor();
		assert(colorImage);
		currentCursorDesc.colorBitmapId = RegisterImage(colorImage)->first;

		auto& maskImage = cursorImage.GetMask();
		if (maskImage)
		{
			currentCursorDesc.maskBitmapId = RegisterImage(maskImage)->first;
		}

		currentCursorDesc.mouseState = newCursor->GetMouseButtonsState();
		currentCursorDesc.screenPos = newCursor->GetScreenPos();
		currentCursorDesc.hotspot = cursorImage.GetHotspot();
	}

	m_prevCursor = std::move(newCursor);
}

CMouseCapturer::ImageStorage::iterator CMouseCapturer::RegisterImage(const CDIBitmap& dib)
{
	assert(dib);
	DIBitmapID dibId(dib);

	auto pos = m_images.find(dibId);
	if (pos == m_images.end())
	{
		std::tie(pos, std::ignore) = m_images.emplace(dibId, CHashedImage(dib, dibId));
	}
	assert(pos != m_images.end());
	return pos;
}

std::size_t CMouseCapturer::DIBitmapIDHasher::operator()(const DIBitmapID& k) const
{
	std::size_t hash = 0;
	boost::hash_combine(hash, k.crc64);
	boost::hash_combine(hash, k.width);
	boost::hash_combine(hash, k.height);
	return hash;
}

} // namespace mousecapture
