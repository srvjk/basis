#pragma once

#include <list>
#include <memory>
#include <functional>
#include <stdexcept>

namespace Iterable 
{
	template <class T>
	using Selector = std::function<bool(T)>;

	/// @brief Абстрактный итератор.
	template <class T>
	class Iterator
	{
	public:
		Iterator(std::function<bool(T)> selector = nullptr);
		/// Получить текущий элемент.
		T value();
		/// Перейти к следующему элементу.
		void next();
		/// Проверить условие конца списка.
		bool isDone() const;
		/// Установить условие отбора.
		void setSelector(std::function<bool(T)> selector);
		/// Получить условие отбора, установленное для этого итератора.
		std::function<bool(T)> selector() const;

	protected:
		virtual T _value() = 0;
		virtual void _next() = 0;
		virtual bool _isDone() const = 0;

	protected:
		std::function<bool(T)> _selector = nullptr; /// условие отбора
	};

	/// @brief Итератор списка.
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

	/// Прогон модульных тестов.
	bool test();

	// ------------------------------------- Реализация ---------------------------------------

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

		// если условие не задано, просто перемещаемся к следующему элементу
		if (_selector == nullptr) {
			_next();
			return;
		}

		// если условие задано, прокручиваем до следующего подходящего элемента
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
	void Iterator<T>::setSelector(std::function<bool(T)> selector)
	{
		_selector = selector;
	}

	template <class T>
	std::function<bool(T)> Iterator<T>::selector() const
	{
		return _selector;
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
			throw std::out_of_range("list iterator is out of range");

		return *_position;
	}

	template <class T>
	void ListIterator<T>::_next()
	{
		if (_isDone())
			return;

		++_position;
	}
} // namespace Iterable
