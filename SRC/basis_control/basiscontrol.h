#pragma once

#include "basis.h"
#include <memory>

#ifdef PLATFORM_WINDOWS
#  ifdef BASISCONTROL_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

class BasisControl : public Basis::Entity
{
	struct Private;

public:
	BasisControl(Basis::System* s);
	~BasisControl();
	void step();

private:
	void showMainToolbar();

private:
	std::unique_ptr<Private> _p = nullptr;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);
