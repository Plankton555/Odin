#pragma once
#include "Common.h"

using namespace std;

class ParsedNode
{
public:
	ParsedNode();
	~ParsedNode();
	
	std::string* name;
	std::list<std::string*> states;
	std::string* probabilities;
	std::string* parents;
};