#include "BayesianNet.h"

BayesianNet::BayesianNet()
{
	solution = NULL;
}

BayesianNet::~BayesianNet()
{
	delete solution;
}

void BayesianNet::AddNode(ParsedNode* node)
{
	parsedNodes.push_back(node);
}

void debug2(std::string str)
{
	ofstream file ("debug.txt", ios::app);
	if (file.is_open())
	{
		file << str.c_str() << endl;
		file.close();
	}
}

void debug2(std::string str, int i)
{
	std::ostringstream stringStream;
	stringStream << str;
	stringStream << ": ";
	stringStream << i;
	std::string newStr = stringStream.str();
	debug2(newStr);
}

void BayesianNet::CreateNetwork()
{
	using namespace bayes_node_utils;
	// Create the network
	int nrNodes = parsedNodes.size();
	bn.set_number_of_nodes(nrNodes);

	// Store all nodes so that the number can be looked up from the name
	int index = 0;
	std::vector<ParsedNode*>::iterator it;
	for (it=parsedNodes.begin(); it!=parsedNodes.end(); it++)
	{
		// Loop through nodes and connect name to id nr
		ParsedNode* current = (*it);
		nodeMap.insert(std::map<std::string, int>::value_type (current->name, index));
		index++;
	}

	for (it = parsedNodes.begin(); it!=parsedNodes.end(); it++)
	{
		// Loop through a second time to add the edges and connect the network
		ParsedNode* current = (*it);
		int currentID = nodeMap.find(current->name)->second;
		set_node_num_values(bn, currentID, current->states.size());
		for (unsigned int i=0; i<current->parents.size(); i++)
		{
			//Adding edges
			std::string parent = current->parents.at(i);
			std::map<std::string, int>::iterator parIt = nodeMap.find(parent);
			if (parIt != nodeMap.end())
			{
				int parentID = parIt->second;
				bn.add_edge(parentID, currentID);
			}
		}
		// call recursive method for setting probabilities
		assignment parent_state;
		std::vector<double>::iterator probIterator = current->probabilities.begin();
		ApplyProbabilities(&(current->parents), 0, &parent_state, current, &probIterator);
	}

	create_moral_graph(bn, join_tree);
	create_join_tree(join_tree, join_tree);
}

/*
	Recursive method for setting probabilities.
*/
void BayesianNet::ApplyProbabilities(std::vector<std::string> *parents, unsigned int n, assignment *parent_state, ParsedNode *currentNode, std::vector<double>::iterator *it)
{
	if (n >= parents->size()) // stop condition for recursion
	{
		int currNodeID = nodeMap.find(currentNode->name)->second;
		int nrOfStates = currentNode->states.size();
		for (int i=0; i<nrOfStates; i++)
		{
			double p = **it;
			bayes_node_utils::set_node_probability(bn, currNodeID, i, (*parent_state), p);
			(*it)++;
		}
		return;
	}

	int parentID = nodeMap.find(parents->at(n))->second;
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
		ApplyProbabilities(parents, n+1, parent_state, currentNode, it); //recursive call
	}
}


void BayesianNet::SetEvidence(const std::string &nodeName, int nodeState)
{
	debug2("GETTING SIZE IN SET EVIDENCE");
	debug2("VALUE?",nodeMap.size());
	debug2("DONE GETTING SIZE IN SET EVID");
	int nodeID = nodeMap.find(nodeName)->second;
	bayes_node_utils::set_node_value(bn, nodeID, nodeState);
	bayes_node_utils::set_node_as_evidence(bn, nodeID);
}


void BayesianNet::ClearEvidence()
{
	std::map<std::string, int>::iterator it;
	for (it = nodeMap.begin(); it != nodeMap.end(); it++)
	{
		int currNodeID = (*it).second;
		bayes_node_utils::set_node_as_nonevidence(bn, currNodeID);
	}
}


double BayesianNet::ReadProbability(const std::string &nodeName, int nodeState)
{
	int nodeID = nodeMap.find(nodeName)->second;
	return solution->probability(nodeID)(nodeState);
}


void BayesianNet::UpdateBeliefs()
{
	delete solution;
	solution = new bayesian_network_join_tree(bn, join_tree);
}

std::map<std::string, std::vector<double> > BayesianNet::getAllProbabilities()
{
	std::map<std::string, std::vector<double > > BNData;
	std::map<std::string, int>::iterator it;
	debug2("START GETTING ALL");
	
	debug2("BEFORE ASKING NODEMAP");
	debug2("Nodemap size", nodeMap.size());
	debug2("AFTER ASKING NODEMAP");
	/*
	//int i = nodeMap.size();
	//std::map<std::string, int>::size_type size;
	int size;
	size = nodeMap.size();
	debug2("GOT VALUE");
	std::ostringstream stringStream;
	stringStream << "Value here: ";
	stringStream << size;
	std::string newStr = stringStream.str();
	debug2("SAVING IT NOW!");
	debug2(newStr);
	//nodeMap.size();
	debug2("AFTER ASKING NODEMAP");
	*/
	for (it = nodeMap.begin(); it != nodeMap.end(); it++)
	{
		debug2("ADDING ONE NODE");
		int nodeID = it->second;
		int nrStates = bayes_node_utils::node_num_values(bn, nodeID);
		std::vector<double> probabilities(nrStates);
		for (int state = 0; state < nrStates; state++)
		{
			debug2("ADDING ONE STATE");
			probabilities.at(state) = ReadProbability(it->first, state);
		}

		BNData[it->first] = probabilities;
	}
	debug2("RETURNING ALL");
	return BNData;
}