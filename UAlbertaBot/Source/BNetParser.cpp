#include "BNetParser.h"

BNetParser::BNetParser(void)
{
}


BNetParser::~BNetParser(void)
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
	cout << "on line " << line_number << " we hit the <" << name << "> tag" << endl;
	
	// print all the tag's attributes
	atts.reset();
	while (atts.move_next())
	{
		cout << "\tattribute: " << atts.element().key() << " = " << atts.element().value() << endl;
	}
}

void BNetParser::end_element(const unsigned long line_number, const std::string& name)
{
	cout << "on line " << line_number << " we hit the closing tag </" << name << ">" << endl;
}

void BNetParser::characters(const std::string& data)
{
	cout << "Got some data between tags and it is:\n" << data << endl;
}

void BNetParser::processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data)
{
	cout << "on line " << line_number << " we hit a processing instruction with a target of '" << target << "' and data '" << data << "'" << endl;
}