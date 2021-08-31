#include "ast/AstParser.h"
#include "ast/AstRansSerDesCodec.h"

#include "InputOptions.h"
#include "OutputOptions.h"
#include "CliApp.h"

#include <sstream>

struct Options: InputOptions, OutputOptions {};

CLI_APP(serialize, "parse and convert descriptor to dense binary format")
{
	Options opts;

	opts.InputOptions::add(this);
	opts.OutputOptions::add(this);

	if(this->processCommandLine())
	{
		auto ast = parse(*opts.input);
		auto data = serialize(ast);

		std::istringstream is(data, std::ios::binary | std::ios::in);
		auto rec = deserialize(is);
		assert(rec == ast);

		*opts.output << data;
		return 0;
	}

	return -1;
}
