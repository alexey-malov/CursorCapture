#pragma once

#include "IFreeImageIO.h"

namespace freeimagecpp
{

class CFreeImageIOOstream : public IFreeImageIO
{
public:
	CFreeImageIOOstream(std::ostream & stream);

	unsigned Read(void * buffer, unsigned size, unsigned count) override;
	unsigned Write(void * buffer, unsigned size, unsigned count) override;
	int Seek(long offset, int origin) override;
	long Tell() override;

private:
	std::ostream & m_stream;
};

}
