#pragma once

#include <list>
#include <memory>
#include <functional>

namespace Iterable {

	/// @brief јбстрактный итератор.
	template <class T>
	class Iterator
	{
	public:
		Iterator(std::function<bool(T)> selector = nullptr);
		/// ѕолучить текущий элемент.
		T value();
		/// ѕерейти к следующему элементу.
		void next();
		/// ѕроверить условие конца списка.
		bool isDone() const;

	protected:
		virtual T _value() = 0;
		virtual void _next() = 0;
		virtual bool _isDone() const = 0;

	protected:
		std::function<bool(T)> _selector = nullptr; /// условие отбора
	};

	/// @brief »тератор списка.
	template <class T>
	class ListIterator : public Iterator<T>
	{
	public:
		ListIterator(std::list<T> &lst, std::function<bool(T)> selector = nullptr);

	protected:
		T _value() override;
		void _next() override;
		bool _isDone() const override;

	private:
		std::list<T> _list;
		typename std::list<T>::iterator _position;
	};

	template <class T>
	using IteratorPtr = std::shared_ptr<Iterator<T>>;

	// ------------------------------------- Implementation ---------------------------------------

	template <class T>
	Iterator<T>::Iterator(std::function<bool(T)> selector) :
		_selector(selector)
	{}

	template <class T>
	T Iterator<T>::value()
	{
		// если условие выбора не задано, просто возвращаем текущий элемент
		if (_selector == nullptr)
			return _value();

		// если условие выбора задано:
		while (!_isDone()) {
			T val = _value();
			if (_selector(val))
				return val; // ok, условие удовлетворено
			// условие не удовлетворено, переходим к следующему элементу
			_next();
		}
	}

	template <class T>
	void Iterator<T>::next()
	{
		if (_isDone())
			return;

		// если условие не задано, просто перемещаемс€ к следующему элементу
		if (_selector == nullptr) {
			_next();
			return;
		}

		// если условие задано, прокручиваем до следующего подход€щего элемента
		// или до конца списка
		while (!_isDone()) {
			_next();
			if (_isDone())
				break;
			if (_selector(_value()))
				break;
		}
	}

	template <class T>
	bool Iterator<T>::isDone() const
	{
		return _isDone();
	}

	template <class T>
	ListIterator<T>::ListIterator(std::list<T> &lst, std::function<bool(T)> selector) :
		Iterator<T>(selector),
		_list(lst)
	{
		_position = _list.begin();
	}

	template <class T>
	bool ListIterator<T>::_isDone() const
	{
		if (_position != _list.end())
			return false;

		return true;
	}

	template <class T>
	T ListIterator<T>::_value()
	{
		if (_isDone())
			return nullptr;

		return *_position;
	}

	template <class T>
	void ListIterator<T>::_next()
	{
		if (_isDone())
			return;

		++_position;
	}

	/// ѕрогон модульных тестов.
	bool test();

} // namespace Iterable
