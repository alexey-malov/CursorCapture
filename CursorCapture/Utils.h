#pragma once

namespace mousecapture
{

template <typename Fn>
void AutoSelectObject(HDC dc, HGDIOBJ obj, Fn&& fn)
{
	assert(dc);
	assert(obj);

	auto oldObject = ::SelectObject(dc, obj);
	BOOST_SCOPE_EXIT_ALL(&)
	{
		if (oldObject)
		{
			::SelectObject(dc, oldObject);
		}
	};
	fn();
}

} // namespace mousecapture