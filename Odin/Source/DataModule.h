#pragma once

#include "Common.h"
#include "OdinUtils.h"


class DataModule
{
public:
	static void init();
	static void destroy();
	static std::map<std::string,std::vector<int>*>* getFuzzyValues();
	static std::vector<BWAPI::UnitType>* getCounter(std::string unit);

	static int loaded;
private:
	static std::map<std::string,std::vector<int>*>* units;
	static std::map<std::string,std::vector<BWAPI::UnitType>* >* counters;
	
};