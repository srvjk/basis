#pragma once

#include <memory>
#include <string>
#include <mutex>

#ifdef PLATFORM_WINDOWS
#  ifdef BASIS_LIB
#    define BASIS_EXPORT __declspec(dllexport)
#  else
#    define BASIS_EXPORT __declspec(dllimport)
#  endif
#else
#  define BASIS_EXPORT
#endif

namespace Basis
{
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
protected:
	//virtual void save();
	//virtual bool load();
};

/**
* @brief ����� - ����������� �������� ������.
*/
class BASIS_EXPORT Sketch : public Entity
{

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

public:
	std::unique_ptr<Private> _p = nullptr;
};

} // namespace Basis