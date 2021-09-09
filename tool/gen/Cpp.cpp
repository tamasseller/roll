#include "Cpp.h"

#include <algorithm>
#include <list>
#include <iterator>

#include <cassert>
#include <cctype>

const CodeGenCpp CodeGenCpp::instance;

static constexpr auto indentStep = 4;

inline std::string indent(const int n) {
	return std::string(n * indentStep, ' ');
}

static inline std::string capitalize(std::string str)
{
	if(str.length())
	{
		str[0] = std::toupper(str[0]);
	}

	return str;
}

static inline std::string decapitalize(std::string str)
{
	if(str.length())
	{
		str[0] = std::tolower(str[0]);
	}

	return str;
}

static constexpr auto cbSgnTypeSuffix = "_callback_t";
static inline auto cbSgnTypeName(const std::string& n) {
	return decapitalize(n) + cbSgnTypeSuffix;
}

static constexpr auto funSgnTypeSuffix = "_get_t";
static inline auto funSgnTypeName(const std::string& n) {
	return decapitalize(n) + funSgnTypeSuffix;
}

static constexpr auto actSgnTypeSuffix = "_call_t";
static inline auto actSgnTypeName(const std::string& n) {
	return decapitalize(n) + actSgnTypeSuffix;
}

static constexpr auto sessFwdSgnTypeSuffix = "_session_call_t";
static inline auto sessFwdSgnTypeName(const std::string& n) {
	return decapitalize(n) + sessFwdSgnTypeSuffix;
}

static constexpr auto sessCreateSgnTypeSuffix = "_session_create_t";
static inline auto sessCreateSgnTypeName(const std::string& n) {
	return decapitalize(n) + sessCreateSgnTypeSuffix;
}

static constexpr auto sessCbSgnTypeSuffix = "_session_callback_t";
static inline auto sessCbSgnTypeName(const std::string& n) {
	return decapitalize(n) + sessCbSgnTypeSuffix;
}

static constexpr auto sessAcceptSgnTypeSuffix = "_session_accept_t";
static inline auto sessAcceptSgnTypeName(const std::string& n) {
	return decapitalize(n) + sessAcceptSgnTypeSuffix;
}

void printDocs(std::stringstream &ss, const std::string& str, const int n)
{
	if(str.length())
	{
		std::stringstream in(str);

		std::string line;
		bool gotLine = (bool)std::getline(in, line, '\n');
		assert(gotLine);

		bool first = true;
		while(true)
		{
			if(first)
			{
				ss << indent(n) << "/* ";
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
				ss << std::endl << indent(n);
				line = std::move(next);
			}
			else
			{
				break;
			}
		}

		ss << " */" << std::endl;
	}
}

struct CommonTypeGenerator
{
	static inline std::string handleTypeRef(const Contract::Primitive& p)
	{
		std::stringstream ss;

		if(!p.isSigned)
			ss << "u";

		ss << "int" << std::to_string(p.length * 8) << "_t";
		return ss.str();
	}

	static inline std::string handleTypeRef(const std::string &n) {
		return capitalize(n);
	}

	static inline std::string handleTypeRef(const Contract::Collection &c)
	{
		std::stringstream ss;
		ss << "rpc::CollectionPlaceholder<" << std::visit([](const auto& e){return handleTypeRef(e);}, *c.elementType) << ">";
		return ss.str();
	}

	static inline std::string handleTypeDef(const std::string& name, const Contract::Aggregate& a, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "struct " << capitalize(name) << std::endl;
		ss << indent(n) << "{" << std::endl;

		bool first = true;
		for(const auto& v: a.members)
		{
			if(first)
			{
				first = false;
			}
			else
			{
				if(v.docs.length())
				{
					ss << std::endl;
				}
			}

			printDocs(ss, v.docs, n + 1);
			ss << indent(n + 1) << std::visit([](const auto& i){ return handleTypeRef(i); }, v.type);
			ss << " " << decapitalize(v.name) << ";";
		}

		ss << std::endl << indent(n) << "}";
		return ss.str();
	}

	template<class T>
	static inline std::string handleTypeDef(const std::string& name, const T& t, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "using " << capitalize(name) << " = " << handleTypeRef(t);
		return ss.str();
	}

	static inline std::string handleItem(const Contract::Alias &a, const int n) {
		return std::visit([name{a.name}, n](const auto &t){ return handleTypeDef(name, t, n); }, a.type) + ";";
	}

	static inline std::array<std::string, 2> toSgnArg(const Contract::Var& a) {
		return {decapitalize(a.name), std::visit([](const auto& t){return handleTypeRef(t);}, a.type)};
	}

	static inline std::vector<std::array<std::string, 2>> toSignArgList(const std::vector<Contract::Var>& args)
	{
		std::vector<std::array<std::string, 2>> ret;
		std::transform(args.begin(), args.end(), std::back_inserter(ret), toSgnArg);
		return ret;
	}

	static inline std::string signature(const std::string &name, const decltype(toSignArgList({})) &args, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "using " << name << " = rpc::Call";

		if(args.size() > 1)
		{
			ss << std::endl << indent(n) << "<" << std::endl << indent(n + 1);
		}
		else
		{
			ss << "<";
		}

		std::list<size_t> lengths;
		std::transform(args.begin(), args.end(), std::back_inserter(lengths), [](const auto &a){ return a[0].length(); });
		const auto width = *std::max_element(lengths.begin(), lengths.end());

		for(auto i = 0u; i < args.size(); i++)
		{
			const auto& v = args[i];
			const auto& name = v[0];
			const auto p = width - name.length();
			ss  << "/* " << name << std::string(p, ' ') << " */ " << v[1];

			if(i != args.size() - 1)
			{
				ss << ',' << std::endl << indent(n + 1);
			}
			else if(args.size() > 1)
			{
				ss << std::endl << indent(n);
			}
		}

		ss << ">;";
		return ss.str();
	}

	static inline std::string handleItem(const Contract::Function &f, const int n)
	{
		std::stringstream ss;
		if(f.returnType)
		{
			std::string cbTypeName = cbSgnTypeName(f.name);
			ss << signature(cbTypeName, {toSgnArg({"retval", *f.returnType, ""})}, n) << std::endl;
			auto args = toSignArgList(f.args);
			args.push_back({"callback", cbTypeName});
			ss << signature(funSgnTypeName(f.name), args, n);
		}
		else
		{
			ss << signature(actSgnTypeName(f.name), toSignArgList(f.args), n);
		}

		return ss.str();
	}

	struct SessionCalls {
		std::vector<std::array<std::string, 2>> fwd, bwd;
	};

	static inline std::string handleSessionItemInitial(SessionCalls &calls, const std::string& docs, const Contract::Session::Ctor &c, const int n) { return {}; }

	static inline std::string handleSessionItemInitial(SessionCalls &calls, const std::string& docs, const Contract::Session::ForwardCall &f, const int n)
	{
		std::stringstream ss;
		printDocs(ss, docs, n);
		const auto typeName = sessFwdSgnTypeName(f.name);
		ss << signature(typeName, toSignArgList(f.args), n);
		calls.fwd.push_back({f.name, typeName});
		return ss.str();
	}

	static inline std::string handleSessionItemInitial(SessionCalls &calls, const std::string& docs, const Contract::Session::CallBack & cb, const int n)
	{
		std::stringstream ss;
		printDocs(ss, docs, n);
		const auto typeName = sessCbSgnTypeName(cb.name);
		ss << signature(typeName, toSignArgList(cb.args), n);
		calls.bwd.push_back({cb.name, typeName});
		return ss.str();
	}

	static inline std::string handleSessionItemFinal(SessionCalls &calls, const std::string& docs, const Contract::Session::Ctor & c, const int n)
	{
		std::stringstream ss;
		printDocs(ss, docs, n);

		auto fwdArgs = toSignArgList(c.args);
		std::copy(calls.bwd.begin(), calls.bwd.end(), std::back_inserter(fwdArgs));
		ss << signature(sessCreateSgnTypeName(c.name), fwdArgs, n) << std::endl;

		std::vector<std::array<std::string, 2>> bwdArgs;
		if(c.returnType)
		{
			bwdArgs.push_back(toSgnArg({"retval", *c.returnType, ""}));
		}

		ss << signature(sessAcceptSgnTypeName(c.name), bwdArgs, n);
		return ss.str();
	}

	static inline std::string handleSessionItemFinal(SessionCalls &calls, const std::string& docs, const Contract::Session::ForwardCall&, const int n) { return {}; }
	static inline std::string handleSessionItemFinal(SessionCalls &calls, const std::string& docs, const Contract::Session::CallBack&, const int n) { return {}; }

	static inline std::string handleItem(const Contract::Session &s, const int n)
	{
		std::stringstream ss;

		SessionCalls scs;

		ss << indent(n) << "struct " << capitalize(s.name) << std::endl;
		ss << indent(n) << "{" << std::endl;

		bool first = true;
		for(const auto& it: s.items)
		{
			const auto str = std::visit([&scs, n, docs{it.first}](const auto& i){ return handleSessionItemInitial(scs, docs, i, n + 1); }, it.second);

			if(str.length())
			{
				if(first)
				{
					first = false;
				}
				else
				{
					ss << std::endl;
				}

				ss << str << std::endl;
			}
		}

		for(const auto& it: s.items)
		{
			const auto str = std::visit([&scs, n, docs{it.first}](const auto& i){ return handleSessionItemFinal(scs, docs, i, n + 1); }, it.second);

			if(str.length())
			{
				ss << std::endl << str << std::endl;
			}
		}

		ss << indent(n) << "};";

		return ss.str();
	}

	static inline void writeTypes(std::stringstream &ss, const std::vector<Contract>& cs)
	{
		for(const auto& c: cs)
		{
			printDocs(ss, c.docs, 0);
			ss << "struct " << capitalize(c.name) << std::endl;
			ss << "{" << std::endl;

			bool first = true;
			for(const auto& i: c.items)
			{
				if(first)
				{
					first = false;
				}
				else
				{
					ss << std::endl;
				}

				printDocs(ss, i.first, 1);
				ss << std::visit([](const auto& i){ return handleItem(i, 1); }, i.second) << std::endl;
			}

			ss << "};" << std::endl << std::endl;
		}
	}
};

void writeHeader(std::stringstream &ss)
{
	ss << "#include \"RpcCall.h\"" << std::endl;
	ss << "#include \"RpcSymbol.h\"" << std::endl;
	ss << "#include \"RpcTypeInfo.h\"" << std::endl;
	ss << std::endl;
}

std::string CodeGenCpp::generateClient(const std::vector<Contract>& cs) const
{
	std::stringstream ss;
	writeHeader(ss);
	CommonTypeGenerator::writeTypes(ss, cs);
	return ss.str();
}

std::string CodeGenCpp::generateServer(const std::vector<Contract>& cs) const
{
	std::stringstream ss;
	writeHeader(ss);
	CommonTypeGenerator::writeTypes(ss, cs);
	return ss.str();
}
