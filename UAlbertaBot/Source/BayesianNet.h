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
	std::vector<ParsedNode*> parsedNodes;
	directed_graph<bayes_node>::kernel_1a_c bn;
	std::map<std::string*, int> nodeMap;

	void ApplyProbabilities(std::vector<std::string*> *parents, unsigned int n, assignment *parent_state, ParsedNode *currentNode, std::vector<double>::iterator *it);
public:
	BayesianNet();
	~BayesianNet();

	void AddNode(ParsedNode* node);
	void CreateNetwork();
};