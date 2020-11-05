#pragma once

#include "basis.h"
#include <map>
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <functional>

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
		Entity* parent = nullptr;                      /// ссылка на родительскую сущность (для граней)
		std::map<tid, std::shared_ptr<Entity>> facets; /// грани этой сущности
		std::shared_ptr<List<Entity>> entities;        /// сущности
		std::map<uid, std::shared_ptr<ListItem<Entity>>> uuid_index; /// индексатор по UUID
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
		std::atomic<bool> paused = { false };                       /// pause flag
		boost::random::mt19937 randGen;                             /// random number generator
		int64_t stepsFromStart = 0;                                 /// number of steps passed from start
		int64_t stepsToDo = -1;                                     /// number of steps to do before pause

		Private() {}
		~Private() {}
	};
};