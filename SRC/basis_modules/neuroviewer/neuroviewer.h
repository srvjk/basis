#pragma once

#include "basis.h"
#include <memory>

#ifdef PLATFORM_WINDOWS
#  ifdef NEUROVIEWER_LIB
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

struct GLFWwindow;

class NeuroViewer : public Basis::Entity
{
	struct Private;

public:
	NeuroViewer(Basis::System* s);
	~NeuroViewer();
	bool init() override;
	void cleanup() override;
	void step();

private:
	void showMainToolbar();
	void drawScene();
	void drawActiveNet();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);