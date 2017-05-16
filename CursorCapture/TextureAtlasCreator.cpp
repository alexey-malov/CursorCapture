#include "stdafx.h"
#include "TextureAtlasCreator.h"
#include <algorithm>

namespace mousecapture
{
using boost::optional;
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

	auto widthDivergence = std::abs(atlasSize.cx - idealAtlasSize.cx) / static_cast<double>(idealAtlasSize.cx);
	auto heightDivergence = std::abs(atlasSize.cy - idealAtlasSize.cy) / static_cast<double>(idealAtlasSize.cy);
	return std::max(widthDivergence, heightDivergence);
}

struct AtlasSizeCandidate
{
	bool IsBetterThan(const AtlasSizeCandidate& other)const
	{
		return std::tie(score, unusedAreaDivergence, aspectRatioDivergence) <
			std::tie(other.score, other.unusedAreaDivergence, other.aspectRatioDivergence);
	}
	CSize atlasSize = { 0,0 };
	double aspectRatioDivergence = 0.0;
	double unusedAreaDivergence = 0.0;
	double score = 0.0;
};

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
	return winner->atlasSize;
}

CTextureAtlasCreator::CTextureAtlasCreator()
{
}


CTextureAtlasCreator::~CTextureAtlasCreator()
{
}

} // namespace mousecapture
