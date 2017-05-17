#include "stdafx.h"
#include "FreeImageIOMemory.h"

namespace freeimagecpp
{

CFreeImageIOMemory::CFreeImageIOMemory(const std::vector<char> &data)
	: m_data(data.data())
	, m_size(data.size())
{
}

CFreeImageIOMemory::CFreeImageIOMemory(const char *begin, size_t size)
	: m_data(begin)
	, m_size(size)
{
}

unsigned CFreeImageIOMemory::Read(void * buffer, unsigned size, unsigned count)
{
	if (m_size <= m_position)
	{
		return 0;
	}

	const size_t readItemsCount = (std::min)((m_size - m_position) / size, size_t(count));
	const size_t readBytesCount = readItemsCount * size;

	std::memcpy(buffer, m_data + m_position, readBytesCount);
	m_position += readBytesCount;

	return unsigned(readItemsCount);
}

unsigned CFreeImageIOMemory::Write(void * buffer, unsigned size, unsigned count)
{
	(void)buffer;
	(void)size;
	(void)count;
	assert(false);
	throw std::logic_error("can't write read-only memory");
}

int CFreeImageIOMemory::Seek(long offset, int origin)
{
	size_t position = 0;
	switch (origin)
	{
	case SEEK_SET:
		position = size_t(offset);
		break;
	case SEEK_CUR:
		position = size_t(m_position + offset);
		break;
	case SEEK_END:
		position = size_t(m_size + offset);
		break;
	default:
		throw std::runtime_error("invalid seek origin");
	}
	if (position <= m_size)
	{
		m_position = position;
		return 0;
	}
	return -1;
}

long CFreeImageIOMemory::Tell()
{
	return long(m_position);
}

}
