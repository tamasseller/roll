#ifndef RPC_TOOL_GEN_GENERATOR_H_
#define RPC_TOOL_GEN_GENERATOR_H_

#include "ast/Contract.h"

#include <sstream>

struct CodeGen
{
	inline virtual ~CodeGen() = default;
	virtual std::string generate(const std::vector<Contract>& ast, const std::string& name, bool generateClientProxy, bool generateServiceProxy) const = 0;
};

struct GeneratorOptions
{
	const CodeGen* language;
	bool doClient = false, doService = false;
	std::optional<std::string> name;

	void select(const std::string &str);

public:
	GeneratorOptions();

	template<class Host>
	void add(Host* h)
	{
		h->addOptions({"-l", "--lang", "--source-language"}, "Set source language to generate code for [default: c++]", [this](const std::string &str)
		{
			this->select(str);
		});

		h->addOptions({"-n", "--name", "--module-name"}, "Set the module name to generate code for [default: based on output filename or first contract name]", [this](const std::string &str)
		{
			this->name = str;
		});

		h->addOptions({"-c", "--client"}, "Generate RPC proxy for client side [default: don't]", [this]()
		{
			this->doClient = true;
		});

		h->addOptions({"-s", "--service"}, "Generate RPC proxy for server side [default: don't]", [this]()
		{
			this->doService = true;
		});
	}

	std::string invokeGenerator(const std::vector<Contract>& ast, std::optional<std::string> name);
};

#endif /* RPC_TOOL_GEN_GENERATOR_H_ */
