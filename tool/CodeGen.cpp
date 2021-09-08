#include "ast/ContractParser.h"
#include "gen/Generator.h"

#include "InputOptions.h"
#include "OutputOptions.h"
#include "CliApp.h"

struct CodeGenOptions: InputOptions, OutputOptions, GeneratorOptions {};

CLI_APP(codegen, "Generate source code from contract descriptor")
{
	CodeGenOptions opts;

	opts.InputOptions::add(this);
	opts.OutputOptions::add(this);
	opts.GeneratorOptions::add(this);

	if(this->processCommandLine())
	{
		const auto ast = parse(*opts.input);
		const auto src = opts.invokeGenerator(ast);
		*opts.output << src;
		return 0;
	}

	return -1;
}
