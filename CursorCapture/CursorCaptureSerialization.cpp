#include "stdafx.h"
#include "CursorCaptureSerialization.h"
#pragma warning(push)
#pragma warning(disable: 4100)
#include "proto/src/UserInput.pb.h"
#pragma warning(pop)
#include "DIBitmap.h"
#include "FreeImage.h"
#include "TextureAtlasCreator.h"
#include <boost/interprocess/streams/vectorstream.hpp>
#include "FreeImageIOOstream.h"
#include "MouseCapturer.h"

namespace mousecapture
{

namespace MouseButtonStates
{

constexpr uint32_t LEFT_BUTTON_PRESSED = 1;
constexpr uint32_t RIGHT_BUTTON_PRESSED = 2;
constexpr uint32_t MIDDLE_BUTTON_PRESSED = 4;

}

using namespace freeimagecpp;
namespace
{
void SerializeTextureAtlas(const CTextureAtlas& atlas, pb_userinput::TextureAtlas& pbTextureAtlas)
{
	auto& dib = atlas.GetBitmap();
	auto w = dib.GetWidth();
	auto h = dib.GetHeight();

	// Create Free image PNG
	CFreeImage fi(w, h, 32, 0xff0000, 0x00ff00, 0x0000ff);
	auto data = dib.GetData();
	for (decltype(h) y = 0; y < h; ++y)
	{
		auto srcScanline = dib.GetScanline(y);
		memcpy(fi.GetScanLine(y), srcScanline.data(), srcScanline.size_bytes());
	}

	// Serialize texture atlas in PNG format
	{
		using Uint8Vector = std::vector<char>;
		boost::interprocess::basic_ovectorstream<Uint8Vector> byteVectorStream;
		byteVectorStream.reserve(dib.GetData().size_bytes() + 1000);
		CFreeImageIOOstream io(byteVectorStream);
		fi.SaveToHandle(FIF_PNG, io, PNG_Z_DEFAULT_COMPRESSION);
		fi.Save(FIF_PNG, LR"(c:\temp\cursor.png)", PNG_Z_DEFAULT_COMPRESSION);

		pbTextureAtlas.set_image_format(pb_userinput::TextureAtlas_ImageFormat_PNG);

		auto dataSpan = gsl::make_span(byteVectorStream.vector());
		pbTextureAtlas.set_image_data(dataSpan.data(), dataSpan.size_bytes());
	}

	// Serialize sub images
	for (auto & subImage : atlas.GetSubImages())
	{
		assert(subImage.left >= 0 && subImage.right > subImage.left);
		assert(subImage.top >= 0 && subImage.bottom > subImage.top);

		auto pbSubImage = pbTextureAtlas.add_sub_images();
		pbSubImage->set_left(subImage.left);
		pbSubImage->set_top(subImage.top);
		pbSubImage->set_width(subImage.right - subImage.left);
		pbSubImage->set_height(subImage.bottom - subImage.top);
	}
}

using ImgToBitmap = std::unordered_map<gsl::not_null<const CDIBitmapData*>, uint32_t>;

void SerializeTimedCursorState(const TimedCursorState & timedState, const ImgToBitmap& imgToBitmap,  pb_userinput::MouseCursorState &pbState)
{
	pbState.set_timestamp(timedState.timestamp.count());

	auto & state = timedState.state;
	if (state.colorBitmap)
	{
		pbState.set_color_image_index(imgToBitmap.at(state.colorBitmap));
		if (state.maskBitmap)
		{
			pbState.set_mask_image_index(imgToBitmap.at(state.maskBitmap));
		}

		pbState.set_hotspot_x(state.hotspot.x);
		pbState.set_hotspot_y(state.hotspot.y);
		pbState.set_cursor_x(state.screenPos.x);
		pbState.set_cursor_y(state.screenPos.y);
		auto & mouseState = state.mouseState;

		uint32_t buttonStateFlags =
			(mouseState.leftPressed ? MouseButtonStates::LEFT_BUTTON_PRESSED : 0) |
			(mouseState.rightPressed ? MouseButtonStates::RIGHT_BUTTON_PRESSED : 0) |
			(mouseState.middlePressed ? MouseButtonStates::MIDDLE_BUTTON_PRESSED : 0);
		if (buttonStateFlags != 0)
		{
			pbState.set_button_states(buttonStateFlags);
		}
	}
	else
	{
		assert(!state.maskBitmap);
	}
}

void SerializeCursorStates(const ICapturedCursorProvider& provider,
	::google::protobuf::RepeatedPtrField<pb_userinput::MouseCursorState>& pbStates)
{
	// map image pointers to image indices
	ImgToBitmap imgToBitmap;
	provider.EnumerateImages([&imgToBitmap](auto & image) {
		imgToBitmap.emplace(&image, gsl::narrow<ImgToBitmap::value_type::second_type>(imgToBitmap.size()));
	});

	provider.EnumerateStates([&imgToBitmap, &pbStates](const TimedCursorState & timedState) {
		SerializeTimedCursorState(timedState, imgToBitmap, *pbStates.Add());
	});
}

} // namespace

void SerializeCursor(const ICapturedCursorProvider& capturedCursorProvider, std::ostream& stream)
{
	pb_userinput::MouseCursor pbCursor;
	CTextureAtlas textureAtlas = BuildTextureAtlas([&capturedCursorProvider](auto && callback) {
		capturedCursorProvider.EnumerateImages(callback);
	});

	SerializeTextureAtlas(textureAtlas, *pbCursor.mutable_texture());
	SerializeCursorStates(capturedCursorProvider, *pbCursor.mutable_states());

	pbCursor.SerializeToOstream(&stream);
}

} // namespace mousecapture
