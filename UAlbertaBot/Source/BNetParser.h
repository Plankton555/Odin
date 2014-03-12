#pragma once

#include <dlib/xml_parser.h>
#include <iostream>
#include <fstream>
#include <BWAPI.h>
#include "BayesianNet.h"
#include "ParsedNode.h"

using namespace std;
using namespace dlib;

class BNetParser : public document_handler
{
private:
	BayesianNet* bn;
	ParsedNode* currentNode;
	int waitingForProbs;

	std::vector<std::string*>* splitDelim(const std::string& str, const std::string& delim); 
public:
	BNetParser();
	~BNetParser();

	virtual void start_document();
	virtual void end_document();
	virtual void start_element(const unsigned long line_number, const std::string& name, const dlib::attribute_list& atts);
	virtual void end_element(const unsigned long line_number, const std::string& name);
	virtual void characters(const std::string& data);
	virtual void processing_instruction(const unsigned long line_number, const std::string& target, const std::string& data);

};

