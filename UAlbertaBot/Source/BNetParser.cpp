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
}

void BNetParser::start_element(const unsigned long line_number, const std::string& name, const dlib::attribute_list& atts)
{
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
	ofstream myfile("bnet_debug.txt", ios::app);
	//cout << "on line " << line_number << " we hit the closing tag </" << name << ">" << endl;
	myfile << "on line " << line_number << " we hit the closing tag </" << name << ">" << endl;
	myfile.close();
}

void BNetParser::characters(const std::string& data)
{
	ofstream myfile("bnet_debug.txt", ios::app);
	//BWAPI::Broodwar->printf("Got some data between tags and it is: s");
	//cout << "Got some data between tags and it is:\n" << data << endl;
	myfile << "Got some data between tags and it is:\n" << data << endl;
	myfile.close();
}

void BNetParser::processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data)
{
	ofstream myfile("bnet_debug.txt", ios::app);
	//cout << "on line " << line_number << " we hit a processing instruction with a target of '" << target << "' and data '" << data << "'" << endl;
	myfile << "on line " << line_number << " we hit a processing instruction with a target of '" << target << "' and data '" << data << "'" << endl;
	myfile.close();
}