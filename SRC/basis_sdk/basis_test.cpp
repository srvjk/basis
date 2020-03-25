#include "basis.h"
#include "iterable.h"

bool Basis::test()
{
	if (!Iterable::test())
		return false;

	return true;
}