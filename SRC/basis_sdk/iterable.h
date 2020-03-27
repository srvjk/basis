#pragma once

#include <list>
#include <memory>
#include <functional>
#include <stdexcept>

namespace Iterable 
{
	template <class T>
	using Selector = std::function<bool(T)>;

	/// @brief ����������� ��������.
	template <class T>
	class Iterator
	{
	public:
		Iterator();
		/// �������� ������� �������.
		T value();
		/// ������� � ���������� ��������.
		void next();
		/// ��������� ������� ����� ������.
		bool finished() const;
		/// ���������� ������� ������.
		void setSelector(std::function<bool(T)> selector);
		/// �������� ������� ������, ������������� ��� ����� ���������.
		std::function<bool(T)> selector() const;

	protected:
		virtual T _value() = 0;
		virtual void _next() = 0;
		virtual bool _finished() const = 0;
		virtual void _reset() = 0;

	protected:
		std::function<bool(T)> _selector = nullptr; /// ������� ������
	};

	/// @brief �������� ������.
	template <class T>
	class ListIterator : public Iterator<T>
	{
	public:
		ListIterator(std::list<T> &lst);

	protected:
		T _value() override;
		void _next() override;
		bool _finished() const override;
		void _reset() override;

	private:
		std::list<T> _list;
		typename std::list<T>::iterator _position;
	};

	template <class T>
	using IteratorPtr = std::shared_ptr<Iterator<T>>;

	/// ������ ��������� ������.
	bool test();

	// ------------------------------------- ���������� ---------------------------------------

	template <class T>
	Iterator<T>::Iterator()
	{}

	template <class T>
	T Iterator<T>::value()
	{
		// ���� ������� ������ �� ������, ������ ���������� ������� �������
		if (_selector == nullptr)
			return _value();

		// ���� ������� ������ ������:
		while (!_finished()) {
			T val = _value();
			if (_selector(val))
				return val; // ok, ������� �������������
			// ������� �� �������������, ��������� � ���������� ��������
			_next();
		}

		return T(); // TODO ������������ std::optional? ����� ��� ������� ����� ����� int ����� ���������� �������������
	}

	template <class T>
	void Iterator<T>::next()
	{
		if (_finished())
			return;

		// ���� ������� �� ������, ������ ������������ � ���������� ��������
		if (_selector == nullptr) {
			_next();
			return;
		}

		// ���� ������� ������, ������������ �� ���������� ����������� ��������
		// ��� �� ����� ������
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
	void Iterator<T>::setSelector(std::function<bool(T)> selector)
	{
		_reset();
		_selector = selector;
		value(); // ��������� �� ������� ��������, ���������������� �������
	}

	template <class T>
	std::function<bool(T)> Iterator<T>::selector() const
	{
		return _selector;
	}

	template <class T>
	ListIterator<T>::ListIterator(std::list<T> &lst) :
		Iterator<T>(),
		_list(lst)
	{
		_position = _list.begin();
	}

	template <class T>
	bool ListIterator<T>::_finished() const
	{
		if (_position != _list.end())
			return false;

		return true;
	}

	template <class T>
	T ListIterator<T>::_value()
	{
		if (_finished())
			return T();

		return *_position;
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
		_position = _list.begin();
	}
} // namespace Iterable
