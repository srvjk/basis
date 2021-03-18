#pragma once

#include "basis.h"
#include <map>
#include <atomic>
#include <functional>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

namespace Basis
{
	struct Module
	{
		std::string name;
		boost::dll::shared_library lib;
		boost::function<void(Basis::System*)> setup_func;
	};

	struct Entity::Private
	{
		System* system_ptr = nullptr;                  /// ссылка на систему
		tid         typeId;                            /// идентификатор типа сущности
		uid         id;                                /// уникальный идентификатор сущности
		std::string name;                              /// собственное имя сущности
		Entity* parent = nullptr;                      /// ссылка на родительскую сущность
		std::map<tid, std::shared_ptr<Entity>> facets; /// грани этой сущности
		std::shared_ptr<List<Entity>> entities;        /// сущности
		std::map<uid, std::shared_ptr<ListItem<Entity>>> uuidIndex;    /// индексатор по UUID
		std::multimap<std::string, std::shared_ptr<Entity>> nameIndex; /// индексатор по имени
	};

	struct Executable::Private
	{
		std::function<void()> stepFunction = nullptr; /// функция, вызываемая внутри step()
		bool active = false; /// activity flag
	};

	struct Spatial::Private
	{
		point3d position;    /// положение в системе координат родителя
		point3d orientation; /// ориентация в системе координат родителя (углы Эйлера)
		double size;         /// размер (радиус занимаемой области)
	};

	struct System::Private 
	{
		/**
		* @brief Загрузить требуемый модуль.
		* @return ссылка на публичный интерфейс загруженного модуля
		*/
		std::shared_ptr<Module> loadModule(const std::string& path);

		std::map<std::string, std::shared_ptr<Module>> modules;     /// загруженные модули
		std::map<tid, std::shared_ptr<FactoryInterface>> factories; /// фабрики сущностей
		std::atomic<bool> shouldStop = { false };                   /// флаг "Завершить вычисления"
		std::atomic<bool> paused = { false };                       /// флаг режима "Пауза"
		boost::random::mt19937 randGen;                             /// генератор случайных чисел
		int64_t stepsFromStart = 0;                                 /// число шагов, пройденных от старта
		int64_t stepsToDo = -1;                                     /// число шагов, которые нужно сделать перед следующей паузой
		int delayBetweenSteps = 0;                                  /// задержка в миллисекундах между шагами

		Private() {}
		~Private() {}
	};
};
