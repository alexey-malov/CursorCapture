// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
//#define WINVER		0x0500
//#define _WIN32_WINNT	0x0501
//#define _WIN32_IE	0x0501
//#define _RICHEDIT_VER	0x0500

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atltypes.h>
#include <atlgdi.h>
#include <atlcrack.h>

// This define is required to let boost algorithms and adaptors deduce result of functions and lambdas 
// using decltype() without need for explicit definition of function objects with result_type typedef
// See http://boost.2283326.n4.nabble.com/range-cannot-use-lambda-predicate-in-adaptor-with-certain-algorithms-td3560157.html
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <vector>
#include <algorithm>
#include <deque>
#include <chrono>
#include <map>
#include <unordered_map>

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/scope_exit.hpp>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/map.hpp>
#include <gsl/span>


#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
