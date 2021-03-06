#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef DUMMY_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

class DummySketch : public Basis::Entity
{
public:
	DummySketch(Basis::System* s);
	void step();
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);