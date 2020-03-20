#include "neuro.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

Neuron::Neuron(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto spatial = addFacet<Basis::Spatial>();
}

NeuroNet::NeuroNet(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto container = addFacet<Basis::Container>();
}

SimplisticNeuralClassification::SimplisticNeuralClassification(Basis::System* sys) :
	Basis::Entity(sys)
{
	// Создаём грань Executable для этого скетча
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&SimplisticNeuralClassification::step, this));
}

bool SimplisticNeuralClassification::init()
{
	std::cout << "SimplisticNeuralClassification::init()" << endl;

	//// создаём нейросеть-классификатор:
	//auto net = sys->container()->newEntity<NeuroNet>();
	//if (!net)
	//	return false;

	//net->setName("SimplisticNeuralClassifier");

	//// заполняем сеть нейронами;
	//// нейроны в общем случае размещаются в узлах трехмерной решетки.
	//auto cont = static_pointer_cast<Basis::Container>(net->facets(TYPEID(Basis::Container)).front());
	//if (!cont)
	//	return false;

	//// создаём входной слой
	//int inLayerSize = 10;
	//for (int i = 0; i < inLayerSize; ++i) {
	//	auto neuron = cont->newEntity<Neuron>();
	//	auto facets = neuron->facets(TYPEID(Basis::Spatial));
	//	if (facets.empty())
	//		continue;
	//	auto spatial = static_pointer_cast<Basis::Spatial>(facets.at(0));
	//	if (spatial)
	//		spatial->setPosition({ (double)i, (double)0, (double)0 });
	//}

	//// создаём внутренний слой
	//int midLayerSize = 20;
	//for (int i = 0; i < midLayerSize; ++i) {
	//	auto neuron = cont->newEntity<Neuron>();
	//	auto facets = neuron->facets(TYPEID(Basis::Spatial));
	//	if (facets.empty())
	//		continue;
	//	auto spatial = static_pointer_cast<Basis::Spatial>(facets.at(0));
	//	if (spatial)
	//		spatial->setPosition({ (double)i, (double)0, (double)1 });
	//}

	//// создаём выходной слой
	//int outLayerSize = 2;
	//for (int i = 0; i < outLayerSize; ++i) {
	//	auto neuron = cont->newEntity<Neuron>();
	//	auto facets = neuron->facets(TYPEID(Basis::Spatial));
	//	if (facets.empty())
	//		continue;
	//	auto spatial = static_pointer_cast<Basis::Spatial>(facets.at(0));
	//	if (spatial)
	//		spatial->setPosition({ (double)i, (double)0, (double)2 });
	//}

	return true;
}

void SimplisticNeuralClassification::step()
{
	std::cout << "SimplisticNeuralClassification::step()" << endl;

}

void SimplisticNeuralClassification::cleanup()
{
	std::cout << "SimplisticNeuralClassification::cleanup()" << endl;
}

void setup(Basis::System* s)
{
	std::cout << "Neuro::setup()" << endl;

	sys = s;
	sys->registerEntity<Neuron>();
	sys->registerEntity<NeuroNet>();
	sys->registerEntity<SimplisticNeuralClassification>();
	auto simpNeuroClassif = sys->container()->newEntity<SimplisticNeuralClassification>();
	simpNeuroClassif->setName("SimplisticNeuralClassification");
}
