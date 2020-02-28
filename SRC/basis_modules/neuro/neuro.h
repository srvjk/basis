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

class Neuro : public Basis::Entity
{
public:
	Neuro(Basis::System* s);
	void step();
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);