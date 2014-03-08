#pragma once

#include <dlib/xml_parser.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace dlib;

class BNetParser : public document_handler
{
public:
	BNetParser(void);
	~BNetParser(void);

	virtual void start_document();
	virtual void end_document();
	virtual void start_element(const unsigned long line_number, const std::string& name, const dlib::attribute_list& atts);
	virtual void end_element(const unsigned long line_number, const std::string& name);
	virtual void characters(const std::string& data);
	virtual void processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data);

};

