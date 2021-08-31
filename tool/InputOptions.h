#ifndef RPC_TOOL_INPUTOPTIONS_H_
#define RPC_TOOL_INPUTOPTIONS_H_

#include <iostream>
#include <fstream>

class InputOptions
{
	std::ifstream inputFile;

public:
	std::istream *input = &std::cin;

	template<class Host>
	void add(Host* h)
	{
		h->addOptions({"-i", "--input"}, "Set input file", [this](const std::string &str)
		{
			if(!(this->inputFile = std::ifstream(str)))
			{
				throw std::runtime_error("Input file '" + str + "' could not be opened [default: standard input]");
			}
			else
			{
				this->input = &inputFile;
			}
		});
	}
};

#endif /* RPC_TOOL_INPUTOPTIONS_H_ */
