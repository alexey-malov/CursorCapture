#pragma once


namespace mousecapture
{

class CDIBitmap
{
public:
	using Span = gsl::span<uint32_t>;
	using ConstSpan = gsl::span<const uint32_t>;
	CDIBitmap() = default;
	CDIBitmap(HDC dc, unsigned width, unsigned height);
	CDIBitmap(CDIBitmap&& other);
	CDIBitmap(const CDIBitmap&) = delete;

	CDIBitmap& operator=(CDIBitmap&& other);
	CDIBitmap& operator=(const CDIBitmap&) = delete;

	unsigned GetWidth()const { return m_width; }
	unsigned GetHeight()const { return m_height; }

	HBITMAP GetBitmap()const;

	explicit operator bool()const { return !!m_bitmap; }

	ConstSpan GetData()const;
	Span GetData();
private:
	WTL::CBitmap m_bitmap;
	Span m_bits;
	unsigned m_width = 0;
	unsigned m_height = 0;
};

} // namespace mousecapture
