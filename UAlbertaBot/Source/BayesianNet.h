#pragma once
#include "ParsedNode.h"

class BayesianNet
{
public:
	BayesianNet();
	~BayesianNet();

	void AddNode(ParsedNode* node);
};