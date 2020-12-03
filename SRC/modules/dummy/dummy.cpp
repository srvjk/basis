#include "dummy.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

DummySketch::DummySketch(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&DummySketch::step, this));
}

void DummySketch::step()
{
	std::cout << "DummySketch::step()" << endl;
}

void setup(Basis::System* s)
{
	std::cout << "DummySketch::setup()" << endl;

	sys = s;
	sys->registerEntity<DummySketch>();
}
