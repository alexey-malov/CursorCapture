#pragma once

class CCapturedCursor
{
public:
	CCapturedCursor();
	CCapturedCursor(const CCapturedCursor&) = delete;
	CCapturedCursor& operator=(const CCapturedCursor&) = delete;

	~CCapturedCursor();
private:
	POINT m_screenPos;
};

