#include "neuro.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

Neuron::Neuron(Basis::System* sys) :
	Basis::Entity(sys)
{

}

NeuroNet::NeuroNet(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto container = addFacet<Basis::Container>();
}

Neuro::Neuro(Basis::System* sys) :
	Basis::Entity(sys)
{
	// Создаём грань Executable для этого скетча
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&Neuro::step, this));
}

void Neuro::step()
{
	std::cout << "Neuro::step()" << endl;
}

void setup(Basis::System* s)
{
	std::cout << "Neuro::setup()" << endl;

	sys = s;
	sys->registerEntity<Neuron>();
	sys->registerEntity<NeuroNet>();
	sys->registerEntity<Neuro>();
	auto dummy = sys->container()->newEntity<Neuro>();
	dummy->setName("NeuroMaster");
}
