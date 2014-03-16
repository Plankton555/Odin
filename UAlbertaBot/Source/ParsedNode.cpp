#include "ParsedNode.h"

ParsedNode::ParsedNode()// : name(NULL), probabilities(NULL), parents(NULL)
{
	name = NULL;
	states = std::vector<std::string*>();
	probabilities = std::vector<std::string*>();
	parents = std::vector<std::string*>();
}


ParsedNode::~ParsedNode()
{
}
// Change ParsedNode to a struct