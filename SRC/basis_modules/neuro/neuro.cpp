#include "neuro.h"
#include <iostream>

using namespace std;

static Basis::System* sys = nullptr;

Neuron::Neuron(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto spatial = addFacet<Basis::Spatial>();
}

void Neuron::setValue(double v)
{
	_value = v;
}

double Neuron::value() const
{
	return _value;
}

NeuroNet::NeuroNet(Basis::System* sys) :
	Basis::Entity(sys)
{
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

	// создаём нейросеть-классификатор:
	auto net = sys->newEntity<NeuroNet>();
	if (!net)
		return false;

	net->setName("SimplisticNeuralClassifier");

	double spacing = 10.0;

	// создаём входной слой
	int inLayerSize = 10;
	for (int i = 0; i < inLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 0.0 });
	}

	// создаём внутренний слой
	int midLayerSize = 20;
	for (int i = 0; i < midLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 1 * spacing });
	}

	// создаём выходной слой
	int outLayerSize = 2;
	for (int i = 0; i < outLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 2 * spacing });
	}

	// создаём Тренера
	auto trainer = sys->newEntity<Trainer>();
	if (trainer) {
		trainer->setName("Trainer");
		trainer->setNet(net);
	}

	return true;
}

void SimplisticNeuralClassification::step()
{
	auto trainers = sys->entityCollection<Trainer>();
	if (trainers.empty())
		return;

	auto trainer = trainers[0];
	if (trainer->isActive())
		trainer->train();
}

void SimplisticNeuralClassification::cleanup()
{
	std::cout << "SimplisticNeuralClassification::cleanup()" << endl;
}

struct Trainer::Private 
{
	bool active = false;
	shared_ptr<NeuroNet> net = nullptr;
};

Trainer::Trainer(Basis::System* s) :
	Basis::Entity(s), _p(make_unique<Private>())
{
}
 
bool Trainer::isActive() const
{
	return _p->active;
}

void Trainer::setActive(bool active)
{
	_p->active = active;
}

void Trainer::setNet(shared_ptr<NeuroNet> net)
{
	_p->net = net;
}

void Trainer::train()
{
	//if (!_p->net)
	//	return;

	//auto iter = _p->net->facets<Basis::Container>();
	//if (!iter)
	//	return;

	//auto cont = static_pointer_cast<Basis::Container>(iter->value());
 //	for (auto neurPtr = cont->instances(TYPEID(Neuron)); neurPtr->finished() == false; neurPtr->next()) {
	//	auto neuron = static_pointer_cast<Neuron>(neurPtr->value());

	//	auto spatial = static_pointer_cast<Basis::Spatial>(neuron->facets()->value());
	//	Basis::point3d pt = spatial->position();
	//	if (pt.get<2>() == 0) { // входной слой
	//		neuron->setValue(1);
	//	}
	//}
}

void setup(Basis::System* s)
{
	std::cout << "Neuro::setup()" << endl;

	sys = s;
	sys->registerEntity<Neuron>();
	sys->registerEntity<NeuroNet>();
	sys->registerEntity<SimplisticNeuralClassification>();
	sys->registerEntity<Trainer>();
	//simpNeuroClassif->setName("SimplisticNeuralClassification");
}
