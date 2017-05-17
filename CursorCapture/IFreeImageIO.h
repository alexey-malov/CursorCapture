#pragma once

#include "FreeImage_fwd.h"

namespace freeimagecpp
{

class IFreeImageIO
{
public:
	virtual unsigned Read(void * buffer, unsigned size, unsigned count) = 0;
	virtual unsigned Write(void * buffer, unsigned size, unsigned count) = 0;
	virtual int Seek(long offset, int origin) = 0;
	virtual long Tell() = 0;

	virtual ~IFreeImageIO(){}
};

}
