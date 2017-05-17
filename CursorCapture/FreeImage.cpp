#include "StdAfx.h"
#include "FreeImage.h"

#include <sstream>

#include "FreeImageIO.h"

namespace freeimagecpp
{

CFreeImage::CFreeImage()
	:m_pBitmap(nullptr)
{
}

CFreeImage::CFreeImage(const wchar_t * path, int flags)
	: m_pBitmap(FreeImage_LoadU(GetFileType(path), path, flags))
{
	if (!m_pBitmap)
	{
		throw std::runtime_error("load");
	}
}

CFreeImage::CFreeImage(CFreeImage const& rhs)
	:m_pBitmap(FreeImage_Clone(rhs.m_pBitmap))
{
	if (!m_pBitmap)
	{
		throw std::runtime_error("clone");
	}
}

CFreeImage::CFreeImage(CFreeImage && rhs)
	:m_pBitmap(nullptr)
{
	swap_no_check(rhs);
}

CFreeImage::CFreeImage(FIBITMAP * pBitmap)
	:m_pBitmap(pBitmap)
{
}

CFreeImage::CFreeImage(int width, int height, int bpp /*= 0*/, unsigned redMask /*= 0*/, unsigned greenMask /*= 0*/, unsigned blueMask /*= 0*/)
	: m_pBitmap(FreeImage_Allocate(width, height, bpp, redMask, greenMask, blueMask))
{
	if (!m_pBitmap)
	{
		throw std::runtime_error("Allocation error");
	}
}

CFreeImage::CFreeImage(FREE_IMAGE_FORMAT fif, IFreeImageIO & io, int flags)
	: m_pBitmap(nullptr)
{
	CFreeImageIO ioWrapper(io);

	m_pBitmap = FreeImage_LoadFromHandle(fif, &ioWrapper, &ioWrapper, flags);
	if (!m_pBitmap)
	{
		throw std::runtime_error("I/O wrapper constructor");
	}
	try
	{
		ioWrapper.RethrowException();
	}
	catch (...)
	{
		FreeImage_Unload(m_pBitmap);
		throw;
	}
}

CFreeImage::~CFreeImage()
{
	if (m_pBitmap)
	{
		FreeImage_Unload(m_pBitmap);
	}
}

CFreeImage CFreeImage::CloneFromRawBitmap(FIBITMAP * pBitmap)
{
	auto result = CFreeImage(FreeImage_Clone(pBitmap));
	if (!result.m_pBitmap)
	{
		throw std::runtime_error("clone from raw");
	}
	return result;
}

FREE_IMAGE_FORMAT CFreeImage::GetFileType(const wchar_t * path)
{
	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeU(path);
	if (format == FIF_UNKNOWN)
	{
		throw std::runtime_error("get file type");
	}
	return format;
}

std::pair<unsigned, unsigned> CFreeImage::GetDotsPerMeterXY() const
{
	const unsigned dpmX = FreeImage_GetDotsPerMeterX(m_pBitmap);
	const unsigned dpmY = FreeImage_GetDotsPerMeterY(m_pBitmap);
	return {dpmX, dpmY};
}

void CFreeImage::SetDotsPerMeterXY(const std::pair<unsigned, unsigned> &value)
{
	FreeImage_SetDotsPerMeterX(m_pBitmap, value.first);
	FreeImage_SetDotsPerMeterY(m_pBitmap, value.second);
}

CFreeImage CFreeImage::ConvertTo32Bits()const
{
	FIBITMAP * pBitmap = FreeImage_ConvertTo32Bits(GetBitmap());
	if (!pBitmap)
	{
		throw std::runtime_error("convert to 32 bits");
	}
	return CFreeImage(pBitmap);
}

CFreeImage CFreeImage::ConvertTo24Bits()const
{
	FIBITMAP * pBitmap = FreeImage_ConvertTo24Bits(GetBitmap());
	if (!pBitmap)
	{
		throw std::runtime_error("convert to 24 bits");
	}
	return CFreeImage(pBitmap);
}

CFreeImage CFreeImage::ConvertTo8Bits() const
{
	FIBITMAP * pBitmap = FreeImage_ConvertTo8Bits(GetBitmap());
	if (!pBitmap)
	{
		throw std::runtime_error("convert to 8 bits");
	}
	return CFreeImage(pBitmap);
}

CFreeImage::operator bool()const
{
	return !!m_pBitmap;
}

void CFreeImage::swap(CFreeImage & rhs)
{
	if (this != &rhs)
	{
		swap_no_check(rhs);
	}
}

CFreeImage & CFreeImage::operator=(CFreeImage const& rhs)
{
	if (this != &rhs)
	{
		CFreeImage tmp(rhs);
		swap_no_check(tmp);
	}
	return *this;
}

CFreeImage & CFreeImage::operator=(CFreeImage && rhs)
{
	swap(rhs);
	return *this;
}

FREE_IMAGE_TYPE CFreeImage::GetImageType()const
{
	return FreeImage_GetImageType(GetBitmap());
}

unsigned CFreeImage::GetBPP()const
{
	return FreeImage_GetBPP(GetBitmap());
}

CFreeImage CFreeImage::ConvertToType(FREE_IMAGE_TYPE type)const
{
	FIBITMAP * pBitmap = FreeImage_ConvertToType(GetBitmap(), type);
	if (pBitmap)
	{
		throw std::runtime_error("convert to type");
	}
	return CFreeImage(pBitmap);
}

CFreeImage CFreeImage::GetChannel(FREE_IMAGE_COLOR_CHANNEL channel)const
{
	FIBITMAP * pBitmap = FreeImage_GetChannel(GetBitmap(), channel);
	if (!pBitmap)
	{
		throw std::runtime_error("get channel");
	}
	return CFreeImage(pBitmap);
}

void CFreeImage::Save(FREE_IMAGE_FORMAT format, const wchar_t * path, int flags)const
{
	if (!FreeImage_SaveU(format, GetBitmap(), path, flags))
	{
		throw std::runtime_error("save");
	}
}

unsigned CFreeImage::GetWidth()const
{
	return FreeImage_GetWidth(GetBitmap());
}

unsigned CFreeImage::GetHeight()const
{
	return FreeImage_GetHeight(GetBitmap());
}

CFreeImage CFreeImage::Rescale(int width, int height, FREE_IMAGE_FILTER filter)const
{
	FIBITMAP * pBitmap = FreeImage_Rescale(GetBitmap(), width, height, filter);
	if (!pBitmap)
	{
		throw std::runtime_error("rescale");
	}
	return CFreeImage(pBitmap);
}

freeimagecpp::CFreeImage CFreeImage::Copy(int left, int top, int width, int height) const
{
	FIBITMAP * pBitmap = FreeImage_Copy(GetBitmap(), left, top, left + width, top + height);
	if (!pBitmap)
	{
		throw std::runtime_error("Copy failed");
	}
	return CFreeImage(pBitmap);
}

void CFreeImage::Paste(CFreeImage const& src, int left, int top, int alpha)
{
	if (!FreeImage_Paste(GetBitmap(), src.GetBitmap(), left, top, alpha))
	{
		throw std::runtime_error("Paste failed");
	}
}

void CFreeImage::CreateICCProfile(const std::vector<uint8_t> &profileBytes)
{
	// FreeImage uses data as const, but `const` keyword was forgotten.
	// So we use `const_cast`.
	if (!FreeImage_CreateICCProfile(GetBitmap(), const_cast<uint8_t*>(profileBytes.data()), long(profileBytes.size())))
	{
		throw std::runtime_error("CreateICCProfile failed");
	}
}

void CFreeImage::DestroyICCProfile()
{
	FreeImage_DestroyICCProfile(GetBitmap());
}

void CFreeImage::FlipVertical()
{
	if (!FreeImage_FlipVertical(GetBitmap()))
	{
		throw std::runtime_error("Flip vertical");
	}
}

void CFreeImage::FlipHorizontal()
{
	if (!FreeImage_FlipHorizontal(GetBitmap()))
	{
		throw std::runtime_error("Flip horizontal");
	}
}

bool CFreeImage::HasICCProfile() const
{
	const FIICCPROFILE *profile = FreeImage_GetICCProfile(GetBitmap());

	return profile && profile->data;
}

bool CFreeImage::IsTransparent() const
{
	return (TRUE == FreeImage_IsTransparent(GetBitmap()));
}

FREE_IMAGE_COLOR_TYPE CFreeImage::GetColorType()const
{
	return FreeImage_GetColorType(GetBitmap());
}

void CFreeImage::SetChannel(CFreeImage const& image, FREE_IMAGE_COLOR_CHANNEL channel)
{
	if (!FreeImage_SetChannel(GetBitmap(), image.GetBitmap(), channel))
	{
		throw std::runtime_error("set channel");
	}
}

unsigned CFreeImage::GetTransparencyCount()const
{
	return FreeImage_GetTransparencyCount(GetBitmap());
}

void CFreeImage::SaveToHandle(FREE_IMAGE_FORMAT format, IFreeImageIO & io, int flags)const
{
	CFreeImageIO ioWrapper(io);

	if (!FreeImage_SaveToHandle(format, GetBitmap(), &ioWrapper, &ioWrapper, flags))
	{
		throw std::runtime_error("save to handle");
	}

	ioWrapper.RethrowException();
}

const BYTE * CFreeImage::GetBits()const
{
	BYTE * pBits = FreeImage_GetBits(GetBitmap());
	if (!pBits)
	{
		throw std::runtime_error("get bits");
	}
	return pBits;
}

BYTE * CFreeImage::GetBits()
{
	BYTE * pBits = FreeImage_GetBits(GetBitmap());
	if (!pBits)
	{
		throw std::runtime_error("get bits");
	}
	return pBits;
}

const BYTE * CFreeImage::GetScanLine(int line)const
{
	BYTE * pBits = FreeImage_GetScanLine(GetBitmap(), line);
	if (!pBits)
	{
		throw std::runtime_error("get scan line");
	}
	return pBits;
}

BYTE * CFreeImage::GetScanLine(int line)
{
	BYTE * pBits = FreeImage_GetScanLine(GetBitmap(), line);
	if (!pBits)
	{
		throw std::runtime_error("get scan line");
	}
	return pBits;
}

CFreeImage CFreeImage::EnlargeCanvas(int left, int top, int width, int height, RGBQUAD backgroundRGBA) const
{
	FIBITMAP * pBitmap = FreeImage_EnlargeCanvas(GetBitmap(), left, top, width, height, &backgroundRGBA, FI_COLOR_IS_RGBA_COLOR);
	if (!pBitmap)
	{
		throw std::runtime_error("enlarge canvas");
	}
	return CFreeImage(pBitmap);
}

CFreeImage CFreeImage::Composite(RGBQUAD backgroundRGB) const
{
	FIBITMAP *pBitmap = FreeImage_Composite(GetBitmap(), FALSE, &backgroundRGB);
	if (!pBitmap)
	{
		throw std::runtime_error("composite with color");
	}
	return CFreeImage(pBitmap);
}

CFreeImage CFreeImage::Composite(const CFreeImage &backgroundImage) const
{
	FIBITMAP *pBitmap = FreeImage_Composite(GetBitmap(), FALSE, nullptr, backgroundImage.GetBitmap());
	if (!pBitmap)
	{
		throw std::runtime_error("composite with image");
	}
	return CFreeImage(pBitmap);
}

}
