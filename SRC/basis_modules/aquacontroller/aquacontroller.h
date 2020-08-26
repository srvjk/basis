#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef AQUACONTROLLER_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

class MODULE_EXPORT AquaController : public Basis::Entity
{
	class Private;

public:
	AquaController(Basis::System* s);
	void step();
	void reset();
	void operate();
	double getDoubleParam(const std::string& name, bool* ok = nullptr);

private:
	std::unique_ptr<Private> _p;
};

class MODULE_EXPORT AquaViewer : public Basis::Entity
{
	class Private;

public:
	AquaViewer(Basis::System* s);
	void step();

private:
	void showInfoPanel();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);