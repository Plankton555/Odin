#include "BayesianNet.h"

BayesianNet::BayesianNet()
{
	ofstream myfile("bnet_debug.txt", ios::app);
	myfile << "In constructor of BayesianNet" << endl;
	myfile.close();
}

BayesianNet::~BayesianNet()
{
	delete solution;
}

void BayesianNet::AddNode(ParsedNode* node)
{
	parsedNodes.push_back(node);
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

	ofstream myfile("bnet_debug.txt", ios::app);
	myfile << "Entering for loop, nrIterations: " << parsedNodes.size() << endl;
	int debug = 0;
	for (it = parsedNodes.begin(); it!=parsedNodes.end(); it++)
	{
		// Loop through a second time to add the edges and connect the network
		ParsedNode* current = (*it);
		debug++;
		myfile << "\nIteration: " << debug << ", node: " << current->name << endl;
		int currentID = nodeMap.find(current->name)->second;
		set_node_num_values(bn, currentID, current->states.size());
		for (unsigned int i=0; i<current->parents.size(); i++)
		{
			//Adding edges
			myfile << "Starting adding edges" << endl;
			std::string parent = current->parents.at(i);
			std::map<std::string, int>::iterator parIt = nodeMap.find(parent);
			if (parIt != nodeMap.end())
			{
				int parentID = parIt->second;
				myfile << "Trying to add edge from " << parent << " to " << current->name << endl;
				myfile << "parentID: " << parentID << ", currentID: " << currentID << endl;
				bn.add_edge(parentID, currentID); // Something goes wrong exactly here. From what I've managed to see in the debug chat ingame, both indexes are 0 "You can't add an edge if it already exists in the graph"
			}
			myfile << "Stopping adding edges" << endl;
		}
		// call recursive method for setting probabilities
		assignment parent_state;
		std::vector<double>::iterator probIterator = current->probabilities.begin();
		ApplyProbabilities(&(current->parents), 0, &parent_state, current, &probIterator);
	}

	create_moral_graph(bn, join_tree);
	create_join_tree(join_tree, join_tree);
	myfile.close();
}

/*
	Recursive method for setting probabilities.
*/
void BayesianNet::ApplyProbabilities(std::vector<std::string> *parents, unsigned int n, assignment *parent_state, ParsedNode *currentNode, std::vector<double>::iterator *it)
{
	ofstream myfile("bnet_debug.txt", ios::app);
	myfile << "\nIn ApplyProbabilities(), n: " << n << ", nrParents: " << parents->size() << endl;
	if (n >= parents->size()) // stop condition for recursion
	{
		myfile << "Condition fulfilled, read probability" << endl;

		int currNodeID = nodeMap.find(currentNode->name)->second;
		int nrOfStates = currentNode->states.size();
		for (int i=0; i<nrOfStates; i++)
		{
			double p = **it;
			myfile << "Setting prob: " << p << ", state: " << i << endl;
			bayes_node_utils::set_node_probability(bn, currNodeID, i, (*parent_state), p);
			myfile << "    Probability set" << endl;
			(*it)++;
		}
		myfile << "no more states here, returning" << endl;
		return;
	}

	myfile << "Condition not fulfilled, do recursion" << endl;
	int parentID = nodeMap.find(parents->at(n))->second;
	ParsedNode* parent = parsedNodes.at(parentID);
	int nrOfParentStates = parent->states.size();
	for (int i=0; i<nrOfParentStates; i++)
	{
		myfile << "\nParent state nr: " << i << endl;
		if (parent_state->has_index(parentID))
		{
			myfile << "Already have index, set new value" << endl;
			(*parent_state)[parentID] = i;
		}
		else
		{
			myfile << "Does not already have index, create new" << endl;
			parent_state->add(parentID, i);
		}
		ApplyProbabilities(parents, n+1, parent_state, currentNode, it); //recursive call
	}
	myfile.close();
}


void BayesianNet::SetEvidence(const std::string &nodeName, int nodeState)
{
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