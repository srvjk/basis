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

enum class LinkType
{
	Negative = -1,
	Neutral = 0, // for use as initial value only
	Positive = 1
};

/// @brief Interneuronal link.
class MODULE_EXPORT Link : 
	public Basis::Entity
{
public:
	Link(Basis::System* s);

public:
	std::shared_ptr<Neuron> srcNeuron = nullptr;
	std::shared_ptr<Neuron> dstNeuron = nullptr;
	bool active = false;
	LinkType type = LinkType::Neutral;
	float growthFactor = 0.5;
	std::vector<Basis::point3d> path;
};

/// константа "невозможно высокого" (запрещающего) порога активации
static const double ACTIVATION_LOCK = DBL_MAX;

/// @brief Neuron.
class MODULE_EXPORT Neuron : 
	public Basis::Entity
{
public:
	Neuron(Basis::System* s);
	/// @brief Добавить величину v к текущему входному значению нейрона.
	void addInValue(double v);
	/// @brief Принудительно установить входное значение.
	void setInValue(double v);
	/// @brief Принудительно установить выходное значение.
	void setOutValue(double v);
	/// @brief Получить входное значение.
	double inValue() const;
	/// @brief Получить выходное значение.
	double outValue() const;
	/// @brief Получить значение порога активации нейрона.
	double activationThreshold() const;
	/// @brief Установить значение порога активации нейрона.
	void setActivationThreshold(double v);
	/// @brief Активен ли нейрон?
	bool isActive() const;
	/// @brief Отметка времени последнего изменения активности нейрона.
	int64_t stateChangedTimeStamp() const;

private:
	double _activationThreshold = ACTIVATION_LOCK; /// по умолчанию нейрон не может активироваться
	double _inValue = 0.0;                         /// входное значение (сумма входных сигналов)
	double _outValue = 0.0;                        /// выходное значение
	int64_t _activityChangedTimeStamp = 0;         /// метка времени последнего изменения активности
};

/// @brief Layer.
class MODULE_EXPORT Layer : 
	public Basis::Entity
{
public:
	Layer(Basis::System* s);

public:
	std::vector<std::shared_ptr<Neuron>> neurons;
};

/// @brief Neural network.
class MODULE_EXPORT NeuroNet : 
	public Basis::Entity
{
	struct Private;

public:
	NeuroNet(Basis::System* s);
	void rememberNeuronByName(std::shared_ptr<Neuron> neuron, const std::string& name);
	std::shared_ptr<Neuron> recallNeuronByName(const std::string& name);
	void tick(); // single step of neural activity
	void pause();
	bool isPaused();
	void resume();
	double activationThreshold() const;
	void setActivationThreshold(double v);
	bool isSpontaneousActivityEnabled() const;
	void enableSpontaneousActivity(bool enable = true);

private:
	std::unique_ptr<Private> _p = nullptr;
};

/// @brief Простейший нейронный классификатор.
class MODULE_EXPORT SimplisticNeuralClassification : 
	public Basis::Entity
{
public:
	SimplisticNeuralClassification(Basis::System* s);
	bool init() override;
	void step();
	void cleanup() override;
};

/// @brief Тренер - сущность, служащая для тренировки нейросетей.
class MODULE_EXPORT Trainer : 
	public Basis::Entity
{
	struct Private;

public:
	Trainer(Basis::System* s);
	void setNet(std::shared_ptr<NeuroNet> net);
	std::shared_ptr<NeuroNet> getNet() const;
	std::list<std::string> listLessons() const;
	std::string activeLesson() const;
	void setActiveLesson(const std::string& lesson);
	void train();

private:
	std::unique_ptr<Private> _p = nullptr;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);