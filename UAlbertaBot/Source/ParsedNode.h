#pragma once
#include "Common.h"

using namespace std;

class ParsedNode
{
public:
	ParsedNode();
	~ParsedNode();
	
	std::string* name;
	std::vector<std::string*> states;
	std::vector<double> probabilities;
	std::vector<std::string*> parents;
};