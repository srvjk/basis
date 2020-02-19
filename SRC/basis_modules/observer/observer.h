#pragma once

#include "basis.h"
#include <memory>

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
	struct Private;

public:
	Observer(Basis::System* s);
	~Observer();
	void step();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);