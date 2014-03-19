#include "ParsedNode.h"

ParsedNode::ParsedNode()// : name(NULL), probabilities(NULL), parents(NULL)
{
	name = "";
	states = std::vector<std::string>();
	probabilities = std::vector<double>();
	parents = std::vector<std::string>();
}


ParsedNode::~ParsedNode()
{
}
// Change ParsedNode to a struct