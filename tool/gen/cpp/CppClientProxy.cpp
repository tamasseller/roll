#include "CppClientProxy.h"

#include "CppCommon.h"
#include "CppProxyCommon.h"

#include <algorithm>

struct SymbolReferenceExtractor
{
	using SymRef = std::array<std::string, 2>;

	static inline void handleItem(std::vector<SymRef> &ret, const Contract::Function &f, const std::string& s) {
		ret.push_back({f.name, s + "::" + symbolName(f.name)});
	}

	static inline void handleSessionItem(std::vector<SymRef> &ret, const Contract::Session::Ctor & c, const std::string& cs, const std::string& sName)
	{
		const auto name = sessionCtorApiName(sName, c.name);
		const auto symbol = cs + "::" + sessionNamespaceName(sName) + "::" + symbolName(c.name);
		ret.push_back({name, symbol});
	}

	template<class C> static inline void handleSessionItem(std::vector<SymRef> &ret, const C&, const std::string&, const std::string&) {}

	static inline void handleItem(std::vector<SymRef> &ret, const Contract::Session &s, const std::string& cs)
	{
		for(const auto& i: s.items) {
			std::visit([&ret, &cs, sName{s.name}](const auto& i){ return handleSessionItem(ret, i, cs, sName); }, i.second);
		}
	}

	template<class C> static inline void handleItem(std::vector<SymRef> &ret, const C&, const std::string&) {}

	static inline auto gatherSymbolReferences(const Contract& c)
	{
		std::vector<SymRef> coll;

		const auto s = contractSymbolsNamespaceName(c.name);

		for(const auto& i: c.items) {
			std::visit([&coll, &s](const auto i){handleItem(coll, i, s);}, i.second);
		}

		std::vector<std::string> strs;
		std::transform(coll.begin(), coll.end(), std::back_inserter(strs), [](const auto& it) {
			return indent(1) + "Link<decltype(" + it[1] + ")> " + callMemberName(it[0]) + " = " + it[1] + ";";
		});

		return strs;
	}
};

struct MemberFunctionGenerator
{
//
//	static inline void handleSessionItem(std::vector<SymRef> &ret, const Contract::Session::Ctor & c, const std::string& cs, const std::string& sName)
//	{
//		const auto name = sessionCtorApiName(sName, c.name);
//		const auto symbol = cs + "::" + sessionNamespaceName(sName) + "::" + symbolName(c.name);
//		ret.push_back({name, symbol});
//	}
//
//	template<class C> static inline void handleSessionItem(std::vector<SymRef> &ret, const C&, const std::string&, const std::string&) {}
//
//	static inline void handleItem(std::vector<SymRef> &ret, const Contract::Session &s, const std::string& cName)
//	{
//		for(const auto& i: s.items) {
//			std::visit([&ret, &cName, sName{s.name}, docs{i.first}](const auto& i){ return handleSessionItem(ret, i, cName, sName, docs); }, i.second);
//		}
//	}
	using ArgInfo = std::array<std::string, 3>;

	static inline std::string templateArgList(int n, std::optional<std::string> extra = {})
	{
		std::stringstream ss;
		ss << "template<";

		std::string sep = "";
		for(int i = 0; i < n; i++)
		{
			ss << sep << "class A" << std::to_string(i);
			sep = ", ";
		}

		if(extra)
		{
			ss << sep << *extra;
		}

		ss << ">" << std::endl;
		return ss.str();
	}

	static inline std::string functionArgList(const std::vector<ArgInfo>& args, std::optional<std::string> extra = {})
	{
		std::stringstream ss;
		ss << "(";

		std::string sep = "";
		for(auto i = 0u; i < args.size(); i++)
		{
			ss << sep << "A" << std::to_string(i) << "&& " << argumentName(args[i][1]);
			sep = ", ";
		}

		if(extra)
		{
			ss << sep << *extra;
		}

		ss << ")" << std::endl;
		return ss.str();
	}

	static inline std::string argCheckList(const std::string& name, const std::vector<ArgInfo>& args, const int n)
	{
		std::stringstream ss;

		for(auto i = 0u; i < args.size(); i++)
		{
			const auto msg = "Argument #" + std::to_string(i + 1) + " of " + name
					+ " must have type compatible with '" + args[i][2] + "'";

			ss << argCheck("A" + std::to_string(i), args[i][0], msg , n);
		}

		return ss.str();
	}

	static inline std::string invocationArgList(const std::vector<ArgInfo>& args, std::optional<std::string> extra = {})
	{
		std::stringstream ss;

		for(auto i = 0u; i < args.size(); i++)
		{
			ss << ", " << "std::forward<A" << std::to_string(i) << ">(" << args[i][1] << ")";
		}

		if(extra)
		{
			ss << ", " << *extra;
		}

		return ss.str();
	}

	static inline void writeActionCall(std::stringstream &ss, const std::string& name, const std::vector<ArgInfo>& args, const int n)
	{
		if(args.size())
		{
			ss << indent(n) << templateArgList(args.size());
		}

		ss << indent(n) << "inline auto " << invocationMemberFunctionName(name) << functionArgList(args);
		ss << indent(n) << "{" << std::endl;
		ss << argCheckList(name, args, n + 1);
		ss << indent(n + 1) << "return this->callAction(" << callMemberName(name) << invocationArgList(args) << ");" << std::endl;
		ss << indent(n) << "}";
	}

	static inline void writeCallbackCall(
			std::stringstream &ss,
			const std::string& name,
			const std::vector<ArgInfo>& args,
			const std::string& cppRetType,
			const std::string& refRetType,
			const int n
	) {
		if(args.size())
		{
			ss << indent(n) << templateArgList(args.size(), "class C");
		}

		ss << indent(n) << "inline auto " << invocationMemberFunctionName(name) << functionArgList(args, "C&& _cb");
		ss << indent(n) << "{" << std::endl;
		ss << argCheckList(name, args, n + 1);
		ss << argCheck("rpc::Arg<0, &C::operator()>", cppRetType, "Callback for " + name + " must take a first argument compatible with '" + refRetType + "'", n + 1);
		ss << indent(n + 1) << "return this->callWithCallback(" << callMemberName(name) << ", std::move(_cb)" << invocationArgList(args) << ");" << std::endl;
		ss << indent(n) << "}";
	}

	static inline void writeFutureCall(
			std::stringstream &ss,
			const std::string& name,
			const std::vector<ArgInfo>& args,
			const std::string& cppRetType,
			const std::string& refRetType,
			const int n
	) {
		ss << indent(n) << "template<class Ret";

		for(auto i = 0u; i < args.size(); i++)
		{
			ss << ", " << "class A" << std::to_string(i);
		}

		ss << ">" << std::endl;

		ss << indent(n) << "inline auto " << invocationMemberFunctionName(name) << functionArgList(args);
		ss << indent(n) << "{" << std::endl;
		ss << argCheckList(name, args, n + 1);
		ss << argCheck("Ret", cppRetType, "Return type of " + name + " must be compatible with '" + refRetType + "'", n + 1);
		ss << indent(n + 1) << "return this->template callWithPromise<Ret>(" << callMemberName(name) << invocationArgList(args) << ");" << std::endl;
		ss << indent(n) << "}";
	}

	static inline void handleItem(std::vector<std::string> &ret, const Contract::Function &f, const std::string& docs, const std::string& cName, const int n)
	{
		std::vector<ArgInfo> argInfo;

		std::transform(f.args.begin(), f.args.end(), std::back_inserter(argInfo), [&cName](const auto& a)
		{
			const auto cppType = std::visit([&cName](const auto& i) { return cppTypeRef(i, cName); }, a.type);
			const auto refType = std::visit([&cName](const auto& i) { return refTypeRef(i); }, a.type);
			return ArgInfo{cppType, a.name, refType};
		});

		std::stringstream ss;
		printDocs(ss, docs, n);

		if(!f.returnType.has_value())
		{
			writeActionCall(ss, f.name, argInfo, n);
		}
		else
		{
			const auto cppRetType = std::visit([&cName](const auto& i) { return cppTypeRef(i, cName); }, f.returnType.value());
			const auto refRetType = std::visit([&cName](const auto& i) { return refTypeRef(i); }, f.returnType.value());
			writeCallbackCall(ss, f.name, argInfo, cppRetType, refRetType, n);
			ss << std::endl << std::endl;
			printDocs(ss, docs, n);
			writeFutureCall(ss, f.name, argInfo, cppRetType, refRetType, n);
		}

		ret.push_back(ss.str());
	}

	template<class C> static inline void handleItem(std::vector<std::string> &, const C&, const std::string&, const std::string&, const int n) {}

	static inline auto generateFunctionDefinitions(const Contract& c)
	{
		std::vector<std::string> ret;

		for(const auto& i: c.items) {
			std::visit([&ret, &c, docs{i.first}](const auto i){handleItem(ret, i, docs, c.name, 1);}, i.second);
		}

		return ret;
	}
};

void writeClientProxy(std::stringstream& ss, const Contract& c)
{
	const auto n = clientProxyName(c.name);
	const auto symRefs = SymbolReferenceExtractor::gatherSymbolReferences(c);
	const auto funDefs = MemberFunctionGenerator::generateFunctionDefinitions(c);

	std::vector<std::string> result;

	if(symRefs.size())
	{
		result.push_back(indent(1) + "template<class T> using Link = typename rpc::ClientBase<Adapter>::template OnDemand<T>;");
		std::copy(symRefs.begin(), symRefs.end(), std::back_inserter(result));
	}

	std::stringstream ctor;
	ctor << "public:" << std::endl;
	ctor << indent(1) << "using " << n << "::ClientBase::ClientBase;";
	result.push_back(ctor.str());

	std::copy(funDefs.begin(), funDefs.end(), std::back_inserter(result));

	printDocs(ss, c.docs, 0);
	writeTopLevelBlock(ss, "template<class Adapter> class " + n + ": public rpc::ClientBase<Adapter>", result, true);
}
