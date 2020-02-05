#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <boost/type_index.hpp>
#include <boost/uuid/uuid.hpp>

#ifdef PLATFORM_WINDOWS
#  ifdef BASIS_LIB
#    define BASIS_EXPORT __declspec(dllexport)
#  else
#    define BASIS_EXPORT __declspec(dllimport)
#  endif
#else
#  define BASIS_EXPORT
#endif

#define TYPEID(className) typeid(className).hash_code()

namespace Basis
{

using tid = size_t; // псевдоним для идентификатора типа
using uid = boost::uuids::uuid; // псевдоним для идентификатора сущности

/**
@brief Базовый класс всех синглетов.
*/
template <class T>
class Singleton
{
protected:
	Singleton();
	~Singleton();

public:
	static T* instance();

private:
	static T *_self;
	static std::mutex _mutex;
};

template <class T>
Singleton<T>::Singleton() {}

template <class T>
Singleton<T>::~Singleton() { delete _self; }

template <class T>
T* Singleton<T>::instance()
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (!_self) {
		_self = new T;
	}

	return _self;
}

template <class T> T* Singleton<T>::_self = nullptr;
template <class T> std::mutex Singleton<T>::_mutex;

/**
* @brief Сущность - базовый класс для всех объектов в системе.
*/
class BASIS_EXPORT Entity
{
	template <class T> friend class Factory;
	friend class System;
	friend class Container;
	struct Private;

public:
	Entity(System* sys);
	virtual ~Entity();

	/// Получить идентификатор типа этой сущности.
	tid typeId() const;

	/// Получить собственный идентификатор этой сущности.
	uid id() const;

	/// Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	std::shared_ptr<Entity> addFacet(tid protoTypeId);

	/// Добавить к этой сущности новую грань.
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

	/// Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	template<class T>
	std::shared_ptr<T> addFacet(T* prototype);

	/// Выяснить, имеет ли сущность прототип или является корневой.
	bool hasPrototype() const;

	/**
	* @brief Получить ссылку на систему.
	*/
	System* system() const;

	/**
	* @brief Распечатать собственное описание.
	*/
	virtual void print();

private:
	void setTypeId(tid typeId);

private:
	std::unique_ptr<Private> _p;
};

template<class T>
std::shared_ptr<T> Entity::addFacet() 
{
	return static_pointer_cast<T>(addFacet(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Entity::addFacet(T* prototype)
{
	return static_pointer_cast<T>(addFacet(prototype));
}

/**
* @brief Скетч - исполняемый сценарий работы.
*/
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

/*
* @brief Контейнер - сущность, содержащая другие сущности.
*/
class BASIS_EXPORT Container : public Entity
{
	struct Private;

public:
	Container(System* sys);
	virtual ~Container();

	/**
	* @brief Создать сущность заданного типа, не имеющую прототипа.
	*
	* @attention Если такая сущность уже существует, функция вернет ее, а не создаст новую,
	* поскольку может существовать максимум одна корневая сущность каждого типа!
	*/
	std::shared_ptr<Entity> newEntity(tid typeId);

	/**
	* @brief Создать сущность по заданному прототипу.
	*
	* Будет автоматически выведен тип создаваемой сущности, созданы её грани и свойства.
	*/
	std::shared_ptr<Entity> newEntity(Entity* prototype);

	/**
	* @brief Создать сущность заданного типа, не имеющую прототипа.
	*
	* @attention Если такая сущность уже существует, функция вернет ее, а не создаст новую,
	* поскольку может существовать максимум одна корневая сущность каждого типа!
	*/
	template<class T>
	std::shared_ptr<T> newEntity();

	/**
	* @brief Создать сущность по заданному прототипу.
	*
	* Будет автоматически выведен тип создаваемой сущности, созданы её грани и свойства.
	*/
	template<class T>
	std::shared_ptr<T> newEntity(T* prototype);

	/**
	* @brief Найти первую сущность, удовлетворяющую заданному критерию.
	*/
	std::shared_ptr<Entity> findEntity(std::function<bool(Entity*)> match);

	/**
	* @brief Найти все сущности, удовлетворяющие заданному критерию.
	*/
	std::vector<std::shared_ptr<Entity>> findEntities(std::function<bool(Entity*)> match);

	/**
	* @brief Получить ссылку на исполняемую сущность, если она есть.
	*/
	Executable* executor() const;

	/**
	* @brief Установить сущность с данным id как корневого исполнителя.
	*/
	bool setExecutor(const uid& id);

	/**
	* @brief Распечатать собственное описание.
	*/
	virtual void print();

private:
	std::unique_ptr<Private> _p;
};

template<class T>
std::shared_ptr<T> Container::newEntity()
{
	return dynamic_pointer_cast<T>(newEntity(TYPEID(T)));
}

template<class T>
std::shared_ptr<T> Container::newEntity(T* prototype)
{
	return dynamic_pointer_cast<T>(newEntity(prototype));
}

/**
* @brief Публичный интерфейс фабрики сущностей.
*/
class FactoryInterface
{
public:
	virtual Entity* newEntity(System*) = 0;
	virtual tid typeId() const = 0;
	virtual std::string typeName() const = 0;
};

/**
* @brief Фабрика сущностей.
*
* Предназначена для динамического создания сущностей заданного типа.
*/
template <class T>
class Factory : public FactoryInterface
{
protected:
	T* _new(System* sys) { return new T(sys); }

public:
	Factory()
	{
		_typeName = boost::typeindex::type_id<T>().pretty_name();
		size_t i = _typeName.find_last_of(':');
		if (i != std::string::npos) {
			if (i < _typeName.size()) {
				_typeName = _typeName.substr(i + 1);
			}
		}
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

class BASIS_EXPORT System : public Singleton<System>
{
	struct Private;

public:
	System();
	~System();

	/**
	* @brief Загрузить все модули по заданному пути.
	*
	* Путь может быть либо файлом, либо директорией.
	* @param recursive просматривать вложенные директории
	* @return количество загруженных модулей
	*/
	int loadModules(const std::string& path, bool recursive = false);

	/**
	* @brief Регистрация сущности заданного типа в системе.
	*/
	template<class T> bool registerEntity();

	/**
	* @brief Зарегистрирована ли в системе сущность данного типа? 
	*/
	bool isEntityRegistered(tid typeId) const;

	/**
	* @brief Возвращает количество зарегистрированных типов сущностей.
	*/
	int64_t entityTypesCount() const;

	/**
	* @brief Доступ к корневому контейнеру сущностей.
	*/
	std::shared_ptr<Container> container();

	/**
	* @brief Создать сущность, не добавляя ее в список, не проверяя единственность и т.п.
	*/
	std::shared_ptr<Entity> createEntity(tid typeId);

	/**
	* @brief Установить сущность с данным id как корневого исполнителя.
	*/
	bool setExecutor(const uid& id);

	/**
	* @brief Корневая выполняемая процедура.
	*/
	void step();

	/**
	* @brief Была ли команда завершить вычисления?
	*/
	bool shouldStop() const;

	template<class T>
	std::shared_ptr<T> createEntity();

private:
	bool addFactory(FactoryInterface* f);

public:
	std::unique_ptr<Private> _p = nullptr;
};

template<class T>
std::shared_ptr<T> System::createEntity()
{
	return dynamic_pointer_cast<T>(createEntity(TYPEID(T)));
}

template<class T> bool System::registerEntity()
{
	if (isEntityRegistered(TYPEID(T)))
		return false;

	// создаём новую фабрику для сущностей этого типа
	return addFactory(new Factory<T>());
}

} // namespace Basis