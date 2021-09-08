#include "ContractFormatter.h"

#include <sstream>
#include <iterator>
#include <algorithm>
#include <numeric>

#include <cassert>

static constexpr const char* resetColor = "\x1b[0m";

static inline std::string getColorFor(FormatOptions::Highlight kind)
{
	static constexpr const char* red = "\x1b[31;1m";
	static constexpr const char* green = "\x1b[32;1m";
	static constexpr const char* yellow = "\x1b[33;1m";
	static constexpr const char* blue = "\x1b[34;1m";
	static constexpr const char* magenta = "\x1b[35;1m";
	static constexpr const char* cyan = "\x1b[36;1m";
	static constexpr const char* white = "\x1b[37;1m";

	switch(kind)
	{
	case FormatOptions::Highlight::Function:
		return red;
	case FormatOptions::Highlight::TypeDef:
		return yellow;
	case FormatOptions::Highlight::TypeRef:
		return white;
	case FormatOptions::Highlight::Argument:
		return green;
	case FormatOptions::Highlight::Member:
		return cyan;
	case FormatOptions::Highlight::Primitive:
		return blue;
	case FormatOptions::Highlight::Session:
		return magenta;
	default:
		return resetColor;
	}
}

inline std::string FormatOptions::colorize(const std::string& str, Highlight kind) const
{
	if(colored)
	{
		return getColorFor(kind) + str + resetColor;
	}

	return str;
}

inline std::string FormatOptions::indent(const int n) const {
	return std::string(n * indentStep, ' ');
}

inline std::string FormatOptions::formatNewlineIndentDelimit(const int n, const std::string& str, const char start, const char end) const
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

static inline std::string typeRef(const FormatOptions& opts, const int n, const Contract::TypeRef& t);

static inline std::string formatComent(const FormatOptions& opts, const int n, const std::string &text, bool firstItemInList)
{
	std::stringstream ss;

	if(text.length())
	{
		std::stringstream in(text);

		std::string line;
		bool gotLine = (bool)std::getline(in, line, '\n');
		assert(gotLine);

		bool first = true;
		bool multiLine = false;
		while(true)
		{
			if(first)
			{
				if(opts.pretty && !firstItemInList)
				{
					ss << "\n" << opts.indent(n);
				}

				ss << "/* ";
				first = false;
			}
			else if(line.length())
			{
				if(line[0] == '*')
				{
					ss << " ";
				}
				else
				{
					ss << "   ";
				}
			}

			ss << line;

			std::string next(1, '\0');
			if(std::getline(in, next, '\n') || !(next.length() == 1 && next[0] == '\0'))
			{
				ss << std::endl << opts.indent(n);
				line = std::move(next);
			}
			else
			{
				break;
			}

			multiLine = true;
		}

		ss << " */";

		if(opts.pretty || multiLine)
		{
			ss << "\n" << opts.indent(n);
		}
		else
		{
			ss << " ";
		}
	}

	return ss.str();
}

static inline std::string memberItem(const FormatOptions& opts, const int n, const Contract::Var &v, FormatOptions::Highlight h, bool first) {
	return formatComent(opts, n, v.docs, first) + opts.colorize(v.name, h) + ": "  + typeRef(opts, n, v.type);
}

template<class C, class F>
static inline std::string list(const FormatOptions& opts, const int n, const C &r, F&& f)
{
	std::vector<std::string> vs;
	for(const auto& v: r)
	{
		vs.push_back(f(opts, n, v));
	}

	if(vs.size())
	{
		if(vs.size() == 1 && vs.front().find('\n') == std::string::npos)
		{
			return vs.front();
		}

		std::stringstream ss;

		bool isFirst = true;
		for(const auto& v: vs)
		{
			ss << (isFirst ? "" : ", ") << ((opts.pretty) ? ("\n" + opts.indent(n)) : std::string{}) << v;
			isFirst = false;
		}

		return ss.str();
	}

	return {};
}

static inline std::string typeRefKindToString(const FormatOptions& opts, const int n, const Contract::Primitive& p) {
	return opts.colorize(std::string(p.isSigned ? "i" : "u") + std::to_string(p.length), FormatOptions::Highlight::Primitive);
}

static inline std::string typeRefKindToString(const FormatOptions& opts, const int n, const Contract::Collection& c) {
	return opts.formatNewlineIndentDelimit(n, typeRef(opts, n + 1, *c.elementType), '[', ']');
}

static inline std::string typeRefKindToString(const FormatOptions& opts, const int n, const std::string& p) {
	return opts.colorize(p, FormatOptions::Highlight::TypeRef);
}

static inline std::string typeRef(const FormatOptions& opts, const int n, const Contract::TypeRef& t) {
	return std::visit([n, &opts](const auto& x){ return typeRefKindToString(opts, n, x); }, t);
}

template<class C>
static inline std::string typeDefKindToString(const FormatOptions& opts, const int n, const C& v) {
	return typeRefKindToString(opts, n, v);
}

static inline std::string typeDefKindToString(const FormatOptions& opts, const int n, const Contract::Aggregate& c)
{
	return opts.formatNewlineIndentDelimit(n, list(opts, n + 1, c.members, [first{true}](const FormatOptions& opts, const int n, const Contract::Var& v) mutable
	{
		auto ret = memberItem(opts, n, v, FormatOptions::Highlight::Member, first);
		first = false;
		return ret;
	}), '{', '}');
}

static inline std::string typeDef(const FormatOptions& opts, const int n, const Contract::TypeDef& t) {
	return std::visit([n, &opts](const auto& x){ return typeDefKindToString(opts, n, x); }, t);
}

static inline std::string argumentList(const FormatOptions& opts, const int n, const std::vector<Contract::Var> &args)
{
	return opts.formatNewlineIndentDelimit(n, list(opts, n + 1, args, [first{true}](const FormatOptions& opts, const int n, const Contract::Var& v) mutable {
		auto ret = memberItem(opts, n, v, FormatOptions::Highlight::Argument, first);
		first = false;
		return ret;
	}), '(', ')');
}

static inline std::string signature(const FormatOptions& opts, const int n, const Contract::Action& s) {
	return opts.colorize(s.name, FormatOptions::Highlight::Function) + argumentList(opts, n, s.args);
}

static inline std::string formatItem(const FormatOptions& opts, const int n, const Contract::Action& s) {
	return opts.indent(n) + signature(opts, n, s) + ";";
}

static inline std::string formatItem(const FormatOptions& opts, const int n, const Contract::Function& s)
{
	return opts.indent(n)
		+ signature(opts, n, s)
		+ ((s.returnType.has_value()) ? (std::string(": ") + typeRef(opts, n + 1, s.returnType.value())) : std::string{}) + ";";
}

static inline std::string formatItem(const FormatOptions& opts, const int n, const Contract::Alias& s) {
	return opts.indent(n) + opts.colorize(s.name, FormatOptions::Highlight::TypeDef) + " = " + typeDef(opts, n, s.type) + ";";
}

static inline std::string formatSessionItem(const FormatOptions& opts, const int n, const Contract::Session::ForwardCall& s) {
	return "!" + signature(opts, n, s);
}

static inline std::string formatSessionItem(const FormatOptions& opts, const int n, const Contract::Session::CallBack& s) {
	return "@" + signature(opts, n, s);
}

static inline std::string formatSessionItem(const FormatOptions& opts, const int n, const Contract::Session::Ctor& s) {
	return signature(opts, n, s);
}

static inline std::string formatItem(const FormatOptions& opts, const int n, const Contract::Session& s)
{
	return opts.indent(n) + opts.colorize(s.name, FormatOptions::Highlight::Session)
			+ "\n" + opts.indent(n) + "<\n"
			+ std::accumulate(s.items.begin(), s.items.end(), std::string{}, [n, &opts, first{true}](const std::string a, const auto &i) mutable{
				const auto ret = a + opts.indent(n + 1) + formatComent(opts, n + 1, i.first, first)
								+ std::visit([&opts, n](auto& v) {return formatSessionItem(opts, n + 1, v);}, i.second) + ";\n";
				first = false;
				return ret;
			})
			+ opts.indent(n) + ">;";
}

std::string format(const FormatOptions& opts, const std::vector<Contract>& ast)
{
	std::stringstream ss;

	for(const auto& c: ast)
	{
		ss << formatComent(opts, 0, c.docs, true) << "$" << c.name << ";" << std::endl;

		std::transform(c.items.begin(), c.items.end(), std::ostream_iterator<std::string>(ss, "\n"), [&opts](const auto& s)
		{
			return formatComent(opts, 0, s.first, false) + std::visit([&opts](const auto &s){return formatItem(opts, 0, s); }, s.second);
		});

		ss << std::endl;
	}

	return ss.str();
}
