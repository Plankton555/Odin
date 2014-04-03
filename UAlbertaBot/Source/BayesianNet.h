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
	typedef dlib::set<unsigned long>::compare_1b_c set_type;
	typedef graph<set_type, set_type>::kernel_1a_c join_tree_type;
	join_tree_type join_tree;
	bayesian_network_join_tree *solution;
	std::map<std::string, int> nodeMap;

	void ApplyProbabilities(std::vector<std::string> *parents, unsigned int n, assignment *parent_state, ParsedNode *currentNode, std::vector<double>::iterator *it);
public:
	BayesianNet();
	~BayesianNet();

	void AddNode(ParsedNode* node);
	void CreateNetwork();

	bool exists(const std::string &nodeName);
	void SetEvidence(const std::string &, int nodeState);
	void ClearEvidence();
	double ReadProbability(const std::string &nodeName, int nodeState);
	void UpdateBeliefs();
};