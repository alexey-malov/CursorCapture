#pragma once

namespace freeimagecpp
{

class IFreeImageIO;

class CFreeImageIO : private FreeImageIO
{
public:
	friend class CFreeImage;
	friend class CFreeImageMulti;

	CFreeImageIO(IFreeImageIO & io);

	static unsigned Read(void * buffer, unsigned size, unsigned count, fi_handle handle);
	static unsigned Write(void *buffer, unsigned size, unsigned count, fi_handle handle);
	static int Seek(fi_handle handle, long offset, int origin);
	static long Tell(fi_handle handle);

	unsigned Read(void * buffer, unsigned size, unsigned count);
	unsigned Write(void * buffer, unsigned size, unsigned count);
	int Seek(long offset, int origin);
	long Tell();

	void RethrowException();

private:
	IFreeImageIO & m_io;
	std::exception_ptr m_pException;
};

}
