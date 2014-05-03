#include "Common.h"
#include "TransportManager.h"

TransportManager::TransportManager()  { }

void TransportManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & transportUnits = getUnits();

	if (transportUnits.empty())
	{
		return;
	}
}