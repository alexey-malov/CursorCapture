#pragma once

namespace mousecapture
{

template <typename Fn>
void AutoSelectObject(HDC dc, HGDIOBJ obj, Fn&& fn)
{
	auto oldObject = SelectObject(dc, obj);
	fn();
	SelectObject(dc, oldObject);
}

} // namespace mousecapture