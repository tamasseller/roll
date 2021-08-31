#ifndef RPC_TOOL_OUTPUTOPTIONS_H_
#define RPC_TOOL_OUTPUTOPTIONS_H_

#include <iostream>
#include <fstream>

class OutputOptions
{
	std::ofstream outputFile;

public:
	std::ostream *output = &std::cout;

	template<class Host>
	void add(Host* h)
	{
		h->addOptions({"-o", "--output"}, "Set output file [default: standard output]", [this](const std::string &str)
		{
			if(!(this->outputFile = std::ofstream(str, std::ios::binary)))
			{
				throw std::runtime_error("Output file '" + str + "' could not be opened");
			}
			else
			{
				this->output = &outputFile;
			}
		});
	}
};

#endif /* RPC_TOOL_OUTPUTOPTIONS_H_ */
