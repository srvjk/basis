#include "iterables.h"

bool Iterable::test()
{
	std::list<std::shared_ptr<int>> lst;
	for (int i = 0; i <= 100; ++i) {
		lst.push_back(std::make_shared<int>(i));
	}

	int n = 0;
	int sum = 0;
	ListIterator<int> iter(
		lst,
		[](int *i)->bool {
		if ((*i) % 2 == 0)
			return true;
		return false;
	});

	while (!iter.isDone()) {
		iter.value();
	}
}
