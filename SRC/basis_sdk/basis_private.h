#pragma once

#include "basis.h"
#include <map>
#include <boost/dll.hpp>
#include <boost/function.hpp>
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
		System* system_ptr;                     /// ссылка на систему
		tid typeId;                             /// тип сущности
		uid id;                                 /// уникальный идентификатор сущности
		Entity* prototype = nullptr;            /// ссылка на прототип
		std::vector<std::shared_ptr<Entity>> facets; /// грани этой сущности
	};

	struct Executable::Private
	{
		std::function<void()> stepFunction = nullptr; /// функция, вызываемая внутри step()
	};

	struct Container::Private
	{
		std::list<std::shared_ptr<Entity>> entities; /// сущности
		std::map<uid, std::list<std::shared_ptr<Entity>>::iterator> uuid_index; /// индексатор по UUID
		std::list<std::shared_ptr<Executable>> executors; /// список исполняемых сущностей для этого контейнера
	};

	struct System::Private {
		/**
		* @brief Загрузить требуемый модуль.
		* @return ссылка на публичный интерфейс загруженного модуля
		*/
		std::shared_ptr<Module> loadModule(const std::string& path);

		std::map<std::string, std::shared_ptr<Module>> modules;     /// загруженные модули
		std::map<tid, std::shared_ptr<FactoryInterface>> factories; /// фабрики сущностей
		std::atomic<bool> shouldStop = { false };                   /// флаг "Завершить вычисления"
		std::shared_ptr<Container> container = nullptr;             /// корневой контейнер
	};
};