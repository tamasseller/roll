#include "ast/ContractParser.h"
#include "ast/ContractFormatter.h"

#include "InputOptions.h"
#include "OutputOptions.h"
#include "CliApp.h"

#include <unistd.h>

struct DumpOptions: InputOptions, OutputOptions, FormatOptions {};

CLI_APP(dump, "Dump descriptor in textual format")
{
	DumpOptions opts;

	opts.InputOptions::add(this);
	opts.OutputOptions::add(this);
	opts.FormatOptions::add(this);

	if(this->processCommandLine())
	{
		if(opts.output != &std::cout)
		{
			opts.colored = false;
		}
		else if(opts.colored && !isatty(fileno(stdout)))
		{
		   std::cerr << "Warning: the output is not a terminal, disabling coloring (the -o option allows saving the output to a file directly)." << std::endl;
		   opts.colored = false;
		}

		*opts.output << format(opts, parse(*opts.input));
		return 0;
	}

	return -1;
}
