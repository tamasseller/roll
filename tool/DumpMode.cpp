#include "DumpMode.h"

template<class C, class P>
static inline std::string csl(C&& c, P&& p)
{
	std::stringstream ss;

	bool first = true;

	for(const auto &e: c)
	{
		if(first)
		{
			first = false;
		}
		else
		{
			ss << ", ";
		}

		ss << p(e);
	}

	return ss.str();
}

static inline std::string indent(const int n) {
	return std::string(n * 4, ' ');
}

static inline std::string typeName(const int n, const Ast::Type& t);

static inline std::string memberItem(const int n, const Ast::Var &v){
	return v.name + ": " + typeName(n, v.type);
}

static inline std::string typeName(const int n, const Ast::Primitive& p) {
	return std::string(p.isSigned ? "i" : "u") + std::to_string(p.length);
}

static inline std::string formatNewlineIndentDelimit(const int n, const std::string& str, const char start, const char end)
{
	if(str.find('\n') != std::string::npos)
	{
		std::stringstream ss;
		ss << "\n" << indent(n) << start;
		ss << indent(n + 1) << str << "\n";
		ss << indent(n) << end;
		return ss.str();
	}
	else
	{
		return start + str + end;
	}
}

static inline std::string typeName(const int n, const Ast::Collection& c) {
	return formatNewlineIndentDelimit(n, typeName(n + 1, *c.elementType), '[', ']');
}

[[noreturn]] static inline std::string typeName(const int, const std::monostate&) { assert(false); }

static inline std::string varList(const int n, const std::vector<Ast::Var> &vs)
{
	if(vs.size())
	{
		const auto first = memberItem(n, vs.front());
		if(vs.size() == 1 && first.find('\n') == std::string::npos)
		{
			return first;
		}
		else
		{
			std::stringstream ss;

			for(const auto& v: vs)
			{
				ss << "\n" << indent(n) << memberItem(n, v);
			}

			return ss.str();
		}
	}

	return {};
}

static inline std::string typeName(const int n, const Ast::Aggreagete& c) {
	return formatNewlineIndentDelimit(n, varList(n + 1, c.members), '{', '}');
}


static inline std::string typeName(const int n, const Ast::Type& t) {
	return std::visit([n](const auto& x){ return typeName(n, x); }, (Ast::Type::variant&)t);
}

int DumpMode::run(Ast ast, std::vector<std::string> args) const
{
	for(const auto& s: ast.push)
	{
		std::cout << indent(0) << s.name << formatNewlineIndentDelimit(0, varList(1, s.args), '(', ')') << std::endl;
	}

	for(const auto& s: ast.pull)
	{
		std::cout << indent(0) << s.name << " -> " << typeName(1, s.returnType) << " " << formatNewlineIndentDelimit(1, varList(2, s.args), '(', ')') << std::endl;
	}

	return 0;
}
