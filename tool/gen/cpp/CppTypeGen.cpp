#include "CppTypeGen.h"

#include "CppCommon.h"

#include <algorithm>
#include <list>

struct CommonTypeGenerator
{
	static inline std::string handleTypeRef(const std::string &n) { return userTypeName(n); }
	static inline std::string handleTypeRef(const Contract::Primitive& p) { return cppPrimitive(p); }

	static inline std::string handleTypeRef(const Contract::Collection &c) {
		return "rpc::CollectionPlaceholder<" + std::visit([](const auto& e){return handleTypeRef(e);}, *c.elementType) + ">";
	}

	static inline std::string handleTypeDef(const std::string& name, const Contract::Aggregate& a, const int n)
	{
		std::vector<std::string> result;

		for(const auto& v: a.members)
		{
			std::stringstream ss;
			ss << printDocs(v.docs, n + 1);
			ss << indent(n + 1) << std::visit([](const auto& i){ return handleTypeRef(i); }, v.type);
			ss << " " << aggregateMemberName(v.name) << ";";
			result.push_back(ss.str());
		}

		std::stringstream ss;
		writeBlock(ss, "struct " + userTypeName(name), result, n);
		return ss.str();
	}

	template<class T>
	static inline std::string handleTypeDef(const std::string& name, const T& t, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "using " << userTypeName(name) << " = " << handleTypeRef(t);
		return ss.str();
	}

	static inline std::string handleItem(const Contract::Alias &a, const int n) {
		return std::visit([name{a.name}, n](const auto &t){ return handleTypeDef(name, t, n); }, a.type) + ";";
	}

	static inline std::array<std::string, 2> toSgnArg(const Contract::Var& a) {
		return {argumentName(a.name), std::visit([](const auto& t){return handleTypeRef(t);}, a.type)};
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
			std::string cbTypeName = callbackSignatureTypeName(f.name);
			ss << signature(cbTypeName, {toSgnArg({"retval", *f.returnType, ""})}, n) << std::endl;
			auto args = toSignArgList(f.args);
			args.push_back({"callback", cbTypeName});
			const auto type = functionSignatureTypeName(f.name);
			ss << signature(type, args, n);
		}
		else
		{
			const auto type = actionSignatureTypeName(f.name);
			ss << signature(type, toSignArgList(f.args), n);
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
		ss << printDocs(docs, n);
		const auto typeName = sessionForwardCallSignatureTypeName(f.name);
		ss << signature(typeName, toSignArgList(f.args), n);
		calls.fwd.push_back({f.name, typeName});
		return ss.str();
	}

	static inline std::string handleSessionItemInitial(SessionCalls &calls, const std::string& docs, const Contract::Session::CallBack & cb, const int n)
	{
		std::stringstream ss;
		ss << printDocs(docs, n);
		const auto typeName = sessionCallbackSignatureTypeName(cb.name);
		ss << signature(typeName, toSignArgList(cb.args), n);
		calls.bwd.push_back({cb.name, typeName});
		return ss.str();
	}

	static inline std::string handleSessionItemFinal(const std::string& sName, const std::string& docs, const Contract::Session::Ctor & c, const int n)
	{
		std::stringstream ss;
		ss << printDocs(docs, n);

		std::vector<std::array<std::string, 2>> bwdArgs;
		if(c.returnType)
		{
			bwdArgs.push_back(toSgnArg({"_retval", *c.returnType, ""}));
		}

		bwdArgs.push_back({"_exports", sessionCallExportTypeName(sName)});

		ss << signature(sessionAcceptSignatureTypeName(c.name), bwdArgs, n) << std::endl;

		auto fwdArgs = toSignArgList(c.args);
		fwdArgs.push_back({"_exports", sessionCallbackExportTypeName(sName)});
		fwdArgs.push_back({"_accept", sessionAcceptSignatureTypeName(c.name)});

		ss << signature(sessionCreateSignatureTypeName(c.name), fwdArgs, n);

		return ss.str();
	}

	static inline std::string handleSessionItemFinal(const std::string& sName, const std::string& docs, const Contract::Session::ForwardCall&, const int n) { return {}; }
	static inline std::string handleSessionItemFinal(const std::string& sName, const std::string& docs, const Contract::Session::CallBack&, const int n) { return {}; }

	static inline std::string sessionExports(const std::string& name, const std::vector<std::array<std::string, 2>> &d, const int n)
	{
		std::vector<std::string> result;

		std::transform(d.begin(), d.end(), std::back_inserter(result), [n](const auto &i){
			return indent(n + 1) + i[1] + " " + i[0] + ";";
		});

		result.push_back(indent(n + 1) + "rpc::Call<> _close;");

		std::stringstream ss;

		if(writeBlock(ss, "struct " + name, result, n))
		{
			ss << ";";
		}

		return ss.str();
	}

	static inline std::string handleItem(const Contract::Session &s, const int n)
	{
		std::vector<std::string> result;

		SessionCalls scs;

		for(const auto& it: s.items) {
			result.push_back(std::visit([&scs, n, docs{it.first}](const auto& i){ return handleSessionItemInitial(scs, docs, i, n + 1); }, it.second));
		}

		result.push_back(sessionExports(sessionCallExportTypeName(s.name), scs.fwd, n + 1));
		result.push_back(sessionExports(sessionCallbackExportTypeName(s.name), scs.bwd, n + 1));

		for(const auto& it: s.items) {
			result.push_back(std::visit([&s, n, docs{it.first}](const auto& i){ return handleSessionItemFinal(s.name, docs, i, n + 1); }, it.second));
		}

		std::stringstream ss;
		writeBlock(ss, "struct " + sessionNamespaceName(s.name), result, n);
		ss << ";";
		return ss.str();
	}
};

void writeContractTypes(std::stringstream &ss, const Contract& c)
{
	std::vector<std::string> result;
	for(const auto& i: c.items)
	{
		std::stringstream ss;
		ss << printDocs(i.first, 1);
		ss << std::visit([](const auto& i){ return CommonTypeGenerator::handleItem(i, 1); }, i.second);
		result.push_back(ss.str());
	}

	writeTopLevelBlock(ss, printDocs(c.docs, 0) + "struct " + contractTypeBlockNameRef(c.name), result);
}
