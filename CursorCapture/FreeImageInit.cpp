#include "StdAfx.h"
#include "FreeImageInit.h"
#include <boost/format.hpp>

//#define DEBUG_INIT
#ifdef DEBUG_INIT
#include "libhack/mutex.h"
#include "libhack/Static.h"
#endif

namespace freeimagecpp
{

#ifdef DEBUG_INIT

using hack::mutex;
using hack::lock_guard;

static int s_initCount;

static mutex & GetMutex()
{
	HACK_STATIC(mutex, s_mutex);
	return s_mutex;
};

#endif

static std::function<void(const std::string &)> g_errorHandler;

/**
FreeImage error handler
@param fif Format / Plugin responsible for the error
@param message Error message
*/
static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	try
	{
		const char *imageFormat = FreeImage_GetFormatFromFIF(fif);
		boost::format format("FreeImage error [%s Format] %s");
		std::string formattedMessage = (format % imageFormat % message).str();
		if (g_errorHandler)
		{
			g_errorHandler(formattedMessage);
		}
	}
	catch (...)
	{
	}
}

CFreeImageInit::CFreeImageInit()
{
#ifdef DEBUG_INIT
	lock_guard<mutex> lock(GetMutex());
	++s_initCount;
	assert(s_initCount == 1);
#endif
	FreeImage_Initialise(TRUE);
}

CFreeImageInit::~CFreeImageInit()
{
#ifdef DEBUG_INIT
	lock_guard<mutex> lock(GetMutex());
	--s_initCount;
	assert(s_initCount == 0);
#endif
	FreeImage_DeInitialise();
}

void CFreeImageInit::StartListenErrors(const std::function<void(const std::string &)> &handler)
{
	g_errorHandler = handler;
	FreeImage_SetOutputMessage(FreeImageErrorHandler);
}

}
