#include "CppServerProxy.h"

#include "CppCommon.h"
#include "CppProxyCommon.h"

namespace ServiceBuilderGenerator
{
	static inline void writeArgsCheckList(std::stringstream& ss, const Contract::Action& a, const std::string& cName, const int n)
	{
		const auto count = a.args.size();

		ss << indent(n) << "static_assert(rpc::nArgs<&Child::" << definitionMemberFunctionName(a.name) << "> == " << count << ", "
			<< "\"Public method " << a.name << " must take " << count << " argument"
			<< ((count > 1) ? "s" : "") << "\");" << std::endl;

		for(auto i = 0u; i < a.args.size(); i++)
		{
			const auto cppType = std::visit([&cName](const auto& i) { return cppTypeRef(i, cName); }, a.args[i].type);
			const auto refType = std::visit([&cName](const auto& i) { return refTypeRef(i); }, a.args[i].type);
			const auto msg = "Argument #" + std::to_string(i + 1) + " to public method " + a.name + " (" + a.args[i].name + ") must have type compatible with '" + refType + "'";
			ss << argCheck("rpc::Arg<" + std::to_string(i) + ", &Child::" + definitionMemberFunctionName(a.name) + ">", cppType, msg, n);
		}
	}

	static inline std::string provideLine(
			const std::string &kind,
			const std::string &symName,
			const std::string &defName,
			const std::vector<Contract::Var>& args,
			std::vector<std::string> extra = {})
	{
		std::stringstream ss;

		ss << "this->template provide" << kind << "<" << symName << ", Child, &Child::" << defName;

		for(const auto& s: extra)
		{
			ss << ", " << s;
		}

		for(auto i = 0u; i < args.size(); i++)
		{
			ss << ", rpc::Arg<" << std::to_string(i) << ", &Child::" << defName << ">";
		}

		ss << ">();";
		return ss.str();
	}

	static inline void handleItem(std::vector<std::string> &ret, const Contract::Function &f, const std::string& cName, const int n)
	{
		std::stringstream ss;

		ss << indent(n) << "{" << std::endl;
		writeArgsCheckList(ss, f, cName, n + 1);

		const auto defName = definitionMemberFunctionName(f.name);
		const auto symName = contractSymbolsBlockNameRef(cName) + "::" + symbolName(f.name);

		if(!f.returnType.has_value())
		{
			ss << indent(n + 1) << provideLine("Action", symName, defName, f.args);
		}
		else
		{
			const auto cppRetType = std::visit([&cName](const auto& i) { return cppTypeRef(i, cName); }, f.returnType.value());
			const auto refRetType = std::visit([&cName](const auto& i) { return refTypeRef(i); }, f.returnType.value());
			ss << argCheck("rpc::Ret<&Child::" + defName + ">", cppRetType, "Return type of " + f.name + " must be compatible with '" + refRetType + "'", n + 1);
			ss << indent(n + 1) << provideLine("Function", symName, defName, f.args, {cppRetType});
		}

		ss << std::endl << indent(n) << "}";

		ret.push_back(ss.str());
	}

	static inline void handleItem(std::vector<std::string> &ret, const Contract::Session &s, const std::string& cName, const int n)
	{
		for(const auto& i: s.items)
		{
			if(const Contract::Function* f = std::get_if<Contract::Session::Ctor>(&i.second))
			{
				std::stringstream ss;

				const auto defName = definitionMemberFunctionName(f->name);
				const auto sObj = serverSessionName(cName, s.name);
				const auto symName = contractSymbolsBlockNameRef(cName) + "::" + sessionNamespaceName(s.name) + "::" + symbolName(f->name);

				ss << indent(n) << "{" << std::endl;

				writeArgsCheckList(ss, *f, cName, n + 1);

				const auto exportsExtra = contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(s.name) + "::" + sessionCallbackExportTypeName(s.name);
				const auto acceptExtra = contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(s.name) + "::" + sessionAcceptSignatureTypeName(f->name);

				if(!f->returnType.has_value())
				{
					ss << indent(n + 1) << "static_assert(rpc::hasCrtpBase<" << sObj << ", decltype(*std::declval<rpc::Ret<&Child::"
						<< defName << ">>())>, \"Session constructor " << f->name << " for " << s.name
						<< " session must return a pointer-like object to a CRTP subclass of " << sObj << "\");" << std::endl;

					ss << indent(n + 1) << provideLine("Ctor", symName, defName, f->args, {exportsExtra, acceptExtra});
				}
				else
				{
					const auto cppTypeName = std::visit([&cName](const auto &i){return cppTypeRef(i, cName);}, f->returnType.value());
					const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, f->returnType.value());

					ss << indent(n + 1) << "using RetType = typename rpc::Ret<&Child::" << f->name << ">;" << std::endl;
					ss << indent(n + 1) << "static constexpr bool _retValOk = rpc::isCompatible<decltype(std::declval<RetType>().first), " << cppTypeName << ">();" << std::endl;
					ss << indent(n + 1) << "static constexpr bool _objectOk = rpc::hasCrtpBase<" << sObj << ", decltype(*std::declval<RetType>().second)>;" << std::endl;
					ss << indent(n + 1) << "static_assert(_retValOk && _objectOk, \"Session constructor "
							<< f->name << " for " << s.name << " session must return a pair of a value compatible with " << refTypeName
							<< " and pointer-like object to a CRTP subclass of "  << sObj << "\");" << std::endl;

					ss << indent(n + 1) << provideLine("CtorWithRetval", symName, defName, f->args, {exportsExtra, acceptExtra});
				}

				ss << std::endl << indent(n) << "}";

				ret.push_back(ss.str());
			}
		}
	}

	template<class C> static inline void handleItem(std::vector<std::string> &, const C&, const std::string&, const int n) {}

	static inline std::string generateCtor(const Contract& c, const std::string& name)
	{
		std::vector<std::string> blocks;

		const auto header = "template<class... Args>\n" +
				indent(1) + name + "(Args&&... args):\n" +
				indent(2) + name + "::ServiceBase(std::forward<Args>(args)...)";

		for(const auto& i: c.items) {
			std::visit([&blocks, &c](const auto i){handleItem(blocks, i, c.name, 2);}, i.second);
		}

		std::stringstream ss;
		writeBlock(ss, header, blocks, 1);
		return ss.str();
	}
}

namespace ServiceDemolisherGenerator
{
	static inline void handleItem(std::vector<std::string> &ret, const Contract::Function &f, const std::string& cName, const int n)
	{
		std::stringstream ss;
		const auto symName = contractSymbolsBlockNameRef(cName) + "::" + symbolName(f.name);
		ss << indent(n) << "this->discard(" << symName << ");";
		ret.push_back(ss.str());
	}

	static inline void handleItem(std::vector<std::string> &ret, const Contract::Session &s, const std::string& cName, const int n)
	{
		for(const auto& i: s.items)
		{
			if(const Contract::Function* f = std::get_if<Contract::Session::Ctor>(&i.second))
			{
				std::stringstream ss;
				const auto symName = contractSymbolsBlockNameRef(cName) + "::" + sessionNamespaceName(s.name) + "::" + symbolName(f->name);
				ss << indent(n) << "this->discard(" << symName << ");";
				ret.push_back(ss.str());
			}
		}
	}

	template<class C> static inline void handleItem(std::vector<std::string> &, const C&, const std::string&, const int n) {}

	static inline std::string generateDtor(const Contract& c, const std::string& name)
	{
		std::vector<std::string> blocks;

		for(const auto& i: c.items) {
			std::visit([&blocks, &c](const auto i){handleItem(blocks, i, c.name, 2);}, i.second);
		}

		std::stringstream ss;
		writeBlock(ss, "~" + name + "()", blocks, 1);
		return ss.str();
	}
}


void writeServerProxy(std::stringstream& ss, const Contract& c)
{
	const auto n = contractServerProxyNameDef(c.name);

	auto ctor = ServiceBuilderGenerator::generateCtor(c, n);

	if(ctor.length())
	{
		ss << printDocs(c.docs, 0);

		const auto header = "template<class Child, class Adapter>\nstruct " + contractServerProxyNameRef(c.name) + ": rpc::ServiceBase<Adapter>";

		std::vector<std::string> result;
		result.push_back(ctor);
		result.push_back(ServiceDemolisherGenerator::generateDtor(c, n));
		writeTopLevelBlock(ss, header, result);
	}
}
