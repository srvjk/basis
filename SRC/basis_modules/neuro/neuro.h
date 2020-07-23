#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef NEURO_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

class Neuron;

/// @brief Interneuronal link.
class MODULE_EXPORT Link : public Basis::Entity
{
public:
	Link(Basis::System* s);

public:
	std::shared_ptr<Neuron> srcNeuron = nullptr;
	std::shared_ptr<Neuron> dstNeuron = nullptr;
	bool active = false;
	float growthFactor = 0.0;
};

/// @brief Neuron.
class MODULE_EXPORT Neuron : public Basis::Entity
{
public:
	Neuron(Basis::System* s);
	void setValue(double v);
	double value() const;

private:
	double _value = 0.0;
};

/// @brief Layer.
class MODULE_EXPORT Layer : public Basis::Entity
{
public:
	Layer(Basis::System* s);

public:
	std::vector<std::shared_ptr<Neuron>> neurons;
};

/// @brief Neural network.
class MODULE_EXPORT NeuroNet : public Basis::Entity
{
	struct Private;

public:
	NeuroNet(Basis::System* s);
	void rememberNeuronByName(std::shared_ptr<Neuron> neuron, const std::string& name);
	std::shared_ptr<Neuron> recallNeuronByName(const std::string& name);

private:
	std::unique_ptr<Private> _p = nullptr;
};

/// @brief Простейший нейронный классификатор.
class MODULE_EXPORT SimplisticNeuralClassification : public Basis::Entity
{
public:
	SimplisticNeuralClassification(Basis::System* s);
	bool init() override;
	void step();
	void cleanup() override;
};

/// @brief Тренер - сущность, служащая для тренировки нейросетей.
class MODULE_EXPORT Trainer : public Basis::Entity
{
	struct Private;

public:
	Trainer(Basis::System* s);
	bool isActive() const;
	void setActive(bool active = true);
	void setNet(std::shared_ptr<NeuroNet> net);
	std::shared_ptr<NeuroNet> getNet() const;
	void train();

private:
	std::unique_ptr<Private> _p = nullptr;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);