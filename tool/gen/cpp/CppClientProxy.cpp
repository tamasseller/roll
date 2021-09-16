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

		const auto s = contractSymbolsBlockNameRef(c.name);

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
		ss << printDocs(docs, n);

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
			ss << printDocs(docs, n);
			writeFutureCall(ss, f.name, argInfo, cppRetType, refRetType, n);
		}

		ret.push_back(ss.str());
	}

	static inline void writeFutureCreate(std::stringstream &ss, const Contract::Session &s, const Contract::Function &f , const std::string& cName, const int n)
	{
		ss << indent(n) << "template<" << (f.returnType.has_value() ? "class Ret, " : "") << "class S";

		for(auto i = 0u; i < f.args.size(); i++)
		{
			ss << ", class A" << i;
		}

		ss << ">" << std::endl;

		const auto defName = definitionMemberFunctionName(f.name);
		ss << indent(n) << "inline auto " << defName << "(S _object";

		for(auto i = 0u; i < f.args.size(); i++)
		{
			ss << ", A" << i << "&& " << argumentName(f.args[i].name);
		}

		ss << ")" << std::endl;
		ss << indent(n) << "{" << std::endl;

		const auto sObj = clientSessionName(cName, s.name);
		ss << indent(n + 1) << "static_assert(rpc::hasCrtpBase<" << sObj << ", decltype(*_object)>, \"The first argument to "
				<< defName << " must be a pointer-like object to a CRTP subclass of " << sObj << "\");" << std::endl;

		for(auto i = 0u; i < f.args.size(); i++)
		{
			const auto cppTypeName = std::visit([&cName](const auto &i){return cppTypeRef(i, cName);}, f.args[i].type);
			const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, f.args[i].type);
			const auto msg = "Argument #" + std::to_string(i + 2) + " to " + defName + " must have type compatible with '" + refTypeName + "'";
			ss << argCheck("A" + std::to_string(i), cppTypeName, msg , n + 1);
		}

		const auto sym = callMemberName(sessionCtorApiName(s.name, f.name));

		if(f.returnType.has_value())
		{
			const auto cppTypeName = std::visit([&cName](const auto &i){return cppTypeRef(i, cName);}, f.returnType.value());
			const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, f.returnType.value());
			const auto msg = "Return type of " + defName + " must be compatible with '" + refTypeName + "'";
			ss << argCheck("Ret", cppTypeName, msg , n + 1);
			ss << indent(n + 1) << "return this->template createWithPromiseRetval<Ret>(" << sym << ", _object";
		}
		else
		{
			ss << indent(n + 1) << "return this->createWithPromise(" << sym << ", _object";
		}

		for(auto i = 0u; i < f.args.size(); i++)
		{
			ss << ", std::forward<A" << i << ">(" << f.args[i].name << ")";
		}

		ss << ");" << std::endl;

		ss << indent(n) << "}";
	}

	static inline void writeCallbackCreate(std::stringstream &ss, const Contract::Session &s, const Contract::Function &f , const std::string& cName, const int n)
	{
		ss << indent(n) << "template<class S";

		for(auto i = 0u; i < f.args.size(); i++)
		{
			ss << ", class A" << i;
		}

		ss << ", class C>" << std::endl;

		const auto defName = definitionMemberFunctionName(f.name);
		ss << indent(n) << "inline auto " << defName << "(S _object";

		for(auto i = 0u; i < f.args.size(); i++)
		{
			ss << ", A" << i << "&& " << argumentName(f.args[i].name);
		}

		ss << ", C&& _cb)" << std::endl;
		ss << indent(n) << "{" << std::endl;

		const auto sObj = clientSessionName(cName, s.name);
		ss << indent(n + 1) << "static_assert(rpc::hasCrtpBase<" << sObj << ", decltype(*_object)>, \"The first argument to "
				<< defName << " must be a pointer-like object to a CRTP subclass of " << sObj << "\");" << std::endl;

		for(auto i = 0u; i < f.args.size(); i++)
		{
			const auto cppTypeName = std::visit([&cName](const auto &i){return cppTypeRef(i, cName);}, f.args[i].type);
			const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, f.args[i].type);
			const auto msg = "Argument #" + std::to_string(i + 2) + " to " + defName + " must have type compatible with '" + refTypeName + "'";
			ss << argCheck("A" + std::to_string(i), cppTypeName, msg , n + 1);
		}

		const auto sym = callMemberName(sessionCtorApiName(s.name, f.name));

		if(f.returnType.has_value())
		{
			const auto cppTypeName = std::visit([&cName](const auto &i){return cppTypeRef(i, cName);}, f.returnType.value());
			const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, f.returnType.value());
			const auto msg = "Callback for " + defName + " must take an argument compatible with '" + refTypeName + "'";
			ss << argCheck("rpc::Arg<0, &C::operator()>", cppTypeName, msg , n + 1);
			ss << indent(n + 1) << "return this->createWithCallbackRetval(" << sym << ", _object, std::move(_cb)";
		}
		else
		{
			ss << indent(n + 1) << "return this->createWithCallback(" << sym << ", _object, std::move(_cb)";
		}

		for(auto i = 0u; i < f.args.size(); i++)
		{
			ss << ", std::forward<A" << i << ">(" << f.args[i].name << ")";
		}

		ss << ");" << std::endl;

		ss << indent(n) << "}";
	}

	static inline void handleItem(std::vector<std::string> &ret, const Contract::Session &s, const std::string& docs, const std::string& cName, const int n)
	{
		for(const auto i: s.items)
		{
			if(const Contract::Function* f = std::get_if<Contract::Session::Ctor>(&i.second))
			{
				{
					std::stringstream ss;
					ss << printDocs(i.first, n);
					writeCallbackCreate(ss, s, *f, cName, n);
					ret.push_back(ss.str());
				}

				{
					std::stringstream ss;
					ss << printDocs(i.first, n);
					writeFutureCreate(ss, s, *f, cName, n);
					ret.push_back(ss.str());
				}
			}
		}
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
	const auto n = contractClientProxyNameRef(c.name);
	const auto symRefs = SymbolReferenceExtractor::gatherSymbolReferences(c);
	const auto funDefs = MemberFunctionGenerator::generateFunctionDefinitions(c);

	if(symRefs.size())
	{
		std::vector<std::string> result;

		result.push_back(indent(1) + "template<class T> using Link = typename rpc::ClientBase<Adapter>::template OnDemand<T>;");
		std::copy(symRefs.begin(), symRefs.end(), std::back_inserter(result));

		std::stringstream ctor;
		ctor << "public:" << std::endl;
		ctor << indent(1) << "using " << contractClientProxyNameDef(n) << "::ClientBase::ClientBase;";
		result.push_back(ctor.str());
		std::copy(funDefs.begin(), funDefs.end(), std::back_inserter(result));

		ss << printDocs(c.docs, 0);
		writeTopLevelBlock(ss, "template<class Adapter> class " + n + ": public rpc::ClientBase<Adapter>", result);
	}
}
