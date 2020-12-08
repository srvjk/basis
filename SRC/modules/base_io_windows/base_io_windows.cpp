#include "base_io_windows.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

BaseIO::BaseIO(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&BaseIO::step, this));
}

void BaseIO::step()
{
	std::cout << "BaseIO::step()" << endl;
}

void setup(Basis::System* s)
{
	std::cout << "BaseIO::setup()" << endl;

	sys = s;
	sys->registerEntity<BaseIO>();
}
