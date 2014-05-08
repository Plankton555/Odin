#include "Common.h"
#include "OdinUtils.h"


OdinUtils::OdinUtils()  
	: gameID(-1)
	, updateID(true)
	, predictTimePeriodsAhead(3) {}
OdinUtils::~OdinUtils() {}

// get an instance of this
OdinUtils & OdinUtils::Instance() 
{
	static OdinUtils instance;
	return instance;
}

