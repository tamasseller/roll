#ifndef RPC_TOOL_GEN_GENERATOR_H_
#define RPC_TOOL_GEN_GENERATOR_H_

#include "ast/Contract.h"

#include <sstream>

struct CodeGen
{
	inline virtual ~CodeGen() = default;
	virtual std::string generate(const std::vector<Contract>& ast, bool generateClientProxy, bool generateServiceProxy) const = 0;
};

struct GeneratorOptions
{
	const CodeGen* language;
	bool doClient = true, doService = true;

	void select(const std::string &str);

public:
	GeneratorOptions();

	template<class Host>
	void add(Host* h)
	{
		h->addOptions({"-l", "--lang", "--source-language"}, "Set source language to generate code for [default: c++]", [this](const std::string &str)
		{
			select(str);
		});

		h->addOptions({"-nc", "--no-client"}, "Do not generate RPC adapters for client side [default: do it]", [this]()
		{
			this->doClient = false;
		});

		h->addOptions({"-ns", "--no-service"}, "Do not generate RPC adapters for server side [default: do it]", [this]()
		{
			this->doService = false;
		});
	}

	inline std::string invokeGenerator(const std::vector<Contract>& ast) {
		return language->generate(ast, doClient, doService);
	}
};

#endif /* RPC_TOOL_GEN_GENERATOR_H_ */
