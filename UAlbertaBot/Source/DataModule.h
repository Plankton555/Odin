#pragma once

#include "Common.h"


class DataModule
{
public:
	static void init();
	static void destroy();
	static std::map<const char*,std::vector<int>*>* getFuzzyValues();

	static int loaded;
private:
	static std::vector<std::string>* splitDelim(const std::string& str, const std::string& delim);

	static std::map<const char*,std::vector<int>*> *units;

	
};