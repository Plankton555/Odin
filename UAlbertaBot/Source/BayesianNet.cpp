#include "BayesianNet.h"

BayesianNet::BayesianNet()
{
}

BayesianNet::~BayesianNet()
{
}

void BayesianNet::AddNode(ParsedNode* node)
{
	parsedNodes.push_back(node);
}

void BayesianNet::CreateNetwork()
{
	directed_graph<bayes_node>::kernel_1a_c bn;
	int nrNodes = parsedNodes.size();

	bn.set_number_of_nodes(nrNodes);

	std::map<std::string*, int> nodeMap;
	
	int index = 0;
	std::list<ParsedNode*>::iterator it;
	for (it=parsedNodes.begin(); it!=parsedNodes.end(); it++)
	{
		ParsedNode* current = (*it);
		nodeMap.insert(std::map<std::string*, int>::value_type (current->name, index));
	}
}