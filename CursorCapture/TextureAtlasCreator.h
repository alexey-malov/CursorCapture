#pragma once

namespace mousecapture
{

CSize FindOptimalTextureAtlasDimensions(const CSize& imageSize, int imageCount);

class CTextureAtlasCreator
{
public:
	CTextureAtlasCreator();
	~CTextureAtlasCreator();
};

} // namespace mousecapture
