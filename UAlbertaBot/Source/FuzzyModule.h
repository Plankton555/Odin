#pragma once

#include "Common.h"


class FuzzyModule
{
public:
	static void init();
	static void destroy();
	static int getFuzzyNr(int nr, std::string name);
private:
	static std::map<const char*,int*> *units;
	static int loaded;
};