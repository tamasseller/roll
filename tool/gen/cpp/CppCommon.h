#ifndef RPC_TOOL_GEN_CPP_CPPCOMMON_H_
#define RPC_TOOL_GEN_CPP_CPPCOMMON_H_

#include <string>
#include <sstream>

#include <cassert>
#include <cctype>

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

static inline auto userTypeName(const std::string& n) {
	return capitalize(n);
}

static inline auto sessionTypeName(const std::string& n) {
	return capitalize(n);
}

static inline auto contractTypeName(const std::string& n) {
	return capitalize(n);
}

static inline auto aggregateMemberName(const std::string& n) {
	return decapitalize(n);
}

static inline auto argumentName(const std::string& n) {
	return decapitalize(n);
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

void printDocs(std::stringstream &ss, const std::string& str, const int n);

#endif /* RPC_TOOL_GEN_CPP_CPPCOMMON_H_ */
