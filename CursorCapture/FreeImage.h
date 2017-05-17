#pragma once

#define FREEIMAGE_LIB
#include <FreeImage.h>

#include "FreeImage_fwd.h"

namespace freeimagecpp
{

class CFreeImage
{
public:
	CFreeImage();
	CFreeImage(const wchar_t * path, int flags = 0);
	CFreeImage(FREE_IMAGE_FORMAT fif, IFreeImageIO & io, int flags);
	CFreeImage(CFreeImage const& rhs);
	CFreeImage(CFreeImage && rhs);
	CFreeImage(int width, int height, int bpp = 0, unsigned redMask = 0, unsigned greenMask = 0, unsigned blueMask = 0);
	~CFreeImage();

	static CFreeImage CloneFromRawBitmap(FIBITMAP *pBitmap);

	CFreeImage & operator=(CFreeImage const& rhs);
	CFreeImage & operator=(CFreeImage && rhs);
	operator bool()const;
	void swap(CFreeImage & rhs);

	static FREE_IMAGE_FORMAT GetFileType(const wchar_t * path);

	std::pair<unsigned, unsigned> GetDotsPerMeterXY()const;
	void SetDotsPerMeterXY(const std::pair<unsigned, unsigned> &value);

	CFreeImage ConvertTo32Bits()const;
	CFreeImage ConvertTo24Bits()const;
	CFreeImage ConvertTo8Bits()const;
	CFreeImage ConvertToType(FREE_IMAGE_TYPE type)const;
	CFreeImage Rescale(int width, int height, FREE_IMAGE_FILTER filter)const;

	CFreeImage Copy(int left, int top, int width, int height)const;
	void Paste(CFreeImage const& src, int left, int top, int alpha = 256);

	void CreateICCProfile(const std::vector<uint8_t> &profileBytes);
	void DestroyICCProfile();

	void FlipVertical();
	void FlipHorizontal();

	bool HasICCProfile()const;
	bool IsTransparent()const;
	FREE_IMAGE_TYPE GetImageType()const;
	FREE_IMAGE_COLOR_TYPE GetColorType()const;
	unsigned GetBPP()const;
	unsigned GetTransparencyCount()const;
	unsigned GetWidth()const;
	unsigned GetHeight()const;
	const BYTE * GetBits()const;
	BYTE * GetBits();
	const BYTE * GetScanLine(int line)const;
	BYTE * GetScanLine(int line);

	CFreeImage GetChannel(FREE_IMAGE_COLOR_CHANNEL channel)const;
	void SetChannel(CFreeImage const& image, FREE_IMAGE_COLOR_CHANNEL channel);
	
	void Save(FREE_IMAGE_FORMAT format, const wchar_t * path, int flags)const;
	void SaveToHandle(FREE_IMAGE_FORMAT format, IFreeImageIO & io, int flags)const;

	CFreeImage EnlargeCanvas(int left, int top, int width, int height, RGBQUAD backgroundRGBA = { 0xFF, 0xFF, 0xFF, 0 })const;
	CFreeImage Composite(RGBQUAD backgroundRGB)const;
	CFreeImage Composite(const CFreeImage &backgroundImage)const;

private:
	CFreeImage(FIBITMAP * pBitmap);

	FIBITMAP * GetBitmap()const
	{
		if (!m_pBitmap)
		{
			throw std::runtime_error("not initialized");
		}
		return m_pBitmap;
	}

	void swap_no_check(CFreeImage & rhs)
	{
		std::swap(m_pBitmap, rhs.m_pBitmap);
	}

	FIBITMAP * m_pBitmap;
};

}

namespace std
{

template <>
inline void swap<freeimagecpp::CFreeImage>(freeimagecpp::CFreeImage & lhs, freeimagecpp::CFreeImage & rhs)
{
	lhs.swap(rhs);
}

}
