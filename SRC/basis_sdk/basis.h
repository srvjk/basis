#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <list>
#include <boost/type_index.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>

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

template <class T>
using Selector = std::function<bool(std::shared_ptr<T>)>;

/// @brief Абстрактный итератор.
template <class T>
class Iterator
{
public:
	Iterator();
	/// Получить текущий элемент.
	std::shared_ptr<T> value();
	/// Перейти к следующему элементу.
	void next();
	/// Проверить условие конца списка.
	bool finished() const;
	/// Установить условие отбора.
	void setSelector(Selector<T> selector);
	/// Получить условие отбора, установленное для этого итератора.
	Selector<T> selector() const;

protected:
	virtual std::shared_ptr<T> _value() = 0;
	virtual void _next() = 0;
	virtual bool _finished() const = 0;
	virtual void _reset() = 0;

protected:
	std::function<bool(std::shared_ptr<T>)> _selector = nullptr; /// условие отбора
};

using EntityList = std::list<std::shared_ptr<Entity>>;

/// @brief Итератор списка.
template <class T>
class ListIterator : public Iterator<T>
{
public:
	ListIterator(std::shared_ptr<EntityList>& lst);

protected:
	std::shared_ptr<T> _value() override;
	void _next() override;
	bool _finished() const override;
	void _reset() override;

private:
	std::shared_ptr<EntityList> _list;
	typename EntityList::iterator _position;
};

template <class T>
using IteratorPtr = std::shared_ptr<Iterator<T>>;

// ------------------------------------- Реализация ---------------------------------------

template <class T>
Iterator<T>::Iterator()
{}

template <class T>
std::shared_ptr<T> Iterator<T>::value()
{
	// если условие выбора не задано, просто возвращаем текущий элемент
	if (_selector == nullptr)
		return _value();

	// если условие выбора задано:
	while (!_finished()) {
		std::shared_ptr<T> val = _value();
		if (_selector(val))
			return val; // ok, условие удовлетворено
		// условие не удовлетворено, переходим к следующему элементу
		_next();
	}

	return nullptr;
}

template <class T>
void Iterator<T>::next()
{
	if (_finished())
		return;

	// если условие не задано, просто перемещаемся к следующему элементу
	if (_selector == nullptr) {
		_next();
		return;
	}

	// если условие задано, прокручиваем до следующего подходящего элемента
	// или до конца списка
	while (!_finished()) {
		_next();
		if (_finished())
			break;
		if (_selector(_value()))
			break;
	}
}

template <class T>
bool Iterator<T>::finished() const
{
	return _finished();
}

template <class T>
void Iterator<T>::setSelector(Selector<T> selector)
{
	_reset();
	_selector = selector;
	value(); // прокрутка до первого элемента, удовлетворяющего условию
}

template <class T>
Selector<T> Iterator<T>::selector() const
{
	return _selector;
}

template <class T>
ListIterator<T>::ListIterator(std::shared_ptr<EntityList>& lst) :
	Iterator<T>(),
	_list(lst)
{
	_position = _list->begin();
	if (_list->empty())
		int jjj = 0;
	if (_position == _list->end())
		int iii = 0;
}

template <class T>
bool ListIterator<T>::_finished() const
{
	if (_position != _list->end())
		return false;

	return true;
}

template <class T>
std::shared_ptr<T> ListIterator<T>::_value()
{
	if (_finished())
		return nullptr;

	std::shared_ptr<Entity> ent = *_position;
	if (ent->typeId() == TYPEID(T))
		return std::static_pointer_cast<T>(ent);

	return nullptr;
}

template <class T>
void ListIterator<T>::_next()
{
	if (_finished())
		return;

	++_position;
}

template <class T>
void ListIterator<T>::_reset()
{
	_position = _list->begin();
}

/// @brief Коллекция сущностей, сгруппированных по какому-либо признаку.
class BASIS_EXPORT EntityCollection
{
	struct Private;

public:
	EntityCollection();
	void append(std::shared_ptr<Entity>& item);
	std::shared_ptr<Entity> at(int64_t index);
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

	/// @brief Create new entity of specified type.
	std::shared_ptr<Entity> newEntity(tid typeId);

	/// @brief Create new entity of specified type.
	template<class T>
	std::shared_ptr<T> newEntity();

	/// @brief Get in-place access to nested entities via iterator.
	IteratorPtr<Entity> entityIterator();

	/// @brief Get in-place access to nested entities via iterator.
	template<class T>
	IteratorPtr<T> entityIterator(Selector<Entity> match);

	/// @brief Get in-place access to nested entities via iterator.
	IteratorPtr<Entity> entityIterator(tid typeId);

	/// @brief Get in-place access to nested entities via iterator.
	template<class T>
	IteratorPtr<T> entityIterator();

	/// @brief Get nested entities arranged in new array.
	std::shared_ptr<EntityCollection> entityCollection();

	/// @brief Get nested entities of specified type, arranged in new array.
	std::shared_ptr<EntityCollection> entityCollection(tid typeId);

	/// @brief Get nested entities of specified type, arranged in new array.
	template<class T>
	std::vector<std::shared_ptr<T>> entityCollection();

	void step();

	operator bool() const;

private:
	void setTypeId(tid typeId);
	std::shared_ptr<EntityList> entities();

private:
	std::unique_ptr<Private> _p;
};

template<class T>
IteratorPtr<T> Entity::entityIterator(Selector<Entity> match)
{
	ListIterator<T>* iter = new ListIterator<T>(entities());
	iter->setSelector(match);

	return IteratorPtr<T>(iter);
}

template<class T>
IteratorPtr<T> Entity::entityIterator()
{
	ListIterator<T>* iter = new ListIterator<T>(entities());
	tid typeId = TYPEID(T);
	iter->setSelector([typeId](std::shared_ptr<Entity> ent)->bool {
		if (!ent)
			return false;
		return (ent->typeId() == typeId);
	});

	return IteratorPtr<T>(iter);
}

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

template<class T>
std::shared_ptr<T> Entity::newEntity()
{
	return dynamic_pointer_cast<T>(newEntity(TYPEID(T)));
}

template<class T>
std::vector<std::shared_ptr<T>> Entity::entityCollection()
{
	std::vector<std::shared_ptr<T>> result;
	IteratorPtr<T> iter = entityIterator<T>();
	while (!iter->finished()) { // TODO should be 'for'
		result.push_back(iter->value());
		iter->next();
	}

	return result;
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
	if (!ent)
		return false;
	return ent->hasFacet(TYPEID(Executable));
});

/// @brief Модульные тесты.
bool BASIS_EXPORT test();

} // namespace Basis
