#include "CppSessionProxy.h"

#include "CppCommon.h"
#include "CppProxyCommon.h"

#include <algorithm>

struct SessionProxyFilter
{
	const std::string cName, sName;

	SessionProxyFilter(const std::string& cName, const std::string& sName): cName(cName), sName(sName) {}
	virtual const Contract::Action* asImport(const Contract::Session::Item&) const = 0;
	virtual const Contract::Action* asExport(const Contract::Session::Item&) const = 0;
	virtual std::string friendName() const = 0;
	virtual std::string typeName() const = 0;
	virtual std::string importedName() const = 0;
	virtual std::string exportedName() const = 0;
	virtual ~SessionProxyFilter() = default;
};

std::unique_ptr<SessionProxyFilter> ClientSessionProxyFilterFactory::make(const std::string& cName, const std::string& sName) const
{
	struct Ret: SessionProxyFilter
	{
		using SessionProxyFilter::SessionProxyFilter;
		virtual std::string friendName() const override { return "rpc::ClientBase"; };
		virtual std::string typeName() const override { return clientSessionName(cName, sName); }
		virtual std::string importedName() const override { return contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(sName) + "::" + sessionCallExportTypeName(sName); }
		virtual std::string exportedName() const override { return contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(sName) + "::" + sessionCallbackExportTypeName(sName); }
		virtual const Contract::Action* asImport(const Contract::Session::Item& item) const override { return std::get_if<Contract::Session::ForwardCall>(&item.second);}
		virtual const Contract::Action* asExport(const Contract::Session::Item& item) const override { return std::get_if<Contract::Session::CallBack>(&item.second);}
	};

	return std::make_unique<Ret>(cName, sName);
}

std::unique_ptr<SessionProxyFilter> ServerSessionProxyFilterFactory::make(const std::string& cName, const std::string& sName) const
{
	struct Ret: SessionProxyFilter
	{
		using SessionProxyFilter::SessionProxyFilter;
		virtual std::string friendName() const override { return "rpc::ServiceBase"; };
		virtual std::string typeName() const override { return serverSessionName(cName, sName); }
		virtual std::string importedName() const override { return contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(sName) + "::" + sessionCallbackExportTypeName(sName); }
		virtual std::string exportedName() const override { return contractTypeBlockNameRef(cName) + "::" + sessionNamespaceName(sName) + "::" + sessionCallExportTypeName(sName); }
		virtual const Contract::Action* asImport(const Contract::Session::Item& item) const override { return std::get_if<Contract::Session::CallBack>(&item.second);}
		virtual const Contract::Action* asExport(const Contract::Session::Item& item) const override { return std::get_if<Contract::Session::ForwardCall>(&item.second);}
	};

	return std::make_unique<Ret>(cName, sName);
}

std::string generateExportLocalMethod(const SessionProxyFilter& nGen, const Contract::Session& s, const int n)
{
	std::stringstream ss;

	ss << indent(n) << "template<class Ep, class Self>" << std::endl;
	ss << indent(n) << "inline auto exportLocal(Ep& _ep, Self _self)" << std::endl;
	ss << indent(n) << "{" << std::endl;

	std::string sep;
	for(const Contract::Session::Item& item: s.items)
	{
		if(const Contract::Action* a = nGen.asExport(item))
		{
			const auto defName = definitionMemberFunctionName(a->name);
			const auto count = a->args.size();

			ss << indent(n + 1) << "static_assert(rpc::nArgs<&Child::" << defName << "> == " << count << ", "
				<< "\"Public method " << a->name << " must take " << count << " argument"
				<< ((count > 1) ? "s" : "") << "\");" << std::endl;

			for(auto i = 0u; i < a->args.size(); i++)
			{
				const auto cppTypeName = std::visit([&nGen](const auto &i){return cppTypeRef(i, nGen.cName);}, a->args[i].type);
				const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, a->args[i].type);

				const auto msg = "Argument #" + std::to_string(i + 1) + " of " + defName
						+ " must have type compatible with '" + refTypeName + "'";

				ss << argCheck("rpc::Arg<" + std::to_string(i) + ", &Child::" + defName + ">", cppTypeName, msg , n + 1);
			}

			ss << indent(n + 1) << "exportCall<&" << nGen.exportedName() << "::" << defName << ", &Child::" << defName << ", Ep, Self";

			for(auto i = 0u; i < a->args.size(); i++)
			{
				ss << ", rpc::Arg<" + std::to_string(i) + ", &Child::" + defName + ">";
			}

			ss << ">(_ep, _self);" << std::endl;

			ss << sep;
			sep = "\n";
		}
	}

	ss << indent(n + 1) << "return finalizeExports<&Child::onClosed>(_ep, _self);" << std::endl;

	ss << indent(n) << "}";
	return ss.str();
}

std::string generateImportProxyMethod(const SessionProxyFilter& nGen, const Contract::Action& a, const int n)
{
	std::stringstream ss;

	ss << indent(n) << "template<class Ep";

	for(auto i = 0u; i < a.args.size(); i++)
	{
		ss << ", class A" + std::to_string(i);
	}

	ss << ">" << std::endl;
	const auto defName = definitionMemberFunctionName(a.name);

	ss << indent(n) << "inline auto " << defName << "(Ep& _ep";

	for(auto i = 0u; i < a.args.size(); i++)
	{
		ss << ", A" << std::to_string(i) << "&& " << argumentName(a.args[i].name);
	}

	ss << ")" << std::endl;
	ss << indent(n) << "{" << std::endl;

	for(auto i = 0u; i < a.args.size(); i++)
	{
		const auto cppTypeName = std::visit([&nGen](const auto &i){return cppTypeRef(i, nGen.cName);}, a.args[i].type);
		const auto refTypeName = std::visit([](const auto &i){return refTypeRef(i);}, a.args[i].type);
		const auto msg = "Argument #" + std::to_string(i + 1) + " to " + defName + " must have type compatible with '" + refTypeName + "'";
		ss << argCheck("A" + std::to_string(i), cppTypeName, msg , n + 1);
	}

	ss << indent(n + 1) << "return this->callImported<&" << nGen.importedName() << "::" << defName << ">(_ep";

	for(auto i = 0u; i < a.args.size(); i++)
	{
		ss << ", std::forward<A" << std::to_string(i) << ">(" << argumentName(a.args[i].name) << ")";
	}

	ss << ");" << std::endl;

	ss << indent(n) << "}";
	return ss.str();
}

void writeSessionProxies(std::stringstream& ss, const Contract& c, const SessionProxyFilterFactory& f)
{
	for(const auto& i: c.items)
	{
		if(const Contract::Session* s = std::get_if<Contract::Session>(&i.second))
		{
			auto nGen = f.make(c.name, s->name);
			std::vector<std::string> result;

			result.push_back(indent(1) + "template<class> friend class " + nGen->friendName() + ";\n");
			result.push_back(generateExportLocalMethod(*nGen, *s, 1));
			result.push_back("public:");

			for(const Contract::Session::Item& item: s->items)
			{
				if(const Contract::Action* a = nGen->asImport(item))
				{
					result.push_back(generateImportProxyMethod(*nGen, *a, 1));
				}
			}

			ss << printDocs(i.first, 0);

			std::stringstream hs;
			hs << "template<class Child>" << std::endl;

			const auto exportCount = std::count_if(s->items.begin(), s->items.end(), [&nGen](const auto& i) {return nGen->asExport(i) != nullptr; });

			hs << "class " << nGen->typeName() << ": public SessionBase<" << nGen->importedName() << ", " << nGen->exportedName() << ", " << exportCount << ">";
			writeTopLevelBlock(ss, hs.str(), result, true);
		}
	}
}
