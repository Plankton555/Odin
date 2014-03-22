#include "ParsedNode.h"

ParsedNode::ParsedNode()
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