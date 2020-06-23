#include "basis.h"
#include "basis_private.h"

using namespace Basis;

class ListWrapper
{
public:
	ListWrapper(std::shared_ptr<std::list<int>> lst) : _lst(lst)
	{
	}

	void changeList()
	{
		for (auto it = _lst->begin(); it != _lst->end(); ++it) {
			(*it)++;
		}
	}
private:
	std::shared_ptr<std::list<int>> _lst;
};

class Enumerable : public Basis::Entity
{
public:
	Enumerable(Basis::System* s) : Entity(s)
	{
	}

public:
	int num = 0;
};

class InnerEntity : public Basis::Entity
{
public:
	InnerEntity(Basis::System* s) : Entity(s)
	{
		addFacet(TYPEID(Enumerable));
	}
};

class OuterEntity : public Basis::Entity 
{
public:
	OuterEntity(Basis::System* s) : Entity(s)
	{
		// create n of inner entities and add them to internal collection:
		int n = 100;
		for (int i = 0; i < n; ++i) {
			auto ent = newEntity(TYPEID(InnerEntity));
			auto enumerable = ent->as<Enumerable>();
			if (enumerable)
				enumerable->num = i;
		}
	}
};

class Worker : public Basis::Entity
{
public:
	Worker(Basis::System* s) : Entity(s)
	{
		auto fct = addFacet(TYPEID(Executable));
		auto worker = fct->as<Executable>();
		if (worker)
			worker->setStepFunction(std::bind(&Worker::step, this));
	}

	void step()
	{
		int kkk = 0;
	}
};

bool IterableTest()
{
	Basis::System* sys = System::instance();

	sys->registerEntity<Enumerable>();
	sys->registerEntity<InnerEntity>();
	sys->registerEntity<OuterEntity>();
	sys->registerEntity<Worker>();

	int i = 0;
	auto ent = sys->newEntity(TYPEID(OuterEntity));
	for (auto iter = ent->entityIteratorNew(); iter.hasMore(); iter.next()) {
		auto inner = iter.value();
		auto enumerable = inner->as<Enumerable>();
		if (!enumerable)
			return false;
		if (enumerable->num != i)
			return false;

		++i;
	}

	int numIter = 100;
	for (int i = 0; i < numIter; ++i) {
		sys->step();
	}

	//// ���� 1. ����� ��� �������.
	//// ������������ ����� ���� ����� �� a1 �� aN � �����
	//// ��������� �� ������� ����� �������������� ����������.
	//{
	//	std::list<int> lst;
	//	int a1 = 0;
	//	int aN = 20;
	//	for (int i = a1; i <= aN; ++i) {
	//		lst.push_back(i);
	//	}

	//	ListIterator<int> iter(lst);

	//	int n = 0;
	//	int sum = 0;
	//	while (!iter.finished()) {
	//		sum += iter.value();
	//		++n;
	//		iter.next();
	//	}

	//	int rightSum = (a1 + aN) * n / 2;
	//	if (sum != rightSum)
	//		return false;
	//}

	//// ���� 2. ����� � ��������.
	//// ������������ ����� ���� ������ ����� �� a1 �� aN � �����
	//// ��������� �� ������� ����� �������������� ����������.
	//{
	//	std::list<int> lst;
	//	int a1 = 0;
	//	int aN = 100;
	//	for (int i = a1; i <= aN; ++i) {
	//		lst.push_back(i);
	//	}

	//	ListIterator<int> iter(lst);
	//	iter.setSelector(
	//		[](int i)->bool {
	//		if (i % 2 == 0)
	//			return true;
	//		return false;
	//	});

	//	int n = 0;
	//	int sum = 0;
	//	while (!iter.finished()) {
	//		sum += iter.value();
	//		++n;
	//		iter.next();
	//	}

	//	int rightSum = (a1 + aN) * n / 2;
	//	if (sum != rightSum)
	//		return false;
	//}

	//// ���� 3. ��������� ������������ �������� ������� ������.
	//// ������ ����� ���� �������� N, � ������ - ����� ������ N ������ ����������.
	//{
	//	std::list<int> lst;
	//	int a1 = 1;
	//	int aN = 20;
	//	for (int i = a1; i <= aN; ++i) {
	//		lst.push_back(i);
	//	}

	//	ListIterator<int> outer(lst);
	//	while (!outer.finished()) {
	//		int n = outer.value();
	//		if (n > 1) {
	//			ListIterator<int> inner(lst);
	//			inner.setSelector(
	//				[n](int i)->bool {
	//				if (i <= n)
	//					return true;
	//				return false;
	//			});

	//			int sum = 0;
	//			while (!inner.finished()) {
	//				sum += inner.value();
	//				inner.next();
	//			}

	//			int rightSum = ((a1 + n) * n) / 2;
	//			if (sum != rightSum)
	//				return false;
	//		}

	//		outer.next();
	//	}
	//}

	//// ���� 4. ��������� ������������ �������� ������� ������.
	//// ������ ����� ��� �������� ����������, � ������ - ����� ���������� � ���� ���������.
	//{
	//	std::list<int> lst;
	//	int a1 = 1;
	//	int aN = 20;
	//	for (int i = a1; i <= aN; ++i) {
	//		lst.push_back(i);
	//	}

	//	ListIterator<int> outer(lst);
	//	while (!outer.finished()) {
	//		int d = outer.value(); // ��� ����� �������� ����������

	//		ListIterator<int> inner(lst);
	//		inner.setSelector(
	//			[d](int i)->bool {
	//			if ((i - 1) % d == 0)
	//				return true; // �������� �����, ������ ����� d
	//			return false;
	//		});

	//		int sum = 0;
	//		int n = 0; // ��� �������� ���������� ����������� ������ ����������
	//		while (!inner.finished()) {
	//			sum += inner.value();
	//			++n;
	//			inner.next();
	//		}

	//		int rightSum = ((2 * a1 + d * (n - 1)) * n) / 2;
	//		if (sum != rightSum)
	//			return false;

	//		outer.next();
	//	}
	//}

	return true; // TODO ������� ��� �������� ��������!
}

bool Basis::test()
{
	if (!IterableTest())
		return false;

	return true;
}
