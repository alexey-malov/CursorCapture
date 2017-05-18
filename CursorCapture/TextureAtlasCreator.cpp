#include "stdafx.h"
#include "TextureAtlasCreator.h"
#include "MouseCapturer.h"
#include "DIBitmapData.h"
#include "Utils.h"

namespace mousecapture
{
using boost::optional;
using namespace std;

namespace 
{
unsigned CeilingDiv(int a, int b)
{
	assert(b != 0);
	return (a + b - 1) / b;
}

CSize FindSquareAtlasRowsCols(const CSize& imageSize, int imageCount)
{
	assert(imageCount != 0);
	assert(imageSize.cx > 0);
	assert(imageSize.cy > 0);
	const int imageArea = imageSize.cx * imageSize.cy * imageCount;
	const int squareAtlasSize = gsl::narrow_cast<int>(ceil(sqrt(imageArea)));


	auto squareAtlasColumns = CeilingDiv(squareAtlasSize, imageSize.cx);
	assert(squareAtlasColumns != 0);
	auto squareAtlasRows = CeilingDiv(imageCount, squareAtlasColumns);
	assert(squareAtlasRows != 0);
	{
		auto atlasRows1 = CeilingDiv(squareAtlasSize, imageSize.cy);
		auto atlasColumns1 = CeilingDiv(imageCount, atlasRows1);
		if (atlasRows1 * atlasColumns1 < squareAtlasColumns * squareAtlasRows)
		{
			squareAtlasColumns = atlasColumns1;
			squareAtlasRows = atlasRows1;
		}
	}
	return { gsl::narrow<int>(squareAtlasColumns), gsl::narrow<int>(squareAtlasRows) };
}

double CalculateAspectRatioDivergence(const CSize& atlasSize, const CSize& idealAtlasSize)
{
	assert(atlasSize.cx > 0 && atlasSize.cy > 0);
	assert(idealAtlasSize.cx > 0 && idealAtlasSize.cy > 0);

	auto widthDivergence = abs(atlasSize.cx - idealAtlasSize.cx) / static_cast<double>(idealAtlasSize.cx);
	auto heightDivergence = abs(atlasSize.cy - idealAtlasSize.cy) / static_cast<double>(idealAtlasSize.cy);
	return max(widthDivergence, heightDivergence);
}

struct AtlasSizeCandidate
{
	bool IsBetterThan(const AtlasSizeCandidate& other)const
	{
		return tie(score, unusedAreaDivergence, aspectRatioDivergence) <
			tie(other.score, other.unusedAreaDivergence, other.aspectRatioDivergence);
	}
	CSize atlasSize = { 0,0 };
	double aspectRatioDivergence = 0.0;
	double unusedAreaDivergence = 0.0;
	double score = 0.0;
};

struct NestedDIB
{
	NestedDIB(const CDIBitmapData& bitmapData, size_t index)
		: bitmapData(&bitmapData)
		, index(index)
	{
	}

	gsl::not_null<const CDIBitmapData*> bitmapData;
	size_t index;
	CRect frame;
};

CSize PlaceImagesOnAtlas(gsl::span<NestedDIB> images, const CSize &textureAtlasSize)
{
	// Sort images by width, then by height
	boost::sort(images, [](const auto& lhs, const auto& rhs) {
		auto lhsBitmap = lhs.bitmapData;
		auto rhsBitmap = rhs.bitmapData;
		return make_pair(lhsBitmap->GetWidth(), lhsBitmap->GetHeight()) >
			make_pair(rhsBitmap->GetWidth(), rhsBitmap->GetHeight());
	});

	CSize usedAtlasSize;

	unsigned shelfWidth = 0;
	unsigned shelfLeft = 0;
	unsigned shelfTop = 0;
	for (auto & image : images)
	{
		auto bd = image.bitmapData;

		assert(bd->GetHeight() <= gsl::narrow<unsigned>(textureAtlasSize.cy));
		// Find a place for the image
		do
		{
			if (shelfWidth == 0)
			{
				shelfWidth = bd->GetWidth();
			}

			// find place on current shelf
			if (bd->GetHeight() + shelfTop > gsl::narrow<unsigned>(textureAtlasSize.cy))
			{
				shelfTop = 0;
				shelfLeft += shelfWidth;
				shelfWidth = 0;
			}
		} while (shelfWidth == 0);

		assert(bd->GetWidth() <= shelfWidth);

		unsigned shelfBottom = shelfTop + bd->GetHeight();
		assert(shelfBottom <= gsl::narrow<unsigned>(textureAtlasSize.cy));

		image.frame.SetRect(shelfLeft, shelfTop, shelfLeft + bd->GetWidth(), shelfBottom);
		shelfTop = shelfBottom;

		usedAtlasSize.cx = std::max(usedAtlasSize.cx, image.frame.right);
		usedAtlasSize.cy = std::max(usedAtlasSize.cy, image.frame.bottom);

		assert(usedAtlasSize.cx <= textureAtlasSize.cx);
		assert(usedAtlasSize.cy <= textureAtlasSize.cy);
	}

	return usedAtlasSize;
}

void DrawNestedImagesOnDIBitmap(CDIBitmap & target, const gsl::span<const NestedDIB>& nestedDibs)
{
	CDC outDC;
	outDC.CreateCompatibleDC();
	AutoSelectObject(outDC, target.GetBitmap(), [&] {
		BITMAPINFO bi = { 0 };
		BITMAPINFOHEADER& bih = bi.bmiHeader;
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biPlanes = 1;
		for (auto & dib : nestedDibs)
		{
			bih.biWidth = dib.frame.Width();
			bih.biHeight = dib.frame.Height();
			ATLVERIFY(outDC.SetDIBitsToDevice(
				dib.frame.left, dib.frame.top,
				dib.frame.Width(), dib.frame.Height(),
				0, 0,
				0, dib.frame.Height(),
				dib.bitmapData->GetBits().data(),
				&bi, DIB_RGB_COLORS));
		}
	});
}

} // anonymous namespace

CSize FindOptimalTextureAtlasDimensions(const CSize& imageSize, int imageCount)
{
	assert(imageCount > 0);
	assert(imageSize.cx > 0);
	assert(imageSize.cy > 0);

	constexpr double areaImportance = 12.0;
	constexpr double ratioImportance = 1.0;

	const int imageArea = imageSize.cx * imageSize.cy * imageCount;

	const CSize squareAtlasCells = FindSquareAtlasRowsCols(imageSize, imageCount);
	
	optional<AtlasSizeCandidate> winner;
	for (int columns = 1; columns <= imageCount; ++columns)
	{
		const int rows = CeilingDiv(imageCount, columns);

		CSize atlasSize(columns * imageSize.cx, rows * imageSize.cy);

		AtlasSizeCandidate candidate;
		candidate.atlasSize = atlasSize;
		candidate.unusedAreaDivergence = ((atlasSize.cx * atlasSize.cy) - imageArea) / static_cast<double>(imageArea);
		candidate.aspectRatioDivergence = CalculateAspectRatioDivergence({ columns, rows }, squareAtlasCells);
		candidate.score = candidate.unusedAreaDivergence * areaImportance + candidate.aspectRatioDivergence * ratioImportance;
		if (!winner || candidate.IsBetterThan(*winner))
		{
			winner = candidate;
		}
	}
	assert(winner);
	assert(winner->atlasSize.cx >= imageSize.cx);
	assert(winner->atlasSize.cy >= imageSize.cy);
	return winner->atlasSize;
}

CTextureAtlas::CTextureAtlas(CDIBitmap && bitmap, const gsl::span<const RECT>& subImages)
	: m_bitmap(move(bitmap))
	, m_subImages(subImages.begin(), subImages.end())
{
}

gsl::span<const RECT> CTextureAtlas::GetSubImages() const
{
	return gsl::make_span(m_subImages);
}

CTextureAtlas BuildTextureAtlas(const ImageProvider& imageProvider)
{
	vector<NestedDIB> images;

	CSize largestImage;
	imageProvider([&](const CDIBitmapData& bd) {
		largestImage.cx = max<int>(largestImage.cx, bd.GetWidth());
		largestImage.cy = max<int>(largestImage.cy, bd.GetHeight());

		assert(largestImage.cx >= gsl::narrow<int>(bd.GetWidth()));
		assert(largestImage.cy >= gsl::narrow<int>(bd.GetHeight()));
		images.emplace_back(bd, images.size());
	});

	if (images.empty())
	{
		return CTextureAtlas();
	}

	const CSize textureAtlasSize = FindOptimalTextureAtlasDimensions(largestImage, gsl::narrow<int>(images.size()));
	auto usedAtlasSize = PlaceImagesOnAtlas(images, textureAtlasSize);

	CWindowDC desktopDC(nullptr);
	CDIBitmap dib(desktopDC, usedAtlasSize.cx, usedAtlasSize.cy);
	DrawNestedImagesOnDIBitmap(dib, images);

	boost::sort(images, [](auto & lhs, auto & rhs) {
		return lhs.index < rhs.index;
	});

	std::vector<RECT> targetRectangles;
	targetRectangles.reserve(images.size());
	boost::transform(images, back_inserter(targetRectangles), [](auto & image) {
		return image.frame;
	});
	
	return CTextureAtlas(move(dib), targetRectangles);
}

} // namespace mousecapture
