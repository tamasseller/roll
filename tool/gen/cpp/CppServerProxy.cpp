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
			const auto msg = "Argument #" + std::to_string(i + 1) + " to public method " + a.name + " must have type compatible with '" + refType + "'";
			ss << argCheck("rpc::Arg<" + std::to_string(i) + ", &Child::" + definitionMemberFunctionName(a.name) + ">", cppType, msg, n);
		}
	}

	static inline std::string provideLine(
			const std::string &symName,
			const std::string &defName,
			const std::vector<Contract::Var>& args,
			const int n,
			std::optional<std::string> extra = {})
	{
		std::stringstream ss;

		ss << "auto _err = this->provide(" << symName << ", [self{static_cast<Child*>(this)}](Endpoint& _ep, rpc::MethodHandle";

		for(auto i = 0u; i < args.size(); i++)
		{
			ss << ", rpc::Arg<" << std::to_string(i) << ", &Child::" << defName << "> " + argumentName(args[i].name);
		}

		if(extra.has_value())
		{
			ss << ", " << *extra;
		}

		ss << ")" << std::endl;
		return ss.str();
	}

	static inline std::string invokeLine(
			const std::string &defName,
			const std::vector<Contract::Var>& args,
			const int n)
	{
		std::stringstream ss;

		ss << "self->" << defName << "(";

		auto sep = "";
		for(auto i = 0u; i < args.size(); i++)
		{
			ss << sep << "std::move(" << argumentName(args[i].name) << ")";
			sep = ", ";
		}

		ss << ")";
		return ss.str();
	}

	static inline void writeRegErrorCheck(std::stringstream &ss, const Contract::Function &f, const int n)
	{
		ss << indent(n) << "if(_err)" << std::endl;
		ss << indent(n) << "{" << std::endl;
		ss << indent(n + 1) << "rpc::fail(std::string(\"Registering public method '" << f.name << "' resulted in error \") + _err);" << std::endl;
		ss << indent(n) << "}" << std::endl;
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
			ss << std::endl << indent(n + 1) << provideLine(symName, defName, f.args, n);
			ss << indent(n + 1) << "{" << std::endl;
			ss << indent(n + 2) << invokeLine(defName, f.args, n) << ";" << std::endl;
			ss << indent(n + 1) << "});" << std::endl;
		}
		else
		{
			const auto cppRetType = std::visit([&cName](const auto& i) { return cppTypeRef(i, cName); }, f.returnType.value());
			const auto refRetType = std::visit([&cName](const auto& i) { return refTypeRef(i); }, f.returnType.value());
			ss << argCheck("rpc::Ret<&Child::" + defName + ">", cppRetType, "Return type of " + f.name + " must be compatible with '" + refRetType + "'", n + 1);

			ss << std::endl << indent(n + 1) << provideLine(symName, defName, f.args, n, "rpc::Call<" + cppRetType + "> _cb");
			ss << indent(n + 1) << "{" << std::endl;
			ss << indent(n + 2) << "if(auto _err = _ep.call(_cb, " + invokeLine(defName, f.args, n) + "))" << std::endl;
			ss << indent(n + 2) << "{" << std::endl;
			ss << indent(n + 3) << "rpc::fail(std::string(\"Calling callback of public method '" << f.name << "' resulted in error \") + _err);" << std::endl;
			ss << indent(n + 2) << "}" << std::endl;
			ss << indent(n + 1) << "});" << std::endl;
		}

		ss << std::endl;
		writeRegErrorCheck(ss, f, n + 1);
		ss << indent(n) << "}";

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

				if(!f->returnType.has_value())
				{
					ss << indent(n + 1) << "static_assert(rpc::hasCrtpBase<" << sObj << ", decltype(*std::declval<rpc::Ret<&Child::"
						<< defName << ">>())>, \"Session constructor " << f->name << " for " << s.name
						<< " session must return a pointer-like object to a CRTP subclass of " << sObj << "\");" << std::endl;
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
				}

				ss << std::endl;

				const auto exportsExtra = contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(s.name) + "::" + sessionCallbackExportTypeName(s.name) + " _exports";
				const auto acceptExtra = contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(s.name) + "::" + sessionAcceptSignatureTypeName(f->name) + " _accept";
				ss << indent(n + 1) << provideLine(symName, defName, f->args, n, exportsExtra + ", " + acceptExtra);
				ss << indent(n + 1) << "{" << std::endl;

				if(!f->returnType.has_value())
				{
					ss << indent(n + 2) << "auto _obj = " << invokeLine(defName, f->args, n) << ";" << std::endl;
					ss << indent(n + 2) << "_obj->importRemote(_exports);" << std::endl;
					ss << indent(n + 2) << "if(auto _err = _ep.call(_accept, _obj->exportLocal(_ep, _obj)))" << std::endl;
				}
				else
				{
					ss << indent(n + 2) << "auto _pair = " << invokeLine(defName, f->args, n) << ";" << std::endl;
					ss << indent(n + 2) << "auto _ret = std::move(_pair.first);" << std::endl;
					ss << indent(n + 2) << "auto _obj = std::move(_pair.second);" << std::endl;
					ss << indent(n + 2) << "_obj->importRemote(_exports);" << std::endl;
					ss << indent(n + 2) << "if(auto _err = _ep.call(_accept, std::move(_ret), _obj->exportLocal(_ep, _obj)))" << std::endl;
				}

				ss << indent(n + 2) << "{" << std::endl;
				ss << indent(n + 3) << "rpc::fail(std::string(\"Calling accept callback of session constructor '" << f->name << "' resulted in error \") + _err);" << std::endl;
				ss << indent(n + 2) << "}" << std::endl;
				ss << indent(n + 1) << "});" << std::endl;

				ss << std::endl;
				writeRegErrorCheck(ss, *f, n + 1);
				ss << indent(n) << "}";

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
				indent(2) + name + "::StlEndpoint(std::forward<Args>(args)...)";

		for(const auto& i: c.items) {
			std::visit([&blocks, &c](const auto i){handleItem(blocks, i, c.name, 2);}, i.second);
		}

		std::stringstream ss;
		writeBlock(ss, header, blocks, 1);
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

		const auto header = "template<class Child, class Adapter>\nstruct " + contractServerProxyNameRef(c.name) + ": rpc::StlEndpoint<Adapter>";

		std::vector<std::string> result;
		result.push_back(indent(1) + "using Endpoint = typename " + n + "::StlEndpoint::Endpoint;");
		result.push_back(ctor);
		writeTopLevelBlock(ss, header, result);
	}
}
