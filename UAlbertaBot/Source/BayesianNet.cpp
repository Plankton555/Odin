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
	std::vector<ParsedNode*>::iterator it;
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
		int currentID = nodeMap.find(current->name)->second;
		bayes_node_utils::set_node_num_values(bn, currentID, current->states.size());
		for (int i=0; i<current->parents.size(); i++)
		{
			//Adding edges
			std::string* parent = current->parents.at(i);
			std::map<std::string*, int>::iterator parIt = nodeMap.find(parent);
			if (parIt != nodeMap.end())
			{
				int parentID = parIt->second;
				bn.add_edge(parentID, currentID);
			}
		}
		for (int i=0; i<current->parents.size(); i++)
		{
			//Setting probabilities
			std::string* parentName = current->parents.at(i);
			std::map<std::string*, int>::iterator parIt = nodeMap.find(parentName);
			if (parIt != nodeMap.end())
			{
				int parentID = nodeMap.find(parentName)->second;
				ParsedNode* parent = parsedNodes.at(parentID);
				int nrOfParentStates = parent->states.size();
			}
		}
	}
}

void BayesianNet::ApplyProbabilities(std::vector<std::string*> *parents, int n, assignment *parent_state, ParsedNode *currentNode, std::map<std::string*, int> *nodeMap, directed_graph<bayes_node>::kernel_1a_c *bn, std::vector<double>::iterator *it)
{
	if (n >= parents->size()) // stop condition for recursion
	{
		int currNodeID = nodeMap->find(currentNode->name)->second;
		int nrOfStates = currentNode->states.size();
		for (int i=0; i<nrOfStates; i++)
		{
			double p = **it;
			bayes_node_utils::set_node_probability((*bn), currNodeID, i, (*parent_state), p);
		}
		return;
	}

	int parentID = nodeMap->find(parents->at(n))->second;
	ParsedNode* parent = parsedNodes.at(parentID);
	int nrOfParentStates = parent->states.size();
	for (int i=0; i<nrOfParentStates; i++)
	{
		if (parent_state->has_index(parentID))
		{
			(*parent_state)[parentID] = i;
		}
		else
		{
			parent_state->add(parentID, i);
		}
		ApplyProbabilities(parents, n+1, parent_state, currentNode, nodeMap, bn, it); //recursive call
	}
}

/*
applyProbabilities(parents, n, parent_state):
	if (n >= parents.size()): //stop condition for the recursion
		for each State state in currentNode
			p = readNextProbability();
			set_node_probability(bn, currentNode, state, parent_state, p);
		end for
		return
	end if
	currentParent = parents.at(n);
	for each State state in currentParent
		parent_state[currentParent] = state;
		applyProbabilities(parents, n+1, parent_state) //recursive call
	end for
*/