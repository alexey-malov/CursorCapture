#include "StdAfx.h"
#include "FreeImageIOOstream.h"

namespace freeimagecpp
{

CFreeImageIOOstream::CFreeImageIOOstream(std::ostream & stream)
	:m_stream(stream)
{
}

unsigned CFreeImageIOOstream::Read(void * /*buffer*/, unsigned /*size*/, unsigned /*count*/)
{
	assert(false);
	throw std::logic_error("can't read ostream");
}

unsigned CFreeImageIOOstream::Write(void * buffer, unsigned size, unsigned count)
{
	std::streamoff posBefore = m_stream.tellp();
	if (posBefore == -1)
	{
		throw std::runtime_error("get before write position");
	}

	m_stream.write((char *)buffer, size * count);

	std::streamoff posAfter = m_stream.tellp();
	if (posAfter == -1)
	{
		throw std::runtime_error("get after write position");
	}

	return boost::numeric_cast<unsigned>((posAfter - posBefore) / size);
}

int CFreeImageIOOstream::Seek(long offset, int origin)
{
	std::ios_base::seekdir dir;

	switch (origin)
	{
	case SEEK_SET:
		dir = std::ios_base::beg;
		break;
	case SEEK_CUR:
		dir = std::ios_base::cur;
		break;
	case SEEK_END:
		dir = std::ios_base::end;
		break;
	default:
		throw std::runtime_error("invalid seek origin");
	}

	m_stream.seekp(offset, dir);
	if (!m_stream)
	{
		throw std::runtime_error("stream seek");
	}

	return 0;
}

long CFreeImageIOOstream::Tell()
{
	std::streamoff pos = m_stream.tellp();
	if (pos == -1)
	{
		throw std::runtime_error("get stream position");
	}
	return boost::numeric_cast<long>(pos);
}

}
