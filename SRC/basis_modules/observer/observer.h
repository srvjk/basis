#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef OBSERVER_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define EXPORTED
#endif

class Observer : public Basis::Entity
{
public:
	Observer(Basis::System* s);
	void step();
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);