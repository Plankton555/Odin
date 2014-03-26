#pragma once
#include "Common.h"
#include "BayesianNet.h"

class CBR
{
public:
	static void getDecision(std::map<std::string, std::vector<double> > nodes);
};