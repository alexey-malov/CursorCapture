#pragma once

#include "IFreeImageIO.h"

namespace freeimagecpp
{

class CFreeImageIOMemory : public IFreeImageIO
{
public:
	CFreeImageIOMemory(const std::vector<char> &data);
	CFreeImageIOMemory(const char *begin, size_t size);

	unsigned Read(void * buffer, unsigned size, unsigned count) override;
	unsigned Write(void * buffer, unsigned size, unsigned count) override;
	int Seek(long offset, int origin) override;
	long Tell() override;

private:
	const char *m_data = nullptr;
	size_t m_size = 0;
	size_t m_position = 0;
};

}
