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
	// Create the network
	directed_graph<bayes_node>::kernel_1a_c bn;
	int nrNodes = parsedNodes.size();

	bn.set_number_of_nodes(nrNodes);

	// Store all nodes so that the number can be looked up from the name
	std::map<std::string*, int> nodeMap;
	
	int index = 0;
	std::list<ParsedNode*>::iterator it;
	for (it=parsedNodes.begin(); it!=parsedNodes.end(); it++)
	{
		// Loop through nodes and connect name to id nr
		ParsedNode* current = (*it);
		nodeMap.insert(std::map<std::string*, int>::value_type (current->name, index));
	}

	for (it = parsedNodes.begin(); it!=parsedNodes.end(); it++)
	{
		// Loop through a second time to add the edges and connect the network
		//bn.add_edge(A, D);
		//set_node_num_values(bn, A, 2);
		// Loop 
		//assignment parent_state;
		//parent_state.clear();
		//parent_state.add(B, 1); (parent and its value)
		//parent_state[B] = 0; (to change)
		// Here we specify that p(A=1 | B=1, C=1) = 0.99 
		//set_node_probability(bn, A, 1, parent_state, 0.99);
		//set_node_probability(bn, A, 0, parent_state, 1-0.99);
		ParsedNode* current = (*it);
	}
}