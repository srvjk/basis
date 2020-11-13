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
namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point2d;
typedef bg::model::point<double, 3, bg::cs::cartesian> point3d;

using tid = size_t;             // псевдоним для идентификатора типа
using uid = boost::uuids::uuid; // псевдоним для идентификатора сущности

point3d BASIS_EXPORT operator+(const point3d& p1, const point3d& p2);
point3d BASIS_EXPORT operator-(const point3d& p1, const point3d& p2);
point3d BASIS_EXPORT operator*(const point3d& p, double v);
point3d BASIS_EXPORT operator*(double v, const point3d& p);
double BASIS_EXPORT length(const point3d& v);

class Entity;
class System;

template <class T>
using Selector = std::function<bool(std::shared_ptr<T>)>;

/// @brief Абстрактный итератор.
class BASIS_EXPORT Iterator
{
public:
	Iterator();
	Iterator(const Iterator&) = delete;
	Iterator(Iterator&&) noexcept;
	Iterator& operator=(const Iterator&) = delete;
	Iterator& operator=(Iterator&&) noexcept;
	/// @brief Получить текущий элемент.
	std::shared_ptr<Entity> value();
	/// @brief Перейти к следующему элементу.
	void next();
	/// @brief Проверить условие конца списка.
	bool finished() const;
	/// @brief Проверить условие продолжения списка.
	bool hasMore() const;
	/// @brief Установить условие отбора.
	void setSelector(Selector<Entity> selector);
	/// @brief Получить условие отбора, установленное для этого итератора.
	Selector<Entity> selector() const;

protected:
	virtual std::shared_ptr<Entity> _value();
	virtual void _next();
	virtual bool _finished() const;
	virtual void _reset();

private:
	void swap(Iterator&) noexcept;

protected:
	Selector<Entity> _selector = nullptr; /// условие отбора
};

/// @brief Указатель на абстрактный итератор.
///
/// Указатель на итератор рекомендуется всегда использовать вместо
/// объекта Iterator, т.к. при этом обеспечивается корректное автоматическое
/// удаление итератора после использования.
class IteratorPtr
{
public:
	IteratorPtr(Iterator*);
	IteratorPtr(const IteratorPtr&);
	IteratorPtr operator=(const IteratorPtr&);
	Iterator* operator->();
	Iterator& operator*();

private:
	std::shared_ptr<Iterator> _iter = nullptr;
};

template <class T>
class List;

/// @brief Элемент абстрактного списка.
template <class T>
class ListItem
{
	friend class List<T>;

public:
	ListItem(std::shared_ptr<T>& value);
	std::shared_ptr<ListItem<T>> next();
	std::shared_ptr<ListItem<T>> prev();
	std::shared_ptr<T> value() const;

private:
	std::shared_ptr<ListItem<T>> _next = nullptr;
	std::shared_ptr<ListItem<T>> _prev = nullptr;
	std::shared_ptr<T> _value = nullptr;
};

/// @brief Абстрактный список.
template <class T>
class List
{
public:
	std::shared_ptr<ListItem<T>> pushBack(std::shared_ptr<T>& value);
	std::shared_ptr<ListItem<T>> remove(std::shared_ptr<ListItem<T>>& item);
	std::shared_ptr<ListItem<T>> head() const;
	std::shared_ptr<ListItem<T>> tail() const;
	int64_t size() const;

private:
	std::shared_ptr<ListItem<T>> _head = nullptr;
	std::shared_ptr<ListItem<T>> _tail = nullptr;
	int64_t _size = 0;
};

/// @brief Итератор списка сущностей.
class BASIS_EXPORT ListIterator : public Iterator
{
public:
	ListIterator(std::shared_ptr<List<Entity>>& lst);
	ListIterator(const ListIterator&) = delete;
	ListIterator(ListIterator&&) noexcept;
	ListIterator& operator=(const ListIterator&) = delete;
	ListIterator& operator=(ListIterator&&) noexcept;

protected:
	std::shared_ptr<Entity> _value() override;
	void _next() override;
	bool _finished() const override;
	void _reset() override;

private:
	void swap(ListIterator&) noexcept;

private:
	std::shared_ptr<List<Entity>> _list;
	std::shared_ptr<ListItem<Entity>> _position;
};

/// @brief Сущность - базовый класс для всех объектов в системе.
class BASIS_EXPORT Entity : public std::enable_shared_from_this<Entity>
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
	/// @brief Получить определенную грань данной сущности.
	template<class T>
	std::shared_ptr<T> as();
	/// @brief Добавить к данной сущности грань определенного типа.
	std::shared_ptr<Entity> addFacet(tid typeId);
	/// @brief Добавить к данной сущности грань определенного типа.
	template<class T>
	std::shared_ptr<T> addFacet();
	/// @brief Имеет ли сущность в своем составе грань данного типа?
	bool hasFacet(tid typeId);
	/// @brief Получить ссылку на объект системы.
	System* system() const;
	/// @brief Распечатать собственное описание.
	virtual void print();
	/// @brief Реакция на сообщение.
	virtual void onMessage(const std::string& message);
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
	/// @brief Создать новую дочернюю сущность заданного типа.
	std::shared_ptr<Entity> newEntity(tid typeId);
	/// @brief Создать новую дочернюю сущность заданного типа.
	template<class T>
	std::shared_ptr<T> newEntity();
	/// @brief Удалить все дочерние сущности, удовлетворяющие условию поиска.
	void removeEntities(Selector<Entity> match = nullptr);
	/// @brief Сколько дочерних сущностей удовлетворяют заданному условию?
	int64_t entityCount(Selector<Entity> match = nullptr);
	/// @brief Получить итератор списка сущностей.
	///
	/// @param match условие отбора
	ListIterator entityIterator(Selector<Entity> match = nullptr);
	/// @brief Найти дочернюю сущность по её уникальному идентификатору.
	std::shared_ptr<Entity> findEntityById(const uid& id);
	/// @brief Найти все дочерние сущности с данным именем.
	std::vector<std::shared_ptr<Entity>> findEntitiesByName(const std::string& name);

	operator bool() const;

private:
	void setTypeId(tid typeId);
	std::shared_ptr<List<Entity>> entities();
	void setParent(Entity* parent);
	// Обновить индекс имён вложенных сущностей, добавив новую запись или изменив старую, если она есть.
	void updateNameIndexRecord(std::shared_ptr<Entity> ent, const std::string& name, const std::string& oldName = "");

private:
	std::unique_ptr<Private> _p;
};

/// @brief Исполняемая сущность.
class BASIS_EXPORT Executable : public Entity
{
	struct Private;

public:
	Executable(System* sys);

	virtual ~Executable();
	/// @brief Шаг моделирования.
	void step();
	/// @brief Установить исполняемую функцию.
	void setStepFunction(std::function<void()> func);
	/// @brief Включение/выключение активности данной исполняемой сущности.
	void setActive(bool active = true);
	/// @brief Активна ли данная сущность?
	bool isActive() const;
	/// @brief Распечатать собственное описание.
	virtual void print() override;

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
	/// @brief Получить пространственное положение.
	point3d position() const;
	/// @brief Установить пространственное положение.
	void setPosition(const point3d& pos);
	/// @brief Получить вектор ориентации в пространстве.
	point3d orientation() const;
	/// @brief Установить вектор ориентации в пространстве.
	void setOrientation(const point3d& orient);
	/// @brief Получить размер.
	double size() const;
	/// @brief Установить размер.
	void setSize(double sz);

private:
	std::unique_ptr<Private> _p;
};

/// @brief Публичный интерфейс фабрики сущностей.
class FactoryInterface
{
public:
	/// @brief Создать новый экземпляр данной сущности.
	virtual Entity* newEntity(System*) = 0;
	/// @brief Получить идентификатор типа сущности, порождаемой этой фабрикой.
	virtual tid typeId() const = 0;
	/// @brief Получить имя типа сущности, порождаемой этой фабрикой.
	virtual std::string typeName() const = 0;
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
	/// @brief Register specified class of entities.
	template<class T> bool registerEntity();
	/// @brief Unregister specified class of entities.
	template<class T> bool unregisterEntity();
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
	/// @brief Read and execute commands from external batch file.
	void executeBatchFile(const std::string& path);
	/// @brief Слот для приёма команды управления от внешнего источника (например, с консоли).
	void onCommand(const std::string& command);
	/// @brief Pause the main loop.
	void pause();
	/// @brief Resume the main loop.
	void resume();
	/// @brief Is main loop paused?
	bool isPaused() const;
	/// @brief Make n steps forward.
	void doSteps(uint64_t n = 1);
	/// Вывести приветствие на консоль.
	void printWelcome() const;
	/// @brief Вывести краткую справку по командам.
	void printUsage() const;
	/// @brief Generate a random integer in range [from, to].
	int randomInt(int from, int to);
	/// @brief Generate a random double in range [from, to].
	double randomDouble(double from, double to);
	/// @brief Get steps count passed from start.
	int64_t stepsFromStart() const;

private:
	System();
	~System();
	bool addFactory(FactoryInterface* f);
	bool removeFactory(tid typeId);

private:
	/// @brief Обработка команды 'to'.
	void onCommand_to(const std::string& command);

private:
	Private* _p = nullptr;
};

/// @brief Функция отрезает от строки str всё до подстроки what включительно.
void BASIS_EXPORT cutoff(std::string& str, const std::string& what);

///////////////////////////////////////////////////////////////////////////////
// РЕАЛИЗАЦИЯ ШАБЛОННЫХ ФУНКЦИЙ И КЛАССОВ, ВСПОМОГАТЕЛЬНЫЕ КЛАССЫ И ФУНКЦИИ
///////////////////////////////////////////////////////////////////////////////

template <class T>
ListItem<T>::ListItem(std::shared_ptr<T>& value)
	:_value(value)
{
}

template<class T>
std::shared_ptr<ListItem<T>> ListItem<T>::next()
{
	return _next;
}

template<class T>
std::shared_ptr<ListItem<T>> ListItem<T>::prev()
{
	return _prev;
}

template<class T>
std::shared_ptr<T> ListItem<T>::value() const
{
	return _value;
}

template<class T>
std::shared_ptr<ListItem<T>> List<T>::pushBack(std::shared_ptr<T>& value)
{
	ListItem<T>* newItem = new ListItem<T>(value);
	std::shared_ptr<ListItem<T>> newItemPtr = std::shared_ptr<ListItem<T>>(newItem);
	if (_tail == nullptr) {
		_head = newItemPtr;
		_tail = _head;
	}
	else {
		_tail->_next = newItemPtr;
		newItemPtr->_prev = _tail;
		_tail = newItemPtr;
	}

	++_size;

	return newItemPtr;
}

template<class T>
std::shared_ptr<ListItem<T>> List<T>::remove(std::shared_ptr<ListItem<T>>& item)
{
	if (item == _head)
		_head = item->_next;
	if (item == _tail)
		_tail = item->_prev;

	if (item->_prev)
		item->_prev->_next = item->_next;
	if (item->_next)
		item->_next->_prev = item->_prev;
	std::shared_ptr<ListItem<T>> ret = item->_next;
	item->_next = nullptr;
	item->_prev = nullptr;

	--_size;

	return ret;
}

template <class T>
std::shared_ptr<ListItem<T>> List<T>::head() const
{
	return _head;
}

template <class T>
std::shared_ptr<ListItem<T>> List<T>::tail() const
{
	return _tail;
}

template <class T>
int64_t List<T>::size() const
{
	return _size;
}

template<class T>
std::shared_ptr<T> Entity::as()
{
	return std::dynamic_pointer_cast<T>(as(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Entity::addFacet()
{
	return std::dynamic_pointer_cast<T>(addFacet(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Entity::newEntity()
{
	return std::dynamic_pointer_cast<T>(newEntity(TYPEID(T)));
}

/// @brief Фабрика сущностей.
/// Предназначена для динамического создания сущностей заданного типа.
template <class T>
class Factory : public FactoryInterface
{
public:
	Factory();
	/// @brief Создать новый экземпляр данной сущности.
	virtual Entity* newEntity(System* sys) override;
	/// @brief Получить идентификатор типа сущности, порождаемой этой фабрикой.
	virtual tid typeId() const override;
	/// @brief Получить имя типа сущности, порождаемой этой фабрикой.
	virtual std::string typeName() const override;

protected:
	T* _new(System* sys) { return new T(sys); }

private:
	std::string _typeName;
};

template <class T>
Factory<T>::Factory()
{
	_typeName = boost::typeindex::type_id<T>().pretty_name();
	cutoff(_typeName, ":");
	cutoff(_typeName, "class ");
}

template <class T>
Entity* Factory<T>::newEntity(System* sys)
{
	Entity* ent = static_cast<Entity*>(_new(sys));
	ent->setTypeId(TYPEID(T));

	return ent;
}

template <class T>
tid Factory<T>::typeId() const
{
	return TYPEID(T);
}

template <class T>
std::string Factory<T>::typeName() const
{
	return _typeName;
}

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

template<class T> bool System::unregisterEntity()
{
	if (!isEntityRegistered(TYPEID(T)))
		return true;

	return removeFactory(TYPEID(T));
}

/// @brief Проверка, что сущность является исполняемой.
static auto check_executable([](std::shared_ptr<Entity> ent)->bool {
	if (!ent)
		return false;
	return ent->hasFacet(TYPEID(Executable));
});

namespace Test {
	/// @brief Модульные тесты.
	bool BASIS_EXPORT test();

} // namespace Test
} // namespace Basis
