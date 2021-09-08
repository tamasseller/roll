#include "ast/ContractParser.h"
#include "ast/ContractRansSerDesCodec.h"

#include "InputOptions.h"
#include "OutputOptions.h"
#include "CliApp.h"

#include <sstream>

struct SerializeOptions: InputOptions, OutputOptions {};

CLI_APP(serialize, "Convert descriptor to dense binary format")
{
	SerializeOptions opts;

	opts.InputOptions::add(this);
	opts.OutputOptions::add(this);

	if(this->processCommandLine())
	{
		auto ast = parse(*opts.input);
		auto data = serialize(ast);

		std::istringstream is(data, std::ios::binary | std::ios::in);
		auto rec = deserialize(is);

		auto it = rec.begin();
		for(const auto& c: ast)
		{
			if(c.items.size())
			{
				assert(it != rec.end() && c == *it++);
			}
		}

		assert(it == rec.end());

		*opts.output << data;
		return 0;
	}

	return -1;
}
