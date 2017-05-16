#pragma once
#include "DIBitmap.h"

namespace mousecapture
{
class CDIBitmapData;

CSize FindOptimalTextureAtlasDimensions(const CSize& imageSize, int imageCount);


class CTextureAtlas
{
public:
	CTextureAtlas() = default;
	CTextureAtlas(CDIBitmap && bitmap, const gsl::span<const RECT>& subImages);
	gsl::span<const RECT> GetSubImages()const;
	const CDIBitmap& GetBitmap()const { return m_bitmap; }
private:
	CDIBitmap m_bitmap;
	std::vector<RECT> m_subImages;
};

using ImageProviderCallback = std::function<void(const CDIBitmapData&)>;
using ImageProvider = std::function<void(const ImageProviderCallback&)>;

CTextureAtlas BuildTextureAtlas(const ImageProvider& imageProvider);


} // namespace mousecapture
