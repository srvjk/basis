#include "basis.h"
#include "iterables.h"

bool Basis::test()
{
	if (!Iterable::test())
		return false;

	return true;
}