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

using tid = size_t; // ��������� ��� �������������� ����

/**
@brief ������� ����� ���� ���������.
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
* @brief �������� - ������� ����� ��� ���� �������� � �������.
*/
class BASIS_EXPORT Entity
{
	template <class T> friend class Factory;
	friend class System;
	struct Private;

public:
	Entity();
	virtual ~Entity();
	/// �������� ������������� ���� ���� ��������.
	tid typeId() const;
	/// �������� � ���� �������� ����� �����.
	///
	/// ����� ������ ���� ����������� ����� ���������� ���������. ��������� �������������
	/// � �������� ������ ���������, ����������� � ����, ����� �� �������� ����������� ����������
	/// ���������.
	Entity* addFacet(tid protoTypeId);
	/// �������� � ���� �������� ����� �����.
	///
	/// ����� ������ ���� ����������� ����� ���������� ���������. ��������� �������������
	/// � �������� ������ ���������, ����������� � ����, ����� �� �������� ����������� ����������
	/// ���������.
	Entity* addFacet(Entity* prototype);
	/// ��������, ����� �� �������� �������� ��� �������� ��������.
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
* @brief ����� - ����������� �������� ������.
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
* @brief ��������� ��������� ������� ���������.
*/
class FactoryInterface
{
public:
	virtual Entity* newEntity() = 0;
	virtual tid typeId() const = 0;
	virtual std::string typeName() const = 0;
};

/**
* @brief ������� ���������.
*
* ������������� ��� ������������� �������� ��������� ��������� ����.
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
	* @brief ��������� ��� ������ �� ��������� ����.
	*
	* ���� ����� ���� ���� ������, ���� �����������.
	* @param recursive ������������� ��������� ����������
	* @return ���������� ����������� �������
	*/
	int loadModules(const std::string& path, bool recursive = false);

	/**
	* @brief ����������� �������� ��������� ���� � �������.
	*/
	template<class T> bool registerEntity();

	/**
	* @brief ���������������� �� � ������� �������� ������� ����? 
	*/
	bool isEntityRegistered(tid typeId) const;

	/**
	* @brief ���������� ���������� ������������������ ����� ���������.
	*/
	int64_t entityTypesCount() const;

	/**
	* @brief ������� �������� ��������� ����, �� ������� ���������.
	*/
	std::shared_ptr<Entity> newEntity(tid typeId);

	/**
	* @brief ������� �������� �� ��������� ���������.
	*
	* ����� ������������� ������� ��� ����������� ��������, ������� � ����� � ��������.
	*/
	std::shared_ptr<Entity> newEntity(Entity* prototype);

	/**
	* @brief ����� ��������, ��������������� ��������� ��������.
	*/
	std::shared_ptr<Entity> findEntity(std::function<bool(Entity*)> match);

	/**
	* @brief ����� ��� ��������, ��������������� ��������� ��������.
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

	// ������ ����� ������� ��� ��������� ����� ����
	return addFactory(new Factory<T>());
}

} // namespace Basis