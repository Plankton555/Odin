#pragma once
#include "ParsedNode.h"
#include "Common.h"
#include <dlib/bayes_utils.h>
#include <dlib/graph_utils.h>
#include <dlib/graph.h>
#include <dlib/directed_graph.h>

using namespace dlib;

class BayesianNet
{
private:
	std::list<ParsedNode*> parsedNodes;
	directed_graph<bayes_node>::kernel_1a_c bn;
public:
	BayesianNet();
	~BayesianNet();

	void AddNode(ParsedNode* node);
	void CreateNetwork();
};