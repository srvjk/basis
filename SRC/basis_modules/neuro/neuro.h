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

/**
* @brief ������.
*/
class Neuron : public Basis::Entity
{
public:
	Neuron(Basis::System* s);

};

/**
* @brief ��������� - ��������� � ���������.
*/
class NeuroNet : public Basis::Entity
{
public:
	NeuroNet(Basis::System* s);
};

/**
* @brief �����, ��������������� ������� ������������� �������� �� ������ ���������.
*
* � ���� ������ ���������, ��������� � ����������� ���������, ����������� ������������� ��
* ������� �� ��������� ���������.
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