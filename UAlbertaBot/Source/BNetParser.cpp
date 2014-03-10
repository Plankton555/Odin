#include "BNetParser.h"

BNetParser::BNetParser()
{
}


BNetParser::~BNetParser()
{
}


void BNetParser::start_document()
{
	BWAPI::Broodwar->printf("Parsing begins");
}

void BNetParser::end_document()
{
	BWAPI::Broodwar->printf("Parsing done");
	// put the network together
}

void BNetParser::start_element(const unsigned long line_number, const std::string& name, const dlib::attribute_list& atts)
{
	const char *cname = name.c_str();
	if (strcmp(cname, "cpt") == 0)
	{
		// start of new node
		// also, remember to count the nodes
		// node name will be in atts.reset(); atts.move_next(); atts.element().value();
	}
	else if (strcmp(cname, "state") == 0)
	{
		// Each state will be enumerated here (eg. Absent/Present or Periodx)
		// state name will be in atts.reset(); atts.move_next(); atts.element().value();
	}
	else if (strcmp(cname, "parents") == 0)
	{
		// Prepare to receive parents in "characters" function
	}
	else if (strcmp(cname, "probabilities") == 0)
	{
		// Prepare to receive probabilities in "characters" function
	}
	ofstream myfile("bnet_debug.txt", ios::app);
	//BWAPI::Broodwar->printf("on line %i we hit the <%s> tag", line_number, name);
	//cout << "on line " << line_number << " we hit the <" << name << "> tag" << endl;
	myfile << "on line " << line_number << " we hit the <" << name << "> tag" << endl;
	
	// print all the tag's attributes
	atts.reset();
	while (atts.move_next())
	{
		//BWAPI::Broodwar->printf("    attribute: %i = %i", atts.element().key(), atts.element().value());
		//cout << "\tattribute: " << atts.element().key() << " = " << atts.element().value() << endl;
		myfile << "\tattribute: " << atts.element().key() << " = " << atts.element().value() << endl;
	}
	myfile.close();
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
	}

	ofstream myfile("bnet_debug.txt", ios::app);
	//cout << "on line " << line_number << " we hit the closing tag </" << name << ">" << endl;
	myfile << "on line " << line_number << " we hit the closing tag </" << name << ">" << endl;
	myfile.close();
}

void BNetParser::characters(const std::string& data)
{
	// If we're waiting for parents, extract parents from data (separated by a ' ')
	// If we're waiting for probabilities, extract from data

	ofstream myfile("bnet_debug.txt", ios::app);
	//BWAPI::Broodwar->printf("Got some data between tags and it is: s");
	//cout << "Got some data between tags and it is:\n" << data << endl;
	myfile << "Got some data between tags and it is:\n" << data << endl;
	myfile.close();
}

void BNetParser::processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data)
{
	// Can ignore
	ofstream myfile("bnet_debug.txt", ios::app);
	//cout << "on line " << line_number << " we hit a processing instruction with a target of '" << target << "' and data '" << data << "'" << endl;
	myfile << "on line " << line_number << " we hit a processing instruction with a target of '" << target << "' and data '" << data << "'" << endl;
	myfile.close();
}