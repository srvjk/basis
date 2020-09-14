#pragma once

#include "basis.h"
#include <boost/system/system_error.hpp>

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
	void readDataFromController();
	double getDoubleParam(const std::string& name, bool* ok = nullptr) const;
	int32_t getInt32Param(const std::string& name, bool* ok = nullptr) const;
	bool setDoubleParam(const std::string& name, double val);
	bool setInt32Param(const std::string& name, int32_t val);
	void switchFilter(bool on = true);
	bool isFilterOn() const;

private:
	void serialWorker();
	void readHandler(const boost::system::error_code& e, std::size_t size);

private:
	std::unique_ptr<Private> _p;
};

class MODULE_EXPORT AquaViewer : public Basis::Entity
{
	struct Private;

public:
	AquaViewer(Basis::System* s);
	void step();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);