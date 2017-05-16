#pragma once

namespace mousecapture
{
class CDIBitmap;

class CDIBitmapData
{
public:
	explicit CDIBitmapData(const CDIBitmap& bitmap);
	CDIBitmapData(const CDIBitmapData&) = default;
	CDIBitmapData(CDIBitmapData&&) = default;

	CDIBitmapData& operator=(const CDIBitmapData&) = default;
	CDIBitmapData& operator=(CDIBitmapData&&) = default;

	gsl::span<const uint32_t> GetBits()const { return m_bits; }

	unsigned GetWidth()const { return m_width; }
	unsigned GetHeight()const { return m_height; }
private:
	std::vector<uint32_t> m_bits;
	unsigned m_width;
	unsigned m_height;
};

} // namespace mousecapture
