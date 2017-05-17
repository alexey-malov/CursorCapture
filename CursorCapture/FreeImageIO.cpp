#include "StdAfx.h"
#include "FreeImageIO.h"
#include "IFreeImageIO.h"

namespace freeimagecpp
{

CFreeImageIO::CFreeImageIO(IFreeImageIO & io)
	:m_io(io)
{
	read_proc = Read;
	write_proc = Write;
	seek_proc = Seek;
	tell_proc = Tell;
}

unsigned CFreeImageIO::Read(void * buffer, unsigned size, unsigned count, fi_handle handle)
{
	return static_cast<CFreeImageIO *>(handle)->Read(buffer, size, count);
}

unsigned CFreeImageIO::Read(void * buffer, unsigned size, unsigned count)
{
	try
	{
		return m_io.Read(buffer, size, count);
	}
	catch (...)
	{
		m_pException = std::current_exception();
		return 0;
	}
}

unsigned CFreeImageIO::Write(void *buffer, unsigned size, unsigned count, fi_handle handle)
{
	return static_cast<CFreeImageIO *>(handle)->Write(buffer, size, count);
}

unsigned CFreeImageIO::Write(void * buffer, unsigned size, unsigned count)
{
	try
	{
		return m_io.Write(buffer, size, count);
	}
	catch (...)
	{
		m_pException = std::current_exception();
		return 0;
	}
}

int CFreeImageIO::Seek(fi_handle handle, long offset, int origin)
{
	return static_cast<CFreeImageIO *>(handle)->Seek(offset, origin);
}

int CFreeImageIO::Seek(long offset, int origin)
{
	try
	{
		return m_io.Seek(offset, origin);
	}
	catch (...)
	{
		m_pException = std::current_exception();
		return -1;
	}
}

long CFreeImageIO::Tell(fi_handle handle)
{
	return static_cast<CFreeImageIO *>(handle)->Tell();
}

long CFreeImageIO::Tell()
{
	try
	{
		return m_io.Tell();
	}
	catch (...)
	{
		m_pException = std::current_exception();
		return -1;
	}
}

void CFreeImageIO::RethrowException()
{
	if (!(m_pException == NULL))
	{
		std::rethrow_exception(m_pException);
	}
}

}
