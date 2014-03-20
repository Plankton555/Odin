#pragma once

#include "Common.h"


class DataModule
{
public:
	static void init();
	static void destroy();
	static int getNrFuzzyValues();
	static std::map<const char*,int*>* getFuzzyValues();
	static int loaded;
private:
	static std::map<const char*,int*> *units;
	
};