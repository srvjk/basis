#include "basis.h"
#include "basis_private.h"
#include "iterable.h"

bool Basis::test()
{
	if (!Iterable::test())
		return false;

	return true;
}
