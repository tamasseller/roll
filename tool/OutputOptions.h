#ifndef RPC_TOOL_OUTPUTOPTIONS_H_
#define RPC_TOOL_OUTPUTOPTIONS_H_

#include <iostream>
#include <fstream>
#include <optional>

#include <libgen.h>

class OutputOptions
{
	std::ofstream outputFile;

public:
	std::ostream *output = &std::cout;
	std::optional<std::string> name;

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
				this->name = basename(const_cast<char*>(str.c_str()));
			}
		});
	}
};

#endif /* RPC_TOOL_OUTPUTOPTIONS_H_ */
