#include "neuro.h"
#include <iostream>
#include <boost/format.hpp>

using namespace std;
using namespace Basis;

static Basis::System* sys = nullptr;

Link::Link(Basis::System* s) :
	Basis::Entity(sys)
{

}

Neuron::Neuron(Basis::System* sys) :
	Basis::Entity(sys)
{
	auto spatial = addFacet<Basis::Spatial>();
	_activationThreshold = 5.0; // TODO ��������!!!
}

void Neuron::addInValue(double v)
{
	_inValue += v;
}

void Neuron::setInValue(double v)
{
	_inValue = v;
}

void Neuron::setOutValue(double v)
{
	int iNewVal = (int)v;
	int iOldVal = (int)_outValue;

	if (iNewVal != iOldVal)
		_activityChangedTimeStamp = sys->stepsFromStart(); // remember when activity changed

	_outValue = v;
}

double Neuron::inValue() const
{
	return _inValue;
}

double Neuron::outValue() const
{
	return _outValue;
}

double Neuron::activationThreshold() const
{
	return _activationThreshold;
}

bool Neuron::isActive() const
{
	return ((int)_outValue == 1);
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
	float growthFactorDelta = 0.01;
	double burstDuration = 100; // ������������ "�������" �������
	double restDuration = 100; // ������������ �����, � ������� ������� ������ "��������" ����� �������

	// �������� �� ������, ��������� �������, ���� ����� ���������
	for (auto entIter = entityIterator(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto link = ent->as<Link>();
		if (!link)
			continue;

		// ��������� ������� �������� �������, � �������� ����� ������ �����
		if (link->active) {
			if (link->srcNeuron->isActive()) {
				double dv = (link->type == LinkType::Positive ? 1.0 : -1.0);
				link->dstNeuron->addInValue(dv);
			}
		}

		int score = 0;
		if (link->srcNeuron->isActive())
			++score;
		if (link->dstNeuron->isActive())
			++score;

		if (link->type == LinkType::Positive) {
			switch (score) {
			case 1:
				// ���� ������� ������ ���� �� ���� ��������, ����� ����� ���� ������ ����������
				link->growthFactor -= growthFactorDelta;
				break;
			case 2:
				// ���� ��� ������� ������� ������������, ����� ����� ���� ������ �����������
				link->growthFactor += growthFactorDelta;
				break;
			default:
				break;
			}
		}

		// ��� ���������� ������ �� ��������:
		if (link->type == LinkType::Negative) {
			switch (score) {
			case 1:
				// ���� ������� ������ ���� �� ���� ��������, ����� ����� ���� ������ �����������
				link->growthFactor += growthFactorDelta;
				break;
			case 2:
				// ���� ��� ������� ������� ������������, ����� ����� ���� ������ ����������
				link->growthFactor -= growthFactorDelta;
				break;
			default:
				break;
			}
		}

		// ������ ����� ���������� ���������� ���������� [0.0; 1.0]
		if (link->growthFactor < 0.0)
			link->growthFactor = 0.0;
		if (link->growthFactor > 1.0)
			link->growthFactor = 1.0;

		// ���������� ��� ������������ ����� � ����������� �� �������� ������� �����:
		if (link->growthFactor > 0.9)
			link->active = true;
		if (link->growthFactor < 0.1)
			link->active = false;
	}

	// ���������� �������; ��� �������� ���������� ����� ������������ �� ��������� ����
	for (auto entIter = entityIterator(); entIter.hasMore(); entIter.next()) {
		auto ent = entIter.value();
		auto layer = ent->as<Layer>();
		if (!layer)
			continue;

		//if (layer->name() == "MidLayer") {
			for (int i = 0; i < layer->neurons.size(); ++i) {
				auto neuron = layer->neurons[i];
				if (neuron->isActive()) {
					// ���� ������ �������, �������, �� ���� �� ��� ����������������
					int64_t currentTime = sys->stepsFromStart();
					int64_t delta = currentTime - neuron->stateChangedTimeStamp();
					if (delta > burstDuration)
						neuron->setOutValue(0.0);
				}
				else {
					// ���� ������ ���������, �������, �� ���� �� ��� ������������
					int64_t currentTime = sys->stepsFromStart();
					int64_t delta = currentTime - neuron->stateChangedTimeStamp();
					if (delta > restDuration) { // ������ ����� ��������������, ������ ���� ������ ��������� ����������� ����� � ������� ��������� ���������
						// ������ ���� ��������� ����������� ���������� ��������� �������:
						if (_p->spontaneousActivityOn) {
							int topVal = 1000000;
							float strikeProb = 0.01;
							int upVal = (int)(strikeProb * topVal);
							int randVal = sys->randomInt(0, topVal);
							if (randVal < upVal) {
								neuron->setOutValue(1.0);
							}
						}

						// "����������" ��������� �� ����� ������� ��������:
						if (neuron->inValue() > neuron->activationThreshold())
							neuron->setOutValue(1.0);

						// �������� ������� �������� �������, ����� ��� ����� ����������� ������:
						neuron->setInValue(0.0);
					}
				}
			}
		//}
	}
}

SimplisticNeuralClassification::SimplisticNeuralClassification(Basis::System* sys) :
	Basis::Entity(sys)
{
	// ������ ����� Executable ��� ����� ������
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&SimplisticNeuralClassification::step, this));
}

void makePolyLineLink(Neuron* n1, Neuron* n2, std::vector<point3d>& result)
{
	auto spt1 = n1->as<Basis::Spatial>();
	auto spt2 = n2->as<Basis::Spatial>();
	Basis::point3d p1 = spt1->position();
	Basis::point3d p2 = spt2->position();
	Basis::point3d vect = p2 - p1;
	double len = length(vect);
	double dd = 3.0; // ���������� ����� �������
	int inptcnt = len / dd; // ���������� �����
	double dLen = 1.0 / inptcnt;
	double dev = len / inptcnt;
	double l = 0.0;
	for (int t = 0; t < inptcnt; ++t) {
		point3d randPoint;
		randPoint.set<0>(n1->system()->randomDouble(0.0, dev));
		randPoint.set<1>(n1->system()->randomDouble(0.0, dev));
		randPoint.set<2>(n1->system()->randomDouble(0.0, dev));
		point3d ipt = p1 + vect * l + randPoint;
		result.push_back(ipt);
		l += dLen;
	}
	result.push_back(p2);
}

bool SimplisticNeuralClassification::init()
{
	std::cout << "SimplisticNeuralClassification::init()" << endl;

	// ������ ���������-�������������:
	auto net = sys->newEntity<NeuroNet>();
	if (!net)
		return false;

	net->setName("SimplisticNeuralClassifier");

	double spacing = 30.0;

	// ������ ������� ����
	auto layer = net->newEntity<Layer>();
	layer->setName("InLayer");
	
	int inLayerSize = 10;
	for (int i = 0; i < inLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		std::string name = (boost::format("in.%d") % i).str();
		neuron->setName(name);
		net->rememberNeuronByName(neuron, name); // ��������� ������ � ��������� ����� �� �����
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 0.0 });

		// ��������� ������ �� ������ � ���� ��� ��������
		layer->neurons.push_back(neuron);
	}

	// ������ ���������� ����
	layer = net->newEntity<Layer>();
	layer->setName("MidLayer");

	int midLayerSize = 20;
	for (int i = 0; i < midLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		std::string name = (boost::format("mid.%d") % i).str();
		neuron->setName(name);
		net->rememberNeuronByName(neuron, name); // ��������� ������ � ��������� ����� �� �����
		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 1 * spacing });

		// ��������� ������ �� ������ � ���� ��� ��������
		layer->neurons.push_back(neuron);
	}

	// ������ �������� ����
	layer = net->newEntity<Layer>();
	layer->setName("OutLayer");

	int outLayerSize = 2;
	for (int i = 0; i < outLayerSize; ++i) {
		auto neuron = net->newEntity<Neuron>();
		std::string name = (boost::format("out.%d") % i).str();
		neuron->setName(name);
		net->rememberNeuronByName(neuron, name); // ��������� ������ � ��������� ����� �� �����

		auto spt = neuron->as<Basis::Spatial>();
		if (spt)
			spt->setPosition({ i * spacing, 0.0, 2 * spacing });

		// ��������� ������ �� ������ � ���� ��� ��������
		layer->neurons.push_back(neuron);
	}

	// ������� ������ ����� �� �������� ���� � ��������������
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

			// ������������� �����:
			auto link = net->newEntity<Link>();
			if (link) {
				link->type = LinkType::Positive;
				link->srcNeuron = srcNeuron;
				link->dstNeuron = dstNeuron;

				makePolyLineLink(srcNeuron.get(), dstNeuron.get(), link->path);
			}
			// ������������� �����:
			link = net->newEntity<Link>();
			if (link) {
				link->type = LinkType::Negative;
				link->srcNeuron = srcNeuron;
				link->dstNeuron = dstNeuron;

				makePolyLineLink(srcNeuron.get(), dstNeuron.get(), link->path);
			}
		}
	}

	// ������� ������ ����� �� �������������� ���� � ���������
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

			// ������������� �����:
			auto link = net->newEntity<Link>();
			if (link) {
				link->type = LinkType::Positive;
				link->srcNeuron = srcNeuron;
				link->dstNeuron = dstNeuron;

				makePolyLineLink(srcNeuron.get(), dstNeuron.get(), link->path);
			}
			// ������������� �����:
			link = net->newEntity<Link>();
			if (link) {
				link->type = LinkType::Negative;
				link->srcNeuron = srcNeuron;
				link->dstNeuron = dstNeuron;

				makePolyLineLink(srcNeuron.get(), dstNeuron.get(), link->path);
			}
		}
	}

	// ������ ��������
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

		// ���� 1.
		// ����� ������� ������ �������� �������� �������� ����, � ������ ���������, ������ ����������������
		// ������ �������� ������, � ��������� ������ - ������.
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
						neuron->setOutValue(1.0);
					else
						neuron->setOutValue(0.0);
				}
			}

			if (layer->name() == "OutLayer") {
				for (int i = 0; i < layer->neurons.size(); ++i) {
					auto neuron = layer->neurons[i];
					if (i == 0)
						neuron->setOutValue(1.0);
					else
						neuron->setOutValue(0.0);
				}
			}
		}
	};
	_p->lessons["Lesson1"] = lesson1;

	std::function<void()> lesson2 = [this]() -> void {
		auto net = getNet();
		if (!net)
			return;

		// ���� 2.
		// ����� ������� ������ �������� �������� �������� ����, � ������ ���������, ������ ����������������
		// ������ �������� ������, � ��������� ������ - ������.
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
						neuron->setOutValue(0.0);
					else
						neuron->setOutValue(1.0);
				}
			}

			if (layer->name() == "OutLayer") {
				for (int i = 0; i < layer->neurons.size(); ++i) {
					auto neuron = layer->neurons[i];
					if (i == 0)
						neuron->setOutValue(0.0);
					else
						neuron->setOutValue(1.0);
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
