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

static const double PI = 3.141592653589793;
static const double PI2 = PI * 2;
static const double PIdiv2 = PI / 2;

/**
@brief “рехмерный вектор, заданный в пол€рных координатах.
*/
struct Polar
{
	Polar();
	Polar(double phi, double theta, double r = 1.0);

	double phi = 0.0;   // азимут
	double theta = 0.0; // высота
	double r = 1.0;     // длина
};

class NeuroViewer : public Basis::Entity
{
	struct Private;

public:
	NeuroViewer(Basis::System* s);
	~NeuroViewer();
	bool init() override;
	void cleanup() override;
	void step();

	/// ѕереместить камеру вправо-влево, не мен€€ направлени€ взгл€да.
	void rotateCameraAroundViewpointLR(double angle);
	/// ѕереместить камеру вверх-вних, не мен€€ направлени€ взгл€да.
	void rotateCameraAroundViewpointUD(double angle);
	/// ѕереместить камеру ближе или дальше от точки "фокуса" на рассто€ние dist км.
	void zoom(double dist);
	/// ќбработать событи€ клавиатуры.
	void processKeyboardEvents();

private:
	void showMainToolbar();
	void showStatistics();
	void showLessons();
	void drawScene();
	void drawActiveNet();

private:
	std::unique_ptr<Private> _p = nullptr;
};


extern "C" MODULE_EXPORT void setup(Basis::System* s);