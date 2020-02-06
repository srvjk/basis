#include "observer.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

Observer::Observer(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&Observer::step, this));
}

void Observer::step()
{
	std::cout << "Observer::step()" << endl;
}

void setup(Basis::System* s)
{
	std::cout << "Observer::setup()" << endl;

	sys = s;
	sys->registerEntity<Observer>();
	sys->container()->newEntity<Observer>();
}

