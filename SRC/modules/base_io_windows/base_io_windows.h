#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef BASE_IO_WINDOWS_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

class BaseIO : public Basis::Entity
{
public:
	BaseIO(Basis::System* s);
	void step();
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);