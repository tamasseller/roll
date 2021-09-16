#ifndef RPC_TOOL_GEN_CPP_CPPPROXYCOMMON_H_
#define RPC_TOOL_GEN_CPP_CPPPROXYCOMMON_H_

#include "CppCommon.h"

static inline std::string cppTypeRef(const Contract::Primitive& p, const std::string& cName) { return cppPrimitive(p); }

static inline std::string cppTypeRef(const std::string &n, const std::string& cName) {
	return contractTypeBlockNameRef(cName) + "::" + userTypeName(n);
}

static inline std::string cppTypeRef(const Contract::Collection &c, const std::string& cName) {
	return "rpc::CollectionPlaceholder<" + std::visit([&cName](const auto& e){ return cppTypeRef(e, cName); }, *c.elementType) + ">";
}

static inline std::string refTypeRef(const Contract::Primitive& p) { return Contract::mapPrimitive(p); }
static inline std::string refTypeRef(const std::string &n) { return n; }

static inline std::string refTypeRef(const Contract::Collection &c) {
	return "[" + std::visit([](const auto& e){ return refTypeRef(e); }, *c.elementType) + "]";
}

static inline std::string argCheck(const std::string& tName, const std::string& uName, const std::string &message, const int n)
{
	std::stringstream ss;
	ss << indent(n) << "static_assert(rpc::isCompatible<" << tName << ", " << uName << ">(), \"" << message << "\");" << std::endl;
	return ss.str();
}

#endif /* RPC_TOOL_GEN_CPP_CPPPROXYCOMMON_H_ */
