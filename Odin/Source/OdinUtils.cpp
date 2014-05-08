#include "Common.h"
#include "OdinUtils.h"


OdinUtils::OdinUtils()  
	: gameID(-1)
	, updateID(true)
	, predictTimePeriodsAhead(TIME_PERIODS_AHEAD) {}
OdinUtils::~OdinUtils() {}

// get an instance of this
OdinUtils & OdinUtils::Instance() 
{
	static OdinUtils instance;
	return instance;
}

