#include "Generator.h"

#include "Cpp.h"

#include <map>
#include <iostream>

GeneratorOptions::GeneratorOptions(): language(&CodeGenCpp::instance) {}

std::map<std::string, const CodeGen*> generators = {
	{"c++", &CodeGenCpp::instance}
};

void GeneratorOptions::select(const std::string &str)
{
	if(auto it = generators.find(str); it != generators.end())
	{
		this->language = it->second;
	}
	else
	{
		std::cerr << "Available languages: " << std::endl << std::endl;;

		for(auto p: generators)
		{
			std::cerr << "\t" << p.first << std::endl;;
		}

		std::cerr << std::endl;
		throw std::runtime_error("Unknown language requested");
	}
}
