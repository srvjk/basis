#include "dummy.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

DummySketch::DummySketch(Basis::System* sys) :
	Basis::Entity(sys)
{
	// Создаём грань Executable для этого скетча
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&DummySketch::step, this));
}

void DummySketch::step()
{

}

void setup(Basis::System* s)
{
	std::cout << "DummySketch::setup()" << endl;

	sys = s;
	sys->registerEntity<DummySketch>();
	auto dummy = sys->container()->newEntity<DummySketch>();
	dummy->setName("MrDummy");
}
