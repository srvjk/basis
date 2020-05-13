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

class System;

/// @brief Сущность - базовый класс для всех объектов в системе.
class BASIS_EXPORT Entity
{
	template <class T> friend class Factory;
	friend class System;
	friend class Container;
	struct Private;

public:
	Entity(System* sys);
	virtual ~Entity();

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

	/// @brief Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	std::shared_ptr<Entity> addFacet(tid protoTypeId);

	/// @brief Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	std::shared_ptr<Entity> addFacet(Entity* prototype);

	/// Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	template<class T>
	std::shared_ptr<T> addFacet();

	/// @brief Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	template<class T>
	std::shared_ptr<T> addFacet(T* prototype);

	/// @brief Выяснить, имеет ли сущность прототип или является корневой.
	bool hasPrototype() const;

	/// @brief Проверить, является ли данная сущность экземпляром другой сущности.
	bool isKindOf(tid typeId) const;

	/// @brief Проверить, является ли данная сущность экземпляром другой сущности.
	template<class T>
	bool isKindOf() const;

	/// @brief Проверить, имеет ли сущность хотя бы одну грань данного типа.
	bool hasFacet(tid typeId); // TODO избыточная функция, убрать!

	/// @brief Доступ к списку граней.
	Iterable::IteratorPtr<std::shared_ptr<Entity>> facets();

	/// @brief Доступ к списку граней заданного типа.
	Iterable::IteratorPtr<std::shared_ptr<Entity>> facets(tid typeId);

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

private:
	void setTypeId(tid typeId);

private:
	std::unique_ptr<Private> _p;
};

template <class T>
std::shared_ptr<T> Entity::addFacet() 
{
	return std::static_pointer_cast<T>(addFacet(TYPEID(T)));
}

template <class T>
std::shared_ptr<T> Entity::addFacet(T* prototype)
{
	return std::static_pointer_cast<T>(addFacet(prototype));
}

template <class T>
bool Entity::isKindOf() const
{
	return isKindOf(TYPEID(T));
}

template <class T>
T* eCast(Entity* e)
{
	if (!e)
		return nullptr;
	if (e->typeId() != TYPEID(T))
		return nullptr;
	return static_cast<T*>(e);
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
	virtual void print();

private:
	std::unique_ptr<Private> _p;
};

/// @brief Контейнер - сущность, содержащая другие сущности.
class BASIS_EXPORT Container : public Entity
{
	struct Private;

public:
	Container(System* sys);
	virtual ~Container();

	/// @brief Создать сущность-прототип.
	///
	/// @attention Если такая сущность уже существует в данном контейнере, функция вернет ее, а не создаст новую,
	/// поскольку в контейнере может существовать максимум один прототип для каждого типа!
	std::shared_ptr<Entity> newPrototype(tid typeId);

	/// @brief Создать сущность по заданному прототипу.
	///
	/// Прототип задаётся идентификатором своего типа. Система гарантирует, что в данном контейнере будет
	/// не более одного прототипа для каждого типа.
	std::shared_ptr<Entity> newEntity(tid protoTypeId);

	/// @brief Создать сущность по заданному прототипу.
	///
	/// Будет автоматически выведен тип создаваемой сущности, созданы её грани и свойства.
	std::shared_ptr<Entity> newEntity(Entity* prototype);

	/// @brief Создать сущность-прототип.
	///
	/// @attention Если такая сущность уже существует в данном контейнере, функция вернет ее, а не создаст новую,
	/// поскольку в контейнере может существовать максимум один прототип для каждого типа!
	template<class T>
	std::shared_ptr<T> newPrototype();

	/// @brief Создать сущность по заданному прототипу.
	///
	/// Прототип задаётся своим типом. Система гарантирует, что в данном контейнере будет
	/// не более одного прототипа для каждого типа.
	template<class T>
	std::shared_ptr<T> newEntity();

	/// @brief Создать сущность по заданному прототипу.
	///
	/// Будет автоматически выведен тип создаваемой сущности, созданы её грани и свойства.
	template<class T>
	std::shared_ptr<T> newEntity(T* prototype);

	/// @brief Доступ к списку вложенных сущностей, удовлетворяющих заданному критерию.
	Iterable::IteratorPtr<std::shared_ptr<Entity>> entities(Iterable::Selector<std::shared_ptr<Entity>> match = nullptr);

	/// @brief Доступ к списку вложенных сущностей заданного типа.
	Iterable::IteratorPtr<std::shared_ptr<Entity>> entities(tid typeId);

	/// @brief Сформировать и вернуть список вложенных сущностей, удовлетворяющих заданному критерию отбора.
	//std::list<std::shared_ptr<Entity>> entList(Iterable::Selector<std::shared_ptr<Entity>> match = nullptr);

	/// @brief Добавить сущность с заданным id в список исполняемых для этого контейнера.
	bool addExecutor(const uid& id);

	/// @brief Пройтись по исполняемым сущностям в контейнере и выполнить их.
	void step();

	/// @brief Распечатать собственное описание.
	virtual void print();

private:
	std::unique_ptr<Private> _p;
};

template<class T>
std::shared_ptr<T> Container::newPrototype()
{
	return std::dynamic_pointer_cast<T>(newPrototype(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Container::newEntity()
{
	return std::dynamic_pointer_cast<T>(newEntity(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Container::newEntity(T* prototype)
{
	return std::dynamic_pointer_cast<T>(newEntity(prototype));
}

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
class BASIS_EXPORT System
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

	/// @brief Доступ к корневому контейнеру сущностей.
	std::shared_ptr<Container> container();

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
