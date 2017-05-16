#include "stdafx.h"
#include "DIBitmapData.h"
#include "DIBitmap.h"

namespace mousecapture
{

CDIBitmapData::CDIBitmapData(const CDIBitmap& bitmap)
	: m_bits(bitmap.GetWidth() * bitmap.GetHeight())
	, m_width(bitmap.GetWidth())
	, m_height(bitmap.GetHeight())
{
	auto srcData = bitmap.GetData();
	auto dstData = gsl::make_span(m_bits);
	assert(srcData.size() == dstData.size());
	memcpy(dstData.data(), srcData.data(), srcData.size_bytes());
/*
	for (unsigned scanline = 0; scanline < m_height; ++scanline)
	{
		auto srcScanline = srcData.subspan((m_height - 1 - scanline) * m_width, m_width);
		auto dstScanline = dstData.subspan(scanline * m_width, m_width);
		assert(srcScanline.length_bytes() == dstScanline.length_bytes());
		memcpy(dstScanline.data(), srcScanline.data(), dstScanline.length_bytes());
	}*/
}

} // namespace mousecapture
