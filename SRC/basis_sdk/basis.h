#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <list>
#include <boost/type_index.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include "iterable.h"

#ifdef PLATFORM_WINDOWS
#  ifdef BASIS_LIB
#    define BASIS_EXPORT __declspec(dllexport)
#  else
#    define BASIS_EXPORT __declspec(dllimport)
#  endif
#else
#  define BASIS_EXPORT
#endif

#define TYPEID(T) typeid(T).hash_code()
#define TYPENAME(T) typeid(T).name()

namespace Basis
{

using tid = size_t; // псевдоним для идентификатора типа
namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point2d;
typedef bg::model::point<double, 3, bg::cs::cartesian> point3d;
using uid = boost::uuids::uuid; // псевдоним для идентификатора сущности

class Entity;
class System;

/// @brief Коллекция сущностей, сгруппированных по какому-либо признаку.
class BASIS_EXPORT EntityCollection
{
	struct Private;

public:
	EntityCollection();
	void append(std::shared_ptr<Entity>& item);
	std::shared_ptr<Entity> operator[](int64_t index);

private:
	std::unique_ptr<Private> _p;
};

/// @brief Сущность - базовый класс для всех объектов в системе.
class BASIS_EXPORT Entity
{
	template <class T> friend class Factory;
	friend class System;
	struct Private;

public:
	Entity();
	Entity(System* sys);
	virtual ~Entity();

	/// @brief Проверка на нулевую сущность.
	bool isNull() const;

	/// @brief Получить идентификатор типа этой сущности.
	tid typeId() const;

	/// @brief Получить собственный идентификатор этой сущности.
	uid id() const;

	/// @brief Получить строковое имя фактического типа этой сущности.
	const std::string typeName() const;

	/// @brief Получить собственное имя сущности.
	const std::string name() const;

	/// @brief Назначить имя данной сущности.
	void setName(const std::string& name);

	/// @brief Получить определенную грань данной сущности.
	std::shared_ptr<Entity> as(tid typeId);

	/// @brief Get certain facet of the entity.
	template<class T>
	std::shared_ptr<T> as();

	/// @brief Добавить к данной сущности грань определенного типа.
	std::shared_ptr<Entity> addFacet(tid typeId);

	/// @brief Добавить к данной сущности грань определенного типа.
	template<class T>
	std::shared_ptr<T> addFacet();

	bool Entity::hasFacet(tid typeId);

	/// @brief Получить ссылку на систему.
	System* system() const;

	/// @brief Распечатать собственное описание.
	virtual void print();

	/// @brief Инициализация сущности.
	///
	/// Вызывается после конструктора, но до первого вызова step().
	/// Здесь может выполняться выделение ресурсов, создание окон и т.п.
	virtual bool init();

	/// @brief Финализация сущности.
	///
	/// Вызывается непосредственно перед деструктором сущности.
	/// Здесь может выполняться освобождение ресурсов, закрытие окон и т.п.
	virtual void cleanup();

	/// @brief Создать сущность заданного типа.
	std::shared_ptr<Entity> newEntity(tid typeId);

	/// @brief Get in-place access to nested entities via iterator.
	Iterable::IteratorPtr<std::shared_ptr<Entity>> entityIterator(Iterable::Selector<std::shared_ptr<Entity>> match = nullptr);

	/// @brief Get nested entities arranged in new array.
	std::shared_ptr<EntityCollection> entityCollection();

	void step();

	operator bool() const;

private:
	void setTypeId(tid typeId);

private:
	std::unique_ptr<Private> _p;
};

template<class T>
std::shared_ptr<T> Entity::as()
{
	return dynamic_pointer_cast<T>(as(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Entity::addFacet()
{
	return dynamic_pointer_cast<T>(addFacet(TYPEID(T)));
}

/// @brief Исполняемая сущность.
class BASIS_EXPORT Executable : public Entity
{
	struct Private;

public:
	Executable(System* sys);
	virtual ~Executable();
	void step();
	void setStepFunction(std::function<void()> func);
	void setActive(bool active = true);
	bool isActive() const;
	virtual void print();

private:
	std::unique_ptr<Private> _p;
};

/// @brief Пространственная сущность.
class BASIS_EXPORT Spatial : public Entity
{
	struct Private;

public:
	Spatial(System* sys);
	virtual ~Spatial();
	point3d position() const;
	void setPosition(const point3d& pos);
	point3d orientation() const;
	void setOrientation(const point3d& orient);
	double size() const;
	void setSize(double sz);

private:
	std::unique_ptr<Private> _p;
};

/// @brief Публичный интерфейс фабрики сущностей.
class FactoryInterface
{
public:
	virtual Entity* newEntity(System*) = 0;
	virtual tid typeId() const = 0;
	virtual std::string typeName() const = 0;
};

/// @brief Функция отрезает от строки str всё до подстроки what включительно.
void BASIS_EXPORT cutoff(std::string& str, const std::string& what);

/// @brief Фабрика сущностей.
/// Предназначена для динамического создания сущностей заданного типа.
template <class T>
class Factory : public FactoryInterface
{
protected:
	T* _new(System* sys) { return new T(sys); }

public:
	Factory()
	{
		_typeName = boost::typeindex::type_id<T>().pretty_name();
		cutoff(_typeName, ":");
		cutoff(_typeName, "class ");
	}

	Entity* newEntity(System* sys)
	{
		Entity* ent = static_cast<Entity*>(_new(sys));
		ent->setTypeId(TYPEID(T));

		return ent;
	}

	tid typeId() const
	{
		return TYPEID(T);
	}

	std::string typeName() const
	{
		return _typeName;
	}

private:
	std::string _typeName;
};

/// @brief Система - корень мира сущностей.
class BASIS_EXPORT System : public Entity
{
	struct Private;

public:
	static System* instance();
	
	/// @brief Загрузить все модули по заданному пути.
	/// Путь может быть либо файлом, либо директорией.
	/// @param recursive просматривать вложенные директории
	/// @return количество загруженных модулей
	int loadModules(const std::string& path, bool recursive = false);

	/// @brief Регистрация сущности заданного типа в системе.
	template<class T> bool registerEntity();

	/// @brief Зарегистрирована ли в системе сущность данного типа? 
	bool isEntityRegistered(tid typeId) const;

	/// @brief Возвращает количество зарегистрированных типов сущностей.
	int64_t entityTypesCount() const;

	/// @brief Создать сущность, не добавляя ее в список, не проверяя единственность и т.п.
	std::shared_ptr<Entity> createEntity(tid typeId);

	/// @brief Получить имя типа сущности по его идентификатору.
	std::string typeIdToTypeName(tid typeId) const;

	/// @brief Корневая выполняемая процедура.
	void step();

	/// @brief Была ли команда завершить вычисления?
	bool shouldStop() const;

	template<class T>
	std::shared_ptr<T> createEntity();

	/// @brief Слот для приёма команды управления от внешнего источника (например, с консоли).
	void onCommand(const std::string& command);

	/// @brief Display brief help.
	void usage() const;

private:
	System();
	~System();
	bool addFactory(FactoryInterface* f);

private:
	Private* _p = nullptr;
};

template<class T>
std::shared_ptr<T> System::createEntity()
{
	return std::dynamic_pointer_cast<T>(createEntity(TYPEID(T)));
}

template<class T> bool System::registerEntity()
{
	if (isEntityRegistered(TYPEID(T)))
		return false;

	// создаём новую фабрику для сущностей этого типа
	return addFactory(new Factory<T>());
}

/// @brief Проверка, что сущность является исполняемой.
static auto check_executable([](std::shared_ptr<Entity> ent)->bool {
	return ent->hasFacet(TYPEID(Executable));
});

/// @brief Модульные тесты.
bool BASIS_EXPORT test();

} // namespace Basis
