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
@brief ���������� ������, �������� � �������� �����������.
*/
struct Polar
{
	Polar();
	Polar(double phi, double theta, double r = 1.0);

	double phi = 0.0;   // ������
	double theta = 0.0; // ������
	double r = 1.0;     // �����
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

	/// ����������� ������ ������-�����, �� ����� ����������� �������.
	void rotateCameraAroundViewpointLR(double angle);
	/// ����������� ������ �����-����, �� ����� ����������� �������.
	void rotateCameraAroundViewpointUD(double angle);
	/// ����������� ������ ����� ��� ������ �� ����� "������" �� ���������� dist ��.
	void zoom(double dist);
	/// ���������� ������� ����������.
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