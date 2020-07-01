#include "basis.h"
#include "basis_private.h"
#include <functional>

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

//struct Callable::Private {
//
//};

class Callable : public Basis::Entity
{
public:
	Callable(Basis::System* s) : Entity(s)
	{
	}

public:
	std::function<void()> stepFunction = nullptr;
};

class InnerEntity : public Basis::Entity
{
public:
	InnerEntity(Basis::System* s) : Entity(s)
	{
		addFacet(TYPEID(Enumerable));
		addFacet(TYPEID(Spatial));
		addFacet(TYPEID(Callable));
		auto cl = as<Callable>();
		if (cl)
			cl->stepFunction = std::bind(&InnerEntity::step, this);
	}

	void step()
	{
		int kkk = 0;
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
		//auto worker = fct->as<Executable>();
		//if (worker)
		//	worker->setStepFunction(std::bind(&Worker::step, this));
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
	sys->registerEntity<Callable>();
	sys->registerEntity<InnerEntity>();
	sys->registerEntity<OuterEntity>();
	sys->registerEntity<Worker>();

	// test entity adding and removing
	{
		auto ent = sys->newEntity(TYPEID(OuterEntity));
		if (sys->entityCount() != 1)
			return false;

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// test nested iterators
	{
		int n = 10;
		for (int i = 0; i < n; ++i)
			sys->newEntity(TYPEID(InnerEntity));
		if (sys->entityCount() != n)
			return false;

		for (auto iter1 = sys->entityIteratorNew(); iter1.hasMore(); iter1.next()) {
			auto ent1 = iter1.value();
			for (auto iter2 = sys->entityIteratorNew(); iter2.hasMore(); iter2.next()) {
				auto ent2 = iter2.value();
				auto spat2 = ent2->as<Spatial>();
				auto call2 = ent2->as<Callable>();
				//if (exe2) {
				//	if (exe2->isActive()) {
				//		exe2->step();
				//	}
				//}
			}
		}

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// test nested iterators
	//{
	//	int n = 10;
	//	for (int i = 0; i < n; ++i)
	//		sys->newEntity(TYPEID(Worker));
	//	if (sys->entityCount() != n)
	//		return false;

	//	for (auto iter1 = sys->entityIteratorNew(); iter1.hasMore(); iter1.next()) {
	//		auto ent1 = iter1.value();
	//		for (auto iter2 = sys->entityIteratorNew(); iter2.hasMore(); iter2.next()) {
	//			auto ent2 = iter2.value();
	//			auto exe2 = ent2->as<Executable>();
	//			//if (exe2) {
	//			//	if (exe2->isActive()) {
	//			//		exe2->step();
	//			//	}
	//			//}
	//		}
	//	}

	//	sys->removeEntities();
	//	if (sys->entityCount() != 0)
	//		return false;
	//}

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

	if (sys->entityCount() == 0)
		return false;
	sys->removeEntities();
	if (sys->entityCount() != 0)
		return false;

	if (!sys->unregisterEntity<Enumerable>())
		return false;
	if (!sys->unregisterEntity<InnerEntity>())
		return false;
	if (!sys->unregisterEntity<OuterEntity>())
		return false;
	if (!sys->unregisterEntity<Worker>())
		return false;

	//// Тест 1. Обход без условия.
	//// Подсчитываем сумму всех чисел от a1 до aN и затем
	//// проверяем по формуле суммы арифметической прогрессии.
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

	//// Тест 2. Обход с условием.
	//// Подсчитываем сумму всех четных чисел от a1 до aN и затем
	//// проверяем по формуле суммы арифметической прогрессии.
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

	//// Тест 3. Несколько одновременно активных обходов списка.
	//// Первый обход дает значение N, а второй - сумму первых N членов прогрессии.
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

	//// Тест 4. Несколько одновременно активных обходов списка.
	//// Первый обход даёт разность прогрессии, а второй - сумму прогрессии с этой разностью.
	//{
	//	std::list<int> lst;
	//	int a1 = 1;
	//	int aN = 20;
	//	for (int i = a1; i <= aN; ++i) {
	//		lst.push_back(i);
	//	}

	//	ListIterator<int> outer(lst);
	//	while (!outer.finished()) {
	//		int d = outer.value(); // это будет разность прогрессии

	//		ListIterator<int> inner(lst);
	//		inner.setSelector(
	//			[d](int i)->bool {
	//			if ((i - 1) % d == 0)
	//				return true; // выбираем числа, идущие через d
	//			return false;
	//		});

	//		int sum = 0;
	//		int n = 0; // для подсчета количества суммируемых членов прогрессии
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

	return true; // TODO удалять все тестовые сущности!
}

bool Basis::test()
{
	if (!IterableTest())
		return false;

	return true;
}
