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
	// ������ ����� Executable ��� ����� ������
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&SimplisticNeuralClassification::step, this));
}

bool SimplisticNeuralClassification::init()
{
	std::cout << "SimplisticNeuralClassification::init()" << endl;

	// ������ ���������-�������������:
	auto net = sys->container()->newEntity<NeuroNet>();
	if (!net)
		return false;

	net->setName("SimplisticNeuralClassifier");

	// ��������� ���� ���������;
	// ������� � ����� ������ ����������� � ����� ���������� �������.
	auto iter = net->facets(TYPEID(Basis::Container));
	if (!iter)
		return false;

	Basis::Container* cont = Basis::eCast<Basis::Container>(iter->value());
	if (!cont)
		return false;

	// ������ ������� ����
	int inLayerSize = 10;
	for (int i = 0; i < inLayerSize; ++i) {
		auto neuron = cont->newEntity<Neuron>();
		auto fcts = neuron->facets(TYPEID(Basis::Spatial));
		if (!fcts)
			continue;
		Basis::Spatial* spt = Basis::eCast<Basis::Spatial>(fcts->value());
		if (spt)
			spt->setPosition({ (double)i, (double)0, (double)0 });
	}

	// ������ ���������� ����
	int midLayerSize = 20;
	for (int i = 0; i < midLayerSize; ++i) {
		auto neuron = cont->newEntity<Neuron>();
		auto fcts = neuron->facets(TYPEID(Basis::Spatial));
		if (!fcts)
			continue;
		Basis::Spatial* spt = Basis::eCast<Basis::Spatial>(fcts->value());
		if (spt)
			spt->setPosition({ (double)i, (double)0, (double)1 });
	}

	// ������ �������� ����
	int outLayerSize = 2;
	for (int i = 0; i < outLayerSize; ++i) {
		auto neuron = cont->newEntity<Neuron>();
		auto fcts = neuron->facets(TYPEID(Basis::Spatial));
		if (!fcts)
			continue;
		Basis::Spatial* spt = Basis::eCast<Basis::Spatial>(fcts->value());
		if (spt)
			spt->setPosition({ (double)i, (double)0, (double)2 });
	}

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