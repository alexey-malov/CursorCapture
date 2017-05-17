#include "stdafx.h"
#include "DIBitmap.h"

namespace mousecapture
{

CDIBitmap& CDIBitmap::operator=(CDIBitmap&& other)
{
	if (this != std::addressof(other))
	{
		m_bitmap.Attach(other.m_bitmap.Detach());


		m_width = other.m_width;
		m_height = other.m_height;
		other.m_width = other.m_height = 0;

		m_bits = other.m_bits;
		other.m_bits = Span();
	}
	return *this;
}

CDIBitmap::CDIBitmap(HDC dc, unsigned width, unsigned height)
	: m_width(width)
	, m_height(height)
{
	BITMAPINFO bi = { 0 };
	auto & bih = bi.bmiHeader;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biCompression = BI_RGB;
	bih.biPlanes = 1;
	bih.biBitCount = 32;
	bih.biWidth = width;
	bih.biHeight = height;
	LPVOID bits = nullptr;
	ATLVERIFY(m_bitmap.CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0));
	m_bits = gsl::make_span(reinterpret_cast<uint32_t*>(bits), width * height);
}

CDIBitmap::CDIBitmap(CDIBitmap&& other)
{
	*this = std::move(other);
}

HBITMAP CDIBitmap::GetBitmap() const
{
	return m_bitmap;
}

CDIBitmap::ConstSpan CDIBitmap::GetScanline(unsigned scanline) const
{
	return m_bits.subspan((GetHeight() - 1 - scanline) * GetWidth(), GetWidth());
}

CDIBitmap::ConstSpan CDIBitmap::GetData() const
{
	return m_bits;
}

CDIBitmap::Span CDIBitmap::GetData()
{
	return m_bits;
}

} // namespace mousecapture
