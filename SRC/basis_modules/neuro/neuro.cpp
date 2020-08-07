#include "neuro.h"
#include <iostream>
#include <boost/format.hpp>

using namespace std;

static Basis::System* sys = nullptr;

Link::Link(Basis::System* s) :
	Basis::Entity(sys)
{

}

Neuron::Neuron(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto spatial = addFacet<Basis::Spatial>();
}

void Neuron::setValue(double v)
{
	int iNewVal = (int)v;
	int iOldVal = (int)_value;

	if (iNewVal != iOldVal)
		_activityChangedTimeStamp = sys->stepsFromStart(); // remember when activity changed

	_value = v;
}

double Neuron::value() const
{
	return _value;
}

bool Neuron::isActive() const
{
	return ((int)_value == 1);
}

int64_t Neuron::stateChangedTimeStamp() const
{
	return _activityChangedTimeStamp;
}

Layer::Layer(Basis::System* sys) :
	Basis::Entity(sys)
{
}

struct NeuroNet::Private
{
	map<string, shared_ptr<Neuron>> indexByName;
	bool spontaneousActivityOn = true; // switch spontaneous neuron activity on/off
};

NeuroNet::NeuroNet(Basis::System* sys) :
	Basis::Entity(sys), _p(make_unique<Private>())
{
}

void NeuroNet::rememberNeuronByName(std::shared_ptr<Neuron> neuron, const std::string& name)
{
	if (_p->indexByName.count(name) > 0)
		return; // key already exists

	_p->indexByName.insert(std::make_pair<>(name, neuron));
}

std::shared_ptr<Neuron> NeuroNet::recallNeuronByName(const std::string& name)
{
	auto it = _p->indexByName.find(name);
	if (it != _p->indexByName.end())
		return it->second;

	return nullptr;
}

void NeuroNet::tick()
{
	// first we activate neurons:
	for (auto entIter = entityIterator(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto layer = ent->as<Layer>();
		if (!layer)
			continue;

		if (layer->name() == "MidLayer") {
			for (int i = 0; i < layer->neurons.size(); ++i) {
				auto neuron = layer->neurons[i];
				if (neuron->isActive()) {
					// если нейрон активен, смотрим, не пора ли ему деактивироваться
					int64_t currentTime = sys->stepsFromStart();
					int64_t delta = currentTime - neuron->stateChangedTimeStamp();
					if (delta > 100)
						neuron->setValue(0.0);
				}
				else {
					// если нейрон неактивен, смотрим, не пора ли его активировать
					int64_t currentTime = sys->stepsFromStart();
					int64_t delta = currentTime - neuron->stateChangedTimeStamp();
					if (delta > 100) { // нейрон может активироваться, только если прошло некоторое минимальное время с момента последней активации
						if (_p->spontaneousActivityOn) {
							int topVal = 1000000;
							float strikeProb = 0.01;
							int upVal = (int)(strikeProb * topVal);
							int randVal = sys->randomInt(0, topVal);
							if (randVal < upVal) {
								neuron->setValue(1.0);
							}
						}
					}
				}
			}
		}
	}
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

	double spacing = 30.0;

	// создаём входной слой
	auto layer = net->newEntity<Layer>();
	layer->setName("InLayer");
	
	int inLayerSize = 10;
	for (int i = 0; i < inLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		std::string name = (boost::format("in.%d") % i).str();
		neuron->setName(name);
		net->rememberNeuronByName(neuron, name); // добавляем нейрон в индексную карту по имени
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 0.0 });

		// добавляем ссылку на нейрон в слой для удобства
		layer->neurons.push_back(neuron);
	}

	// создаём внутренний слой
	layer = net->newEntity<Layer>();
	layer->setName("MidLayer");

	int midLayerSize = 20;
	for (int i = 0; i < midLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		std::string name = (boost::format("mid.%d") % i).str();
		neuron->setName(name);
		net->rememberNeuronByName(neuron, name); // добавляем нейрон в индексную карту по имени
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 1 * spacing });

		// добавляем ссылку на нейрон в слой для удобства
		layer->neurons.push_back(neuron);
	}

	// создаём выходной слой
	layer = net->newEntity<Layer>();
	layer->setName("OutLayer");

	int outLayerSize = 2;
	for (int i = 0; i < outLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		std::string name = (boost::format("out.%d") % i).str();
		neuron->setName(name);
		net->rememberNeuronByName(neuron, name); // добавляем нейрон в индексную карту по имени

		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 2 * spacing });

		// добавляем ссылку на нейрон в слой для удобства
		layer->neurons.push_back(neuron);
	}

	// создаем прямые связи от входного слоя к промежуточному
	for (int i = 0; i < inLayerSize; ++i) {
		string srcName = (boost::format("in.%d") % i).str();
		shared_ptr<Neuron> srcNeuron = net->recallNeuronByName(srcName);
		if (!srcNeuron)
			continue;

		for (int j = 0; j < midLayerSize; ++j) {
			string dstName = (boost::format("mid.%d") % j).str();
			shared_ptr<Neuron> dstNeuron = net->recallNeuronByName(dstName);
			if (!dstNeuron)
				continue;

			auto link = net->newEntity<Link>();
			if (link) {
				link->srcNeuron = srcNeuron;
				link->dstNeuron = dstNeuron;
			}
		}
	}

	// создаем прямые связи от промежуточного слоя к выходному
	for (int i = 0; i < midLayerSize; ++i) {
		string srcName = (boost::format("mid.%d") % i).str();
		shared_ptr<Neuron> srcNeuron = net->recallNeuronByName(srcName);
		if (!srcNeuron)
			continue;

		for (int j = 0; j < outLayerSize; ++j) {
			string dstName = (boost::format("out.%d") % j).str();
			shared_ptr<Neuron> dstNeuron = net->recallNeuronByName(dstName);
			if (!dstNeuron)
				continue;

			auto link = net->newEntity<Link>();
			if (link) {
				link->srcNeuron = srcNeuron;
				link->dstNeuron = dstNeuron;
			}
		}
	}

	// создаём тренеров
	auto trainer = sys->newEntity<Trainer>();
	if (trainer) {
		trainer->setName("Trainer1");
		trainer->setNet(net);
	}

	return true;
}

void SimplisticNeuralClassification::step()
{
	for (auto entIter = sys->entityIterator(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();

		auto trainer = ent->as<Trainer>();
		if (trainer) {
			trainer->train();
		}

		auto net = ent->as<NeuroNet>();
		if (net) {
			net->tick();
		}
	}
}

void SimplisticNeuralClassification::cleanup()
{
	std::cout << "SimplisticNeuralClassification::cleanup()" << endl;
}

struct Trainer::Private 
{
	shared_ptr<NeuroNet> net = nullptr;
	map<string, function<void()>> lessons;
	string activeLesson;
};

Trainer::Trainer(Basis::System* s) :
	Basis::Entity(s), _p(make_unique<Private>())
{
	std::function<void()> lesson1 = [this]() -> void {
		auto net = getNet();
		if (!net)
			return;

		// Урок 1.
		// Когда активна первая половина нейронов входного слоя, а вторая неактивна, должен активизироваться
		// первый выходной нейрон, в противном случае - второй.
		for (auto entIter = net->entityIterator(); entIter.hasMore(); entIter.next()) {
			auto ent = entIter.value();
			auto layer = ent->as<Layer>();
			if (!layer)
				continue;

			if (layer->name() == "InLayer") {
				int halfSize = layer->neurons.size() / 2;
				for (int i = 0; i < layer->neurons.size(); ++i) {
					auto neuron = layer->neurons[i];
					if (i < halfSize)
						neuron->setValue(1);
					else
						neuron->setValue(0);
				}
			}

			if (layer->name() == "OutLayer") {
				for (int i = 0; i < layer->neurons.size(); ++i) {
					auto neuron = layer->neurons[i];
					if (i == 0)
						neuron->setValue(1);
					else
						neuron->setValue(0);
				}
			}
		}
	};
	_p->lessons["Lesson1"] = lesson1;

	std::function<void()> lesson2 = [this]() -> void {
		auto net = getNet();
		if (!net)
			return;

		// Урок 2.
		// Когда активна вторая половина нейронов входного слоя, а первая неактивна, должен активизироваться
		// второй выходной нейрон, в противном случае - первый.
		for (auto entIter = net->entityIterator(); entIter.hasMore(); entIter.next()) {
			auto ent = entIter.value();
			auto layer = ent->as<Layer>();
			if (!layer)
				continue;

			if (layer->name() == "InLayer") {
				int halfSize = layer->neurons.size() / 2;
				for (int i = 0; i < layer->neurons.size(); ++i) {
					auto neuron = layer->neurons[i];
					if (i < halfSize)
						neuron->setValue(0);
					else
						neuron->setValue(1);
				}
			}

			if (layer->name() == "OutLayer") {
				for (int i = 0; i < layer->neurons.size(); ++i) {
					auto neuron = layer->neurons[i];
					if (i == 0)
						neuron->setValue(0);
					else
						neuron->setValue(1);
				}
			}
		}
	};
	_p->lessons["Lesson2"] = lesson2;
}
 
void Trainer::setNet(shared_ptr<NeuroNet> net)
{
	_p->net = net;
}

std::shared_ptr<NeuroNet> Trainer::getNet() const
{
	return _p->net;
}

void Trainer::train()
{
	for (auto iter : _p->lessons) {
		if (iter.first == _p->activeLesson) {
			std::function<void()> lesson = iter.second;
			lesson();
			break;
		}
	}
}

std::list<std::string> Trainer::listLessons() const
{
	std::list<std::string> res;

	for (auto iter : _p->lessons)
		res.push_back(iter.first);

	return res;
}

std::string Trainer::activeLesson() const
{
	return _p->activeLesson;
}

void Trainer::setActiveLesson(const std::string& lesson)
{
	_p->activeLesson = lesson;
}

void setup(Basis::System* s)
{
	std::cout << "Neuro::setup()" << endl;

	sys = s;
	sys->registerEntity<Link>();
	sys->registerEntity<Neuron>();
	sys->registerEntity<Layer>();
	sys->registerEntity<NeuroNet>();
	sys->registerEntity<SimplisticNeuralClassification>();
	sys->registerEntity<Trainer>();
	//simpNeuroClassif->setName("SimplisticNeuralClassification");
}
