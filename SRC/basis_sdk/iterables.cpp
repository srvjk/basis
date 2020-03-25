#include "iterables.h"

bool Iterable::test()
{
	// Тест 1. Обход без условия.
	// Подсчитываем сумму всех чисел от a1 до aN и затем
	// проверяем по формуле суммы арифметической прогрессии.
	{
		std::list<int> lst;
		int a1 = 0;
		int aN = 20;
		for (int i = a1; i <= aN; ++i) {
			lst.push_back(i);
		}

		ListIterator<int> iter(lst);

		int n = 0;
		int sum = 0;
		while (!iter.isDone()) {
			sum += iter.value();
			++n;
			iter.next();
		}

		int rightSum = (a1 + aN) * n / 2;
		if (sum != rightSum)
			return false;
	}

	// Тест 2. Обход с условием.
	// Подсчитываем сумму всех четных чисел от a1 до aN и затем
	// проверяем по формуле суммы арифметической прогрессии.
	{
		std::list<int> lst;
		int a1 = 0;
		int aN = 100;
		for (int i = a1; i <= aN; ++i) {
			lst.push_back(i);
		}

		ListIterator<int> iter(
			lst,
			[](int i)->bool {
			if (i % 2 == 0)
				return true;
			return false;
		});

		int n = 0;
		int sum = 0;
		while (!iter.isDone()) {
			sum += iter.value();
			++n;
			iter.next();
		}

		int rightSum = (a1 + aN) * n / 2;
		if (sum != rightSum)
			return false;
	}

	// Тест 3. Несколько одновременно активных обходов списка.
	// Первый обход дает значение N, а второй - сумму первых N членов прогрессии.
	{
		std::list<int> lst;
		int a1 = 1;
		int aN = 20;
		for (int i = a1; i <= aN; ++i) {
			lst.push_back(i);
		}

		ListIterator<int> outer(lst);
		while (!outer.isDone()) {
			int n = outer.value();
			if (n > 1) {
				ListIterator<int> inner(
					lst,
					[n](int i)->bool {
					if (i <= n)
						return true;
					return false;
				});

				int sum = 0;
				while (!inner.isDone()) {
					sum += inner.value();
					inner.next();
				}

				int rightSum = ((a1 + n) * n) / 2;
				if (sum != rightSum)
					return false;
			}

			outer.next();
		}
	}

	// Тест 4. Несколько одновременно активных обходов списка.
	// Первый обход даёт разность прогрессии, а второй - сумму прогрессии с этой разностью.
	{
		std::list<int> lst;
		int a1 = 1;
		int aN = 20;
		for (int i = a1; i <= aN; ++i) {
			lst.push_back(i);
		}

		ListIterator<int> outer(lst);
		while (!outer.isDone()) {
			int d = outer.value(); // это будет разность прогрессии

			ListIterator<int> inner(lst);
			inner.setSelector(
				[d](int i)->bool {
					if ((i - 1) % d == 0)
						return true; // выбираем числа, идущие через d
					return false;
			});

			int sum = 0;
			int n = 0; // для подсчета количества суммируемых членов прогрессии
			while (!inner.isDone()) {
				sum += inner.value();
				++n;
				inner.next();
			}

			int rightSum = ((2 * a1 + d * (n - 1)) * n ) / 2;
			if (sum != rightSum)
				return false;

			outer.next();
		}
	}

	return true;
}
