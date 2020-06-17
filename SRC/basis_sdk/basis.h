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
	Iterator(const Iterator<T>&) = delete;
	Iterator(Iterator<T>&&) noexcept;
	Iterator<T>& operator=(const Iterator<T>&) = delete;
	Iterator<T>& operator=(Iterator<T>&&) noexcept;
	/// Получить текущий элемент.
	std::shared_ptr<T> value();
	/// Перейти к следующему элементу.
	void next();
	/// Проверить условие конца списка.
	bool finished() const;
	/// Проверить условие продолжения списка.
	bool hasMore() const;
	/// Установить условие отбора.
	void setSelector(Selector<T> selector);
	/// Получить условие отбора, установленное для этого итератора.
	Selector<T> selector() const;
	/// Prefix increment operator.
	Iterator<T>& operator++();
	/// Postfix increment operator.
	Iterator<T> operator++(int);

protected:
	virtual std::shared_ptr<T> _value();
	virtual void _next();
	virtual bool _finished() const;
	virtual void _reset();
	virtual void _swap(Iterator<T>&) noexcept;

private:
	/// Non-virtual wrapper for _swap().
	void swap(Iterator<T>&) noexcept;

protected:
	Selector<T> _selector = nullptr; /// условие отбора
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
	void _swap(Iterator<T>&) noexcept override;

private:
	std::shared_ptr<EntityList> _list;
	typename EntityList::iterator _position;
};

//template <class T>
//using IteratorPtr = std::shared_ptr<Iterator<T>>;

template <class T>
class IteratorPtr
{
public:
	IteratorPtr(Iterator<T>*);
	IteratorPtr(const IteratorPtr<T>&);
	IteratorPtr<T> operator=(const IteratorPtr<T>&);
	Iterator<T>* operator->();
	Iterator<T>& operator*();

private:
	std::shared_ptr<Iterator<T>> _iter = nullptr;
};

// ------------------------------------- Реализация ---------------------------------------

template <class T>
std::shared_ptr<T> Iterator<T>::_value()
{
	return nullptr;
}

template <class T>
void Iterator<T>::_next()
{
}

template <class T>
bool Iterator<T>::_finished() const
{
	return true;
}

template <class T>
void Iterator<T>::_reset()
{
}

template <class T>
void Iterator<T>::_swap(Iterator<T>&) noexcept
{
}

template <class T>
IteratorPtr<T>::IteratorPtr(Iterator<T> *iter) : _iter(std::shared_ptr<Iterator<T>>(iter))
{
}

template <class T>
IteratorPtr<T>::IteratorPtr(const IteratorPtr<T>& other)
{
	_iter = other._iter;
}

template <class T>
IteratorPtr<T> IteratorPtr<T>::operator=(const IteratorPtr<T>& other)
{
	_iter = other._iter;
	return *this;
}

template <class T>
Iterator<T>* IteratorPtr<T>::operator->()
{
	return _iter.get();
}

template <class T>
Iterator<T>& IteratorPtr<T>::operator*()
{
	return *(_iter.get());
}

template <class T>
Iterator<T>::Iterator()
{}

template <class T>
Iterator<T>::Iterator(Iterator<T> &&src) noexcept
{
	_swap(src);
}

template <class T>
Iterator<T>& Iterator<T>::operator=(Iterator<T>&& src) noexcept
{
	Iterator<T> tmp(std::move(src));
	_swap(tmp);
	return *this;
}

template <class T>
void Iterator<T>::swap(Iterator<T>& other) noexcept
{
	_swap();
	std::swap<>(_selector, other._selector);
	//Selector<T> tmp = this->_selector;
	//this->_selector = other._selector;
	//other._selector = tmp;
}

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
bool Iterator<T>::hasMore() const
{
	return (_finished() == false);
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
Iterator<T>& Iterator<T>::operator++()
{
	next();
	return *this;
}

template <class T>
Iterator<T> Iterator<T>::operator++(int)
{
	Iterator<T> temp = *this;
	++* this;
	return temp;
}

template <class T>
ListIterator<T>::ListIterator(std::shared_ptr<EntityList>& lst) :
	Iterator<T>(),
	_list(lst)
{
	_position = _list->begin();
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

template <class T>
void ListIterator<T>::_swap(Iterator<T> &other) noexcept
{
	ListIterator<T> *li = static_cast<ListIterator<T>*>(&other);
	std::swap<>(_list, li->_list);
	std::swap<>(_position, li->_position);
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
	IteratorPtr<Entity> entityIterator(Selector<Entity> match = nullptr);

	Iterator<Entity> entityIteratorNew(Selector<Entity> match = nullptr);

	/// @brief Get nested entities of specified type, arranged in new array.
	std::vector<std::shared_ptr<Entity>> entityCollection(Selector<Entity> match = nullptr);

	void step();

	operator bool() const;

private:
	void setTypeId(tid typeId);
	std::shared_ptr<EntityList> entities();

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

template<class T>
std::shared_ptr<T> Entity::newEntity()
{
	return dynamic_pointer_cast<T>(newEntity(TYPEID(T)));
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
