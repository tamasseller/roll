#include "DumpMode.h"

#include <iterator>
#include <iostream>
#include <functional>

enum class Highlight
{
	TypeRef, TypeDef, Argument, Member, Function, Primitive, Session
};

struct Options
{
	bool colored = true;
	bool pretty = true;
	int indentStep = 4;

private:
	static constexpr const char* reset = "\x1b[0m";
	static constexpr const char* red = "\x1b[31;1m";
	static constexpr const char* green = "\x1b[32;1m";
	static constexpr const char* yellow = "\x1b[33;1m";
	static constexpr const char* blue = "\x1b[34;1m";
	static constexpr const char* magenta = "\x1b[35;1m";
	static constexpr const char* cyan = "\x1b[36;1m";
	static constexpr const char* white = "\x1b[37;1m";

	static inline std::string getColorFor(Highlight kind)
	{
		switch(kind)
		{
		case Highlight::Function:
			return magenta;
		case Highlight::TypeDef:
			return yellow;
		case Highlight::TypeRef:
			return white;
		case Highlight::Argument:
			return green;
		case Highlight::Member:
			return cyan;
		case Highlight::Primitive:
			return blue;
		case Highlight::Session:
			return red;
		default:
			return reset;
		}
	}

public:
	inline std::string colorize(const std::string& str, Highlight kind) const
	{
		if(colored)
		{
			return getColorFor(kind) + str + reset;
		}

		return str;
	}

	inline std::string indent(const int n) const {
		return std::string(n * indentStep, ' ');
	}

	inline std::string formatNewlineIndentDelimit(const int n, const std::string& str, const char start, const char end) const
	{
		if(pretty && str.find('\n') != std::string::npos)
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
};

static inline std::string typeName(const Options& opts, const int n, const Ast::Type& t, bool = true);

static inline std::string memberItem(const Options& opts, const int n, const Ast::Var &v, Highlight h) {
	return opts.colorize(v.name, h) + ": " + typeName(opts, n, v.type);
}

template<class C, class F>
static inline std::string list(const Options& opts, const int n, const C &vs, F&& f)
{
	if(vs.size())
	{
		if(const auto first = f(opts, n, vs.front()); vs.size() == 1 && first.find('\n') == std::string::npos)
		{
			return first;
		}

		std::stringstream ss;

		bool isFirst = true;
		for(const auto& v: vs)
		{
			if(opts.pretty)
			{
				ss << (isFirst ? "\n" : ",\n") << opts.indent(n) << f(opts, n, v);
			}
			else
			{
				ss << (isFirst ? "" : ", ") << f(opts, n, v);
			}

			isFirst = false;
		}

		return ss.str();
	}

	return {};
}

static inline std::string typeKindToString(const Options& opts, const int n, const Ast::Primitive& p) {
	return opts.colorize(std::string(p.isSigned ? "i" : "u") + std::to_string(p.length), Highlight::Primitive);
}

static inline std::string typeKindToString(const Options& opts, const int n, const Ast::Collection& c) {
	return opts.formatNewlineIndentDelimit(n, typeName(opts, n + 1, *c.elementType), '[', ']');
}

static inline std::string typeKindToString(const Options& opts, const int n, const Ast::Aggreagete& c) {
	return opts.formatNewlineIndentDelimit(n, list(opts, n + 1, c.members, [](const Options& opts, const int n, const Ast::Var& v){
		return memberItem(opts, n, v, Highlight::Member);}
	), '{', '}');
}

static inline std::string typeName(const Options& opts, const int n, const Ast::Type& t, bool useNameIfNotEmpty)
{
	const auto trueType = std::visit([n, opts](const auto& x){ return typeKindToString(opts, n, x); }, t.second);

	if(useNameIfNotEmpty && t.first.length())
	{
		return opts.colorize(t.first, Highlight::TypeRef) + (opts.pretty ? (std::string(" /* ") + trueType + " */") : std::string{});
	}

	return trueType;
}

static inline std::string argumentList(const Options& opts, const int n, const std::vector<Ast::Var> &args)
{
	return opts.formatNewlineIndentDelimit(n, list(opts, n + 1, args, [](const Options& opts, const int n, const Ast::Var& v){
		return memberItem(opts, n, v, Highlight::Argument);}
	), '(', ')');
}

static inline std::string signature(const Options& opts, const int n, const Ast::Call& s) {
	return opts.colorize(s.name, Highlight::Function) + argumentList(opts, n, s.args);
}

static inline std::string formatItem(const Options& opts, const int n, const Ast::Call& s) {
	return opts.indent(n) + signature(opts, n, s) + ";";
}

static inline std::string formatItem(const Options& opts, const int n, const Ast::Pull& s) {
	return opts.indent(n) + signature(opts, n, s) + ": " + typeName(opts, n + 1, s.returnType) + ";";
}

static inline std::string formatItem(const Options& opts, const int n, const Ast::Alias& s) {
	return opts.indent(n) + opts.colorize(s.first, Highlight::TypeDef) + " = " + typeName(opts, n + 1, s, false) + ";";
}

static inline std::string formatSessionItem(const Options& opts, const int n, const Ast::Session::ForwardCall& s) {
	return "!" + signature(opts, n, s);
}

static inline std::string formatSessionItem(const Options& opts, const int n, const Ast::Session::CallBack& s) {
	return "@" + signature(opts, n, s);
}

static inline std::string formatItem(const Options& opts, const int n, const Ast::Session& s)
{
	return opts.indent(n) + opts.colorize(s.name, Highlight::Session) +
		opts.formatNewlineIndentDelimit(n, list(opts, n + 1, s.items, [](const Options& opts, const int n, const Ast::Session::Item& i){
			return std::visit([opts, n](auto& v) {return formatSessionItem(opts, n, v);}, i);
		}), '<', '>') + ";";
}

int DumpMode::run(Ast ast, std::vector<std::string> args) const
{
	Options opts;

	if(std::find(args.begin(), args.end(), "--no-colors") != args.end())
		opts.colored = false;

	std::transform(ast.items.begin(), ast.items.end(), std::ostream_iterator<std::string>(std::cout, "\n"), [opts](const auto& s){
		return std::visit([opts](const auto &s){return formatItem(opts, 0, s); }, s);
	});

	return 0;
}
