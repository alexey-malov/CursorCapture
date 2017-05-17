#pragma once

namespace mousecapture
{

class CTextureAtlas;
struct CursorState;

using CursorStateCallback = std::function<void(const CursorState& state, const std::chrono::milliseconds& timestamp)>;
using CursorStateProvider = std::function<void(const CursorStateCallback& callback)>;

class ICapturedCursorProvider;

void SerializeCursor(const ICapturedCursorProvider& capturedCursorProvider);


} // namespace mousecapture
