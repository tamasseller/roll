#ifndef RPC_TOOL_GEN_GENERATOR_H_
#define RPC_TOOL_GEN_GENERATOR_H_

#include "ast/Ast.h"

struct CodeGen
{
	inline virtual ~CodeGen() = default;
	virtual std::string generateClient(const Ast&) const = 0;
	virtual std::string generateServer(const Ast&) const = 0;
};

struct GeneratorOptions
{
	const CodeGen* language;
	std::string (CodeGen::* direction)(const Ast&) const = &CodeGen::generateClient;

	void select(const std::string &str);

public:
	GeneratorOptions();

	template<class Host>
	void add(Host* h)
	{
		h->addOptions({"-l", "--lang", "--source-language"}, "Set source language to generate code for [default: c++]", [this](const std::string &str)
		{

		});

		h->addOptions({"-c", "--client"}, "Generate RPC adapters for client side [default]", [this]()
		{
			this->direction = &CodeGen::generateClient;
		});

		h->addOptions({"-s", "--server"}, "Generate RPC adapters for server side [default]", [this]()
		{
			this->direction = &CodeGen::generateServer;
		});
	}

	inline std::string invokeGenerator(const Ast& ast) {
		return (language->*direction)(ast);
	}
};

#endif /* RPC_TOOL_GEN_GENERATOR_H_ */
