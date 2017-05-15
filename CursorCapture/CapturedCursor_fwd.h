#pragma once

namespace mousecapture
{

class CCapturedCursor;
class CDIBitmap;

struct MouseButtonsState
{
	bool leftPressed = false;
	bool rightPressed = false;
	bool middlePressed= false;

	bool operator==(const MouseButtonsState& rhs) const
	{
		return leftPressed == rhs.leftPressed
			&& rightPressed == rhs.rightPressed
			&& middlePressed == rhs.middlePressed;
	}

	bool operator!=(const MouseButtonsState& rhs) const
	{
		return !(*this == rhs);
	}
};

} // namespace mousecapture
