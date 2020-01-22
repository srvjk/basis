#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <boost/type_index.hpp>

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
	struct Private;

public:
	Entity();
	virtual ~Entity();
	/// Получить идентификатор типа этой сущности.
	tid typeId() const;
	/// Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	Entity* addFacet(tid protoTypeId);
	/// Добавить к этой сущности новую грань.
	///
	/// Грань должна быть реализацией ранее созданного прототипа. Запрещено использование
	/// в качестве граней сущностей, создаваемых с нуля, чтобы не поощрять размножение однотипных
	/// сущностей.
	Entity* addFacet(Entity* prototype);
	/// Выяснить, имеет ли сущность прототип или является корневой.
	bool hasPrototype() const;

protected:
	//virtual void save();
	//virtual bool load();

private:
	void setTypeId(tid typeId);

private:
	std::unique_ptr<Private> _p;
};

/**
* @brief Скетч - исполняемый сценарий работы.
*/
class BASIS_EXPORT Executable : public Entity
{
	struct Private;

public:
	Executable();
	virtual ~Executable();
	void step();
	void setStepFunction(std::function<void()> func);

private:
	std::unique_ptr<Private> _p;
};

/**
* @brief Публичный интерфейс фабрики сущностей.
*/
class FactoryInterface
{
public:
	virtual Entity* newEntity() = 0;
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
	T* _new() { return new T; }

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

	Entity* newEntity()
	{
		Entity* ent = static_cast<Entity*>(_new());
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
	* @brief Создать сущность заданного типа, не имеющую прототипа.
	*/
	std::shared_ptr<Entity> newEntity(tid typeId);

	/**
	* @brief Создать сущность по заданному прототипу.
	*
	* Будет автоматически выведен тип создаваемой сущности, созданы её грани и свойства.
	*/
	std::shared_ptr<Entity> newEntity(Entity* prototype);

	/**
	* @brief Найти сущность, удовлетворяющую заданному критерию.
	*/
	std::shared_ptr<Entity> findEntity(std::function<bool(Entity*)> match);

	/**
	* @brief Найти все сущности, удовлетворяющие заданному критерию.
	*/
	std::vector<std::shared_ptr<Entity>> findEntities(std::function<bool(Entity*)> match);

private:
	bool addFactory(FactoryInterface* f);

public:
	std::unique_ptr<Private> _p = nullptr;
};

template<class T> bool System::registerEntity()
{
	if (isEntityRegistered(TYPEID(T)))
		return false;

	// создаём новую фабрику для сущностей этого типа
	return addFactory(new Factory<T>());
}

} // namespace Basis