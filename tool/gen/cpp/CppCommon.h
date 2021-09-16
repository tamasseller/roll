#ifndef RPC_TOOL_GEN_CPP_CPPCOMMON_H_
#define RPC_TOOL_GEN_CPP_CPPCOMMON_H_

#include "ast/Contract.h"

#include <string>
#include <vector>
#include <sstream>

#include <cassert>
#include <cctype>

namespace detail
{
	static constexpr auto indentStep = 4;
	static constexpr auto symPrefix = "sym";
	static constexpr auto callMemberPrefix = "id";
	static constexpr auto contractNsSuffix = "Contract";
	static constexpr auto typeNsSuffix = "Types";
	static constexpr auto symNsSuffix = "Symbols";
	static constexpr auto actSgnTypeSuffix = "Call";
	static constexpr auto funSgnTypeSuffix = "Function";
	static constexpr auto cbSgnTypeSuffix = "Callback";
	static constexpr auto sessNsSuffix = "Session";
	static constexpr auto sessFwdSgnTypeSuffix = "Call";
	static constexpr auto sessCbSgnTypeSuffix = "Callback";
	static constexpr auto sessCreateSgnTypeSuffix = "Create";
	static constexpr auto sessAcceptSgnTypeSuffix = "Accept";
	static constexpr auto sessFwdExportTypeSuffix = "CallExports";
	static constexpr auto sessBwdExportTypeSuffix = "CallbackExports";
	static constexpr auto clientProxySuffix = "ClientProxy";
	static constexpr auto serverProxySuffix = "ServerProxy";
	static constexpr auto clientSessionSuffix = "ClientSession";
	static constexpr auto serverSessionSuffix = "ServerSession";

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
}

inline std::string indent(const int n) {
	return std::string(n * detail::indentStep, ' ');
}

bool writeBlock(std::stringstream& ss, const std::string &header, const std::vector<std::string>& strs, const int n);

void writeTopLevelBlock(std::stringstream& ss, const std::string &header, const std::vector<std::string>& strs, bool addSemi = true);

static inline std::string cppPrimitive(Contract::Primitive p)
{
	switch(p)
	{
	case Contract::Primitive::Bool: return "bool";
	case Contract::Primitive::I1: return "int8_t";
	case Contract::Primitive::U1: return "uint8_t";
	case Contract::Primitive::I2: return "int16_t";
	case Contract::Primitive::U2: return "uint16_t";
	case Contract::Primitive::I4: return "int32_t";
	case Contract::Primitive::U4: return "uint32_t";
	case Contract::Primitive::I8: return "int64_t";
	case Contract::Primitive::U8: return "uint64_t";
	default: throw std::runtime_error("unknown primitive type: " + std::to_string((int)p));
	}
}

static inline auto contractRootBlockName(const std::string& contractName) {
	return detail::capitalize(contractName) + detail::contractNsSuffix;
}

static inline auto contractTypeBlockNameDef(const std::string& contractName) {
	return detail::typeNsSuffix;
}

static inline auto contractTypeBlockNameRef(const std::string& contractName) {
	return contractRootBlockName(contractName) + "::" + contractTypeBlockNameDef(contractName);
}

static inline auto contractSymbolsBlockNameDef(const std::string& contractName) {
	return detail::symNsSuffix;
}

static inline auto contractSymbolsBlockNameRef(const std::string& contractName) {
	return contractRootBlockName(contractName) + "::" + contractSymbolsBlockNameDef(contractName);
}

static inline auto contractClientProxyNameDef(const std::string& contractName) {
	return detail::clientProxySuffix;
}

static inline auto contractClientProxyNameRef(const std::string& contractName) {
	return contractRootBlockName(contractName) + "::" + contractClientProxyNameDef(contractName);
}

static inline auto contractServerProxyNameDef(const std::string& contractName) {
	return detail::serverProxySuffix;
}

static inline auto contractServerProxyNameRef(const std::string& contractName) {
	return contractRootBlockName(contractName) + "::" + contractServerProxyNameDef(contractName);
}

static inline auto clientSessionName(const std::string& contractName, const std::string& sessionName) {
	return detail::capitalize(contractName) + detail::capitalize(sessionName) + detail::clientSessionSuffix;
}

static inline auto serverSessionName(const std::string& contractName, const std::string& sessionName) {
	return detail::capitalize(contractName) + detail::capitalize(sessionName) + detail::serverSessionSuffix;
}

static inline auto aggregateMemberName(const std::string& n) {
	return detail::decapitalize(n);
}

static inline auto argumentName(const std::string& n) {
	return detail::decapitalize(n);
}

static inline auto invocationMemberFunctionName(const std::string& n) {
	return detail::decapitalize(n);
}

static inline auto definitionMemberFunctionName(const std::string& n) {
	return detail::decapitalize(n);
}

static inline auto userTypeName(const std::string& n) {
	return detail::capitalize(n);
}

static inline auto symbolName(const std::string& n) {
	return detail::symPrefix + detail::capitalize(n);
}

static inline auto callMemberName(const std::string& n) {
	return detail::callMemberPrefix + detail::capitalize(n);
}

static inline auto actionSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::actSgnTypeSuffix;
}

static inline auto functionSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::funSgnTypeSuffix;
}

static inline auto callbackSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::cbSgnTypeSuffix;
}

static inline auto sessionNamespaceName(const std::string& n) {
	return detail::capitalize(n) + detail::sessNsSuffix;
}

static inline auto sessionCtorApiName(const std::string& s, const std::string& c) {
	return detail::decapitalize(s) + detail::capitalize(c);
}

static inline auto sessionForwardCallSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::sessFwdSgnTypeSuffix;
}

static inline auto sessionCallbackSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::sessCbSgnTypeSuffix;
}

static inline auto sessionCreateSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::sessCreateSgnTypeSuffix;
}

static inline auto sessionAcceptSignatureTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::sessAcceptSgnTypeSuffix;
}

static inline auto sessionCallExportTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::sessFwdExportTypeSuffix;
}

static inline auto sessionCallbackExportTypeName(const std::string& n) {
	return detail::capitalize(n) + detail::sessBwdExportTypeSuffix;
}

std::string printDocs(const std::string& str, const int n);

#endif /* RPC_TOOL_GEN_CPP_CPPCOMMON_H_ */
