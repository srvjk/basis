#include "iterables.h"

bool Iterable::test()
{
	// ���� 1. ������� ��� �������.
	// ������������ ����� ���� ����� �� a1 �� aN � �����
	// ��������� �� ������� ����� �������������� ����������.
	{
		std::list<std::shared_ptr<int>> lst;
		int a1 = 0;
		int aN = 20;
		for (int i = a1; i <= aN; ++i) {
			lst.push_back(std::make_shared<int>(i));
		}

		ListIterator<std::shared_ptr<int>> iter(lst);

		int n = 0;
		int sum = 0;
		while (!iter.isDone()) {
			auto val = iter.value();
			sum += *val;
			++n;
			iter.next();
		}

		int rightSum = (a1 + aN) * n / 2;
		if (sum != rightSum)
			return false;
	}

	// ���� 2. ������� � ��������.
	// ������������ ����� ���� ������ ����� �� a1 �� aN � �����
	// ��������� �� ������� ����� �������������� ����������.
	{
		std::list<std::shared_ptr<int>> lst;
		int a1 = 0;
		int aN = 100;
		for (int i = a1; i <= aN; ++i) {
			lst.push_back(std::make_shared<int>(i));
		}

		ListIterator<std::shared_ptr<int>> iter(
			lst,
			[](std::shared_ptr<int> i)->bool {
			if ((*i) % 2 == 0)
				return true;
			return false;
		});

		int n = 0;
		int sum = 0;
		while (!iter.isDone()) {
			auto val = iter.value();
			sum += *val;
			++n;
			iter.next();
		}

		int rightSum = (a1 + aN) * n / 2;
		if (sum != rightSum)
			return false;
	}

	return true;
}
