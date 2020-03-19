#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef NEURO_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define EXPORTED
#endif

/**
* @brief Нейрон.
*/
class Neuron : public Basis::Entity
{
public:
	Neuron(Basis::System* s);

};

/**
* @brief Нейросеть - контейнер с нейронами.
*/
class NeuroNet : public Basis::Entity
{
public:
	NeuroNet(Basis::System* s);
};

/**
* @brief Скетч, демонстрирующий простую классификацию объектов на основе нейросети.
*
* В этом скетче создается, обучается и тестируется нейросеть, разбивающая предъявленные ей
* объекты на несколько категорий.
*/
class SimplisticNeuralClassification : public Basis::Entity
{
public:
	SimplisticNeuralClassification(Basis::System* s);
	bool init() override;
	void step();
	void cleanup() override;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);