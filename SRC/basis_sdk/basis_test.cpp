#include "basis.h"
#include "basis_private.h"
#include <functional>

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

// Tests for general functionality.
bool doGeneralTests()
{
	Basis::System* sys = System::instance();

	sys->registerEntity<Enumerable>();
	sys->registerEntity<InnerEntity>();
	sys->registerEntity<OuterEntity>();
	sys->registerEntity<Worker>();

	// test entity adding and removing
	{
		auto ent = sys->newEntity(TYPEID(Basis::Test::OuterEntity));
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
			}
		}

		sys->removeEntities();
		if (sys->entityCount() != 0)
			return false;
	}

	// test both nested iterators and executable entities
	{
		int n = 10;
		for (int i = 0; i < n; ++i)
			sys->newEntity(TYPEID(Worker));
		if (sys->entityCount() != n)
			return false;

		for (auto iter1 = sys->entityIteratorNew(); iter1.hasMore(); iter1.next()) {
			auto ent1 = iter1.value();
			for (auto iter2 = sys->entityIteratorNew(); iter2.hasMore(); iter2.next()) {
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

	return true;
}

bool Basis::Test::test()
{
	if (!doGeneralTests())
		return false;

	return true;
}
