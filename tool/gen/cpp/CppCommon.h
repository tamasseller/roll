#ifndef RPC_TOOL_GEN_CPP_CPPCOMMON_H_
#define RPC_TOOL_GEN_CPP_CPPCOMMON_H_

#include <string>
#include <sstream>

#include <cassert>
#include <cctype>

namespace detail
{
	static constexpr auto indentStep = 4;
	static constexpr auto symPrefix = "sym";
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

static inline auto aggregateMemberName(const std::string& n) {
	return detail::decapitalize(n);
}

static inline auto argumentName(const std::string& n) {
	return detail::decapitalize(n);
}

static inline auto contractTypesNamespaceName(const std::string& n) {
	return detail::capitalize(n) + detail::typeNsSuffix;
}

static inline auto contractSymbolsNamespaceName(const std::string& n) {
	return detail::capitalize(n) + detail::symNsSuffix;
}

static inline auto userTypeName(const std::string& n) {
	return detail::capitalize(n);
}

static inline auto symbolName(const std::string& n) {
	return detail::symPrefix + detail::capitalize(n);
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

void printDocs(std::stringstream &ss, const std::string& str, const int n);

#endif /* RPC_TOOL_GEN_CPP_CPPCOMMON_H_ */
