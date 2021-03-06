#include "basis.h"
#include "basis_private.h"
#include <functional>
#include <iostream>

using namespace Basis;

namespace Basis {
	namespace Test {
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
				addFacet(TYPEID(Spatial));
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
				{
					auto exe = fct->as<Executable>();
					if (exe)
						exe->setStepFunction(std::bind(&Worker::step, this));
				}
			}

			void step()
			{
			}
		};

	} // namespace Test
} // namespace Basis

using namespace Basis;
using namespace Basis::Test;

// ????? ??????? ???????.
bool doGeneralTests()
{
	Basis::System* sys = System::instance();

	sys->registerEntity<Enumerable>();
	sys->registerEntity<InnerEntity>();
	sys->registerEntity<OuterEntity>();
	sys->registerEntity<Worker>();

	// ?????????? ? ???????? ?????????
	{
		auto ent = sys->newEntity(TYPEID(Basis::Test::OuterEntity));
		if (sys->entityCount() != 1)
			return false;

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// ????? ????????? ?? ??????????????
	{
		int n = 10;
		for (int i = 0; i < n; ++i)
			sys->newEntity(TYPEID(InnerEntity));
		if (sys->entityCount() != n)
			return false;

		for (auto iter = sys->entityIterator(); iter.hasMore(); iter.next()) {
			auto ent1 = iter.value();
			auto ent2 = sys->findEntityById(ent1->id());
			if (ent2 != ent1)
				return false;
		}

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// ????? ????????? ?? ?????
	{
		int nIter = 5;
		for (int i = 0; i < nIter; ++i) {
			// ????????? 1 ???????? ? ?????? "1", 2 ???????? ? ?????? "2" ? ?.?.
			std::string name = std::to_string(i + 1);
			for (int j = 0; j < i + 1; ++j) {
				auto ent = sys->newEntity(TYPEID(InnerEntity));
				ent->setName(name);
			}
		}

		// ?????????, ??? ????????? ? ?????? "cnt" ?????????? ????? cnt
		for (int i = 0; i < nIter; ++i) {
			int cnt = i + 1;
			std::string name = std::to_string(cnt);
			std::vector<std::shared_ptr<Entity>> items = sys->findEntitiesByName(name);
			if (items.size() != cnt)
				return false;
			// ... ? ??? ??? ????????????? ????? ??? "cnt"
			for (auto iter = items.begin(); iter != items.end(); ++iter) {
				auto ent = *iter;
				if (ent->name() != name)
					return false;
			}
		}

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// ???????? ??????????? ????????? ??? ?????????????? ????????
	{
		std::shared_ptr<Entity> ent = nullptr;
		
		ent = sys->newEntity(TYPEID(InnerEntity));
		if (!ent)
			return false;
		ent->setName("first");

		ent = sys->newEntity(TYPEID(InnerEntity));
		if (!ent)
			return false;
		ent->setName("second");

		ent = sys->newEntity(TYPEID(InnerEntity));
		if (!ent)
			return false;
		ent->setName("third");

		if (sys->entityCount() != 3)
			return false;

		std::vector<std::shared_ptr<Entity>> items = sys->findEntitiesByName("second");
		if (items.size() != 1)
			return false;
		auto item = items[0];
		item->setName("middle");

		// ?????????, ??? ????? ?????????? ????????? ?? ??????????
		if (sys->entityCount() != 3)
			return false;

		// ?????????, ??? ?????? ??? ???????? ? ?????? 'second'
		items = sys->findEntitiesByName("second");
		if (!items.empty())
			return false;

		// ?????????, ??? ???? ????? ???? ???????? ? ?????? 'middle'
		items = sys->findEntitiesByName("middle");
		if (items.size() != 1)
			return false;

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// ????????? ?????????
	{
		int n = 10;
		for (int i = 0; i < n; ++i)
			sys->newEntity(TYPEID(InnerEntity));
		if (sys->entityCount() != n)
			return false;

		for (auto iter1 = sys->entityIterator(); iter1.hasMore(); iter1.next()) {
			auto ent1 = iter1.value();
			for (auto iter2 = sys->entityIterator(); iter2.hasMore(); iter2.next()) {
				auto ent2 = iter2.value();
				auto spat2 = ent2->as<Spatial>();
			}
		}

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// ????????? ????????? ? ??????????? ????????
	{
		int n = 10;
		for (int i = 0; i < n; ++i)
			sys->newEntity(TYPEID(Worker));
		if (sys->entityCount() != n)
			return false;

		for (auto iter1 = sys->entityIterator(); iter1.hasMore(); iter1.next()) {
			auto ent1 = iter1.value();
			for (auto iter2 = sys->entityIterator(); iter2.hasMore(); iter2.next()) {
				auto ent2 = iter2.value();
				auto spat2 = ent2->as<Spatial>();
				auto exe2 = ent2->as<Executable>();
				if (exe2) {
					exe2->step();
				}
				else {
					return false;
				}
			}
		}

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	int i = 0;
	auto ent = sys->newEntity(TYPEID(OuterEntity));
	for (auto iter = ent->entityIterator(); iter.hasMore(); iter.next()) {
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

	return true;
}

bool Basis::Test::test()
{
	if (!doGeneralTests())
		return false;

	return true;
}
