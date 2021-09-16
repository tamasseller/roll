#ifndef RPC_TOOL_GEN_CPP_CPPSESSIONPROXY_H_
#define RPC_TOOL_GEN_CPP_CPPSESSIONPROXY_H_

#include "ast/Contract.h"

#include <sstream>

struct SessionProxyFilter;

struct SessionProxyFilterFactory
{
	virtual std::unique_ptr<SessionProxyFilter> make(const std::string& cName, const std::string& sName) const = 0;
	virtual ~SessionProxyFilterFactory() = default;
};

class ClientSessionProxyFilterFactory: public SessionProxyFilterFactory
{
	virtual std::unique_ptr<SessionProxyFilter> make(const std::string& cName, const std::string& sName) const override;
public:
	inline virtual ~ClientSessionProxyFilterFactory() = default;
};

class ServerSessionProxyFilterFactory: public SessionProxyFilterFactory
{
	virtual std::unique_ptr<SessionProxyFilter> make(const std::string& cName, const std::string& sName) const override;
public:
	inline virtual ~ServerSessionProxyFilterFactory() = default;
};

void writeSessionProxies(std::stringstream&, const Contract&, const SessionProxyFilterFactory&);

#endif /* RPC_TOOL_GEN_CPP_CPPSESSIONPROXY_H_ */
