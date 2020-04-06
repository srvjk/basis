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
#  define MODULE_EXPORT
#endif

enum class ViewingMode
{
	List,
	Graph
};

class Observer : public Basis::Entity
{
	struct Private;

public:
	Observer(Basis::System* s);
	~Observer();
	bool init() override;
	void cleanup() override;
	void step();

private:
	void showMainToolbar();
	void showListView();
	void showGraphView();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);