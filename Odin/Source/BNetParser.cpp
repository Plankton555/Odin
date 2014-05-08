#include "BNetParser.h"

BNetParser::BNetParser()
{
}


BNetParser::~BNetParser()
{
}


void BNetParser::start_document()
{
	documentNr = 1; // start_document and end_document is apparently run twice... This makes sure it's only run once.
	bn = new BayesianNet();
}

void BNetParser::end_document()
{
	if (documentNr > 0)
	{
		documentNr = 0; // start_document and end_document is apparently run twice... This makes sure it's only run once.
		// put the network together
		bn->CreateNetwork();
	}
}

void BNetParser::start_element(const unsigned long line_number, const std::string& name, const dlib::attribute_list& atts)
{
	const char *cname = name.c_str(); // Should be able to just use string == "" according to some guy at StackOverflow
	if (strcmp(cname, "cpt") == 0)
	{
		// start of new node element
		currentNode = new ParsedNode();
		atts.reset();
		atts.move_next();
		std::string nodename = atts.element().value();
		currentNode->name = nodename;
	}
	else if (strcmp(cname, "state") == 0)
	{
		// start of new state element
		atts.reset();
		atts.move_next();
		std::string statename = atts.element().value();
		currentNode->states.push_back(statename);
	}
	else if (strcmp(cname, "parents") == 0)
	{
		// start of new parents element
		waitingForProbs = 0;
		// Prepare to receive parents in "characters" function
	}
	else if (strcmp(cname, "probabilities") == 0)
	{
		// start of new probabilities element
		waitingForProbs = 1;
		// Prepare to receive probabilities in "characters" function
	}
}

void BNetParser::end_element(const unsigned long line_number, const std::string& name)
{
	const char *cname = name.c_str();
	if (strcmp(cname, "probabilities") == 0)
	{
		// Received the probabilities for the current node
	}
	else if (strcmp(cname, "parents") == 0)
	{
		// Received the parents for the current node
	}
	else if (strcmp(cname, "cpt") == 0)
	{
		// finished with the node
		bn->AddNode(currentNode);
	}
}

void BNetParser::characters(const std::string& data)
{
	std::string delim = " ";
	std::vector<std::string> *dataVec = splitDelim(data, delim);

	if (waitingForProbs == 0)
	{
		currentNode->parents = *dataVec;
		waitingForProbs = -1; // don't wait for more parents
	}
	else if (waitingForProbs == 1)
	{
		// Convert dataVec to vector of doubles
		std::vector<double> *doubleVec = new std::vector<double>();
		for (unsigned int i=0; i<dataVec->size(); i++)
		{
			double x = atof(dataVec->at(i).c_str());
			doubleVec->push_back(x);
		}
		currentNode->probabilities = *doubleVec;
		delete doubleVec;
		// Possibly memory leak if not removing dataVec (since doubleVec is used instead)
		waitingForProbs = -1; // don't wait for more probabilities
	}
	delete dataVec;
}

void BNetParser::processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data)
{
	// Can ignore
}

std::vector<std::string>* BNetParser::splitDelim(const std::string& str, const std::string& delim)
{
	std::string s = str;
	std::vector<std::string> *output = new std::vector<std::string>;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delim)) != std::string::npos) {
		token = s.substr(0, pos);
		output->push_back(token);
		s.erase(0, pos + delim.length());
	}
	output->push_back(s);
	return output;
}

BayesianNet* BNetParser::getBayesianNet()
{
	return bn;
}