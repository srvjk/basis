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

/// @brief Нейрон.
class MODULE_EXPORT Neuron : public Basis::Entity
{
public:
	Neuron(Basis::System* s);
	void setValue(double v);
	double value() const;

private:
	double _value = 0.0;
};

/// @brief Нейронная сеть.
class MODULE_EXPORT NeuroNet : public Basis::Entity
{
public:
	NeuroNet(Basis::System* s);
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
	void train();

private:
	std::unique_ptr<Private> _p = nullptr;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);