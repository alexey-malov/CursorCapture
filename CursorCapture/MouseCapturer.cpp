#include "stdafx.h"
#include "MouseCapturer.h"
#include "CapturedCursor.h"
#include "crc64.h"

namespace mousecapture
{
using boost::for_each;
using boost::adaptors::map_values;

CDIBitmapData::CDIBitmapData(const CDIBitmap& bitmap)
	: m_bits(bitmap.GetWidth() * bitmap.GetHeight())
	, m_width(bitmap.GetWidth())
	, m_height(bitmap.GetHeight())
{
	auto srcData = bitmap.GetData();
	auto dstData = gsl::make_span(m_bits);
	assert(srcData.size() == dstData.size());

	for (unsigned scanline = 0; scanline < m_height; ++scanline)
	{
		auto srcScanline = srcData.subspan((m_height - 1 - scanline) * m_width, m_width);
		auto dstScanline = dstData.subspan(scanline * m_width, m_width);
		assert(srcScanline.length_bytes() == dstScanline.length_bytes());
		memcpy(dstScanline.data(), srcScanline.data(), dstScanline.length_bytes());
	}
}

CursorFrameDescription::CursorFrameDescription(const CDIBitmapData* colorBitmap, const CDIBitmapData* maskBitmap,
	const MouseButtonsState& mouseState, const POINT& screenPos, const POINT& hotspot)
	: colorBitmap(colorBitmap), maskBitmap(maskBitmap)
	, mouseState(mouseState), screenPos(screenPos), hotspot(hotspot)
{
}

bool CursorFrameDescription::operator==(const CursorFrameDescription& rhs) const
{
	return (colorBitmap == rhs.colorBitmap) && (maskBitmap == rhs.maskBitmap)
		&& (mouseState == rhs.mouseState) && (screenPos == rhs.screenPos)
		&& (hotspot == rhs.hotspot);
}

bool CursorFrameDescription::operator!=(const CursorFrameDescription& rhs) const
{
	return !(*this == rhs);
}

CMouseCapturer::~CMouseCapturer() = default;

void CMouseCapturer::CaptureCursor(const Timestamp& timestamp)
{
	auto newCursor = std::make_unique<CCapturedCursor>(m_prevCursor.get());
	
	auto frameDesc = CreateCursorFrameDescriptor(*newCursor);

	if (m_cursorFrames.empty() 
		|| ((timestamp > m_cursorFrames.back().timestamp) 
			&& (frameDesc != m_cursorFrames.back().description))
		)
	{
		m_cursorFrames.emplace_back(frameDesc, timestamp);
	}

	m_prevCursor = std::move(newCursor);
}

void CMouseCapturer::EnumerateImages(const std::function<void(const CDIBitmapData& img)> &fn) const
{
	for_each(m_images | map_values, fn);
}

void CMouseCapturer::EnumerateFrames(const std::function<void(const TimedCursorFrame& frame)>& fn) const
{
	for_each(m_cursorFrames, fn);
}

CursorFrameDescription CMouseCapturer::CreateCursorFrameDescriptor(const CCapturedCursor& cursor)
{
	if (!cursor.IsVisible())
	{
		return {};
	}

	auto& cursorImage = cursor.GetImage();
	return {
		RegisterImage(cursorImage.GetColor()),
		RegisterImage(cursorImage.GetMask()),
		cursor.GetMouseButtonsState(),
		cursor.GetScreenPos(),
		cursorImage.GetHotspot()
	};
}

CDIBitmapData* CMouseCapturer::RegisterImage(const CDIBitmap& dib)
{
	if (!dib)
	{
		return nullptr;
	}
	DIBitmapID dibId(dib);

	auto pos = m_images.find(dibId);
	if (pos == m_images.end())
	{
		std::tie(pos, std::ignore) = m_images.emplace(dibId, dib);
	}
	assert(pos != m_images.end());
	return &pos->second;
}

CMouseCapturer::TimedCursorFrame::TimedCursorFrame(
	const CursorFrameDescription& descr, 
	const Timestamp& timestamp) 
	: description(descr)
	, timestamp(timestamp)
{
}

CMouseCapturer::DIBitmapID::DIBitmapID(const CDIBitmap& dib)
{
	auto data = dib.GetData();
	auto byteSpan = gsl::make_span(
		reinterpret_cast<const uint8_t*>(data.data()),
		data.size_bytes()
	);
	crc64 = utils::Crc64(byteSpan);
	width = dib.GetWidth();
	height = dib.GetHeight();
}

bool CMouseCapturer::DIBitmapID::operator!=(const DIBitmapID& rhs) const
{
	return !(*this == rhs);
}

bool CMouseCapturer::DIBitmapID::operator==(const DIBitmapID& rhs)const
{
	return crc64 == rhs.crc64 && width == rhs.width && height == rhs.height;
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
