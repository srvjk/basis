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
		Entity*     prototype = nullptr;           /// ссылка на прототип
		System*     system_ptr;                    /// ссылка на систему
		tid         typeId;                        /// идентификатор типа сущности
		std::string typeName;                      /// имя типа сущности
		uid         id;                            /// уникальный идентификатор сущности
		std::string name;                          /// собственное имя сущности
		std::list<std::shared_ptr<Entity>> facets; /// грани этой сущности
	};

	struct Executable::Private
	{
		std::function<void()> stepFunction = nullptr; /// функция, вызываемая внутри step()
	};

	struct Container::Private
	{
		std::list<std::shared_ptr<Entity>> entities; /// сущности
		std::map<uid, std::list<std::shared_ptr<Entity>>::iterator> uuid_index; /// индексатор по UUID
		std::list<std::shared_ptr<Entity>> executors; /// список исполняемых сущностей для этого контейнера
	};

	struct Spatial::Private
	{
		point3d position;    /// положение в системе координат родителя
		point3d orientation; /// ориентация в системе координат родителя (углы Эйлера)
		double size;         /// размер (радиус занимаемой области)
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