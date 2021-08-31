#ifndef RPC_TOOL_AST_ASTFORMATTER_H_
#define RPC_TOOL_AST_ASTFORMATTER_H_

#include "Ast.h"

struct FormatOptions
{
	bool colored = true;
	bool pretty = true;
	int indentStep = 4;

	template<class Host>
	void add(Host* h)
	{
		h->addOptions({"-n", "--no-colors"}, "Do not emit terminal color codes for output [default: enabled standard output, disabled for file]", [this]()
		{
			this->colored = false;
		});

		h->addOptions({"-u", "--ugly", "--no-wrap"}, "Do not break argument/members lists in multiple lines [default: do it]", [this]()
		{
			this->pretty = false;
		});

		h->addOption("--indent", "Set number of space characters used for indentation [default: 4]", [this](int n)
		{
			if(0 <= n && n < 10)
			{
				this->indentStep = n;
			}
			else
			{
				throw std::runtime_error("Invalid number of spaces for indentation");
			}
		});
	}

public:
	enum class Highlight
	{
		TypeRef, TypeDef, Argument, Member, Function, Primitive, Session
	};

	inline std::string colorize(const std::string& str, Highlight kind) const;
	inline std::string indent(const int n) const;
	inline std::string formatNewlineIndentDelimit(const int n, const std::string& str, const char start, const char end) const;
};

std::string format(const FormatOptions& opts, const Ast& ast);

#endif /* RPC_TOOL_AST_ASTFORMATTER_H_ */
