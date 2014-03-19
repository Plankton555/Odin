#include "BNetParser.h"

BNetParser::BNetParser()
{
}


BNetParser::~BNetParser()
{
}


void BNetParser::start_document()
{
	bn = new BayesianNet();
}

void BNetParser::end_document()
{
	// put the network together
	bn->CreateNetwork();
}

void BNetParser::start_element(const unsigned long line_number, const std::string& name, const dlib::attribute_list& atts)
{
	ofstream myfile("bnet_debug.txt", ios::app);
	const char *cname = name.c_str();
	if (strcmp(cname, "cpt") == 0)
	{
		currentNode = new ParsedNode();
		atts.reset();
		atts.move_next();
		std::string nodename = atts.element().value();
		currentNode->name = nodename;
		// start of new node
		// also, remember to count the nodes
		// node name will be in atts.reset(); atts.move_next(); atts.element().value();
		myfile << "on line " << line_number << " starting with node" << endl;
	}
	else if (strcmp(cname, "state") == 0)
	{
		atts.reset();
		atts.move_next();
		std::string statename = atts.element().value();
		currentNode->states.push_back(statename);
		// Each state will be enumerated here (eg. Absent/Present or Periodx)
		// state name will be in atts.reset(); atts.move_next(); atts.element().value();
		myfile << "on line " << line_number << " starting with state" << endl;
	}
	else if (strcmp(cname, "parents") == 0)
	{
		waitingForProbs = 0;
		// Prepare to receive parents in "characters" function
		myfile << "on line " << line_number << " waiting for parents" << endl;
	}
	else if (strcmp(cname, "probabilities") == 0)
	{
		waitingForProbs = 1;
		// Prepare to receive probabilities in "characters" function
		myfile << "on line " << line_number << " waiting for probabilities" << endl;
	}
	myfile.close();
}

void BNetParser::end_element(const unsigned long line_number, const std::string& name)
{
	ofstream myfile("bnet_debug.txt", ios::app);
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
		myfile << "on line " << line_number << " node is finished" << endl;
	}

	myfile.close();
}

void BNetParser::characters(const std::string& data)
{
	ofstream myfile("bnet_debug.txt", ios::app);
	std::string delim = " ";
	std::vector<std::string> *dataVec = splitDelim(data, delim);

	if (waitingForProbs == 0)
	{
		currentNode->parents = *dataVec;
		myfile << "Stored some parents in the node:\n" /*<< data*/ << endl;
		waitingForProbs = -1;
	}
	else if (waitingForProbs == 1)
	{
		// Convert dataVec to vector of doubles
		//myfile << "Starting storing probabilities:" << endl;
		std::vector<double> *doubleVec = new std::vector<double>();
		for (unsigned int i=0; i<dataVec->size(); i++)
		{
			//std::istringstream is(doubleVec->at(i)); //What is this?... Problem is here
			double x = atof(dataVec->at(i).c_str());
			//myfile << "Used atof(): " << x << endl;
			//if (!(is >> x))
			//	x = 0;
			doubleVec->push_back(x);
		}
		currentNode->probabilities = *doubleVec;
		myfile << "Stored some probabilities in the node:\n" /*<< data*/ << endl;
		// Possibly memory leak if not removing dataVec (since doubleVec is used instead)
		waitingForProbs = -1;
	}

	myfile.close();
}

void BNetParser::processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data)
{
	// Can ignore
}

std::vector<std::string>* BNetParser::splitDelim(const std::string& str, const std::string& delim)
{
	ofstream myfile("bnet_debug.txt", ios::app);
	std::string s = str;
	std::vector<std::string> *output = new std::vector<std::string>;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delim)) != std::string::npos) {
		token = s.substr(0, pos);
		//myfile << "str: " << str << endl;
		//myfile << "s.substr(0, pos): " << s.substr(0, pos) << endl;
		//myfile << "token: " << token << endl;
		output->push_back(token);
		s.erase(0, pos + delim.length());
	}
	output->push_back(s);
	myfile.close();
	return output;
}

BayesianNet* BNetParser::getBayesianNet()
{
	return bn;
}