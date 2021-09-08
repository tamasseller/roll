#include "ContractParser.h"
#include "ContractRansSerDesCodec.h"

#include "Taboo.h"

#include "rpcParser.h"
#include "rpcLexer.h"

#include <map>
#include <algorithm>
#include <cassert>

static inline std::string validateName(const std::string& str)
{
	if(forbiddenNames.find(str) != forbiddenNames.end())
	{
		throw std::runtime_error("Name '" + str + "' is forbidden");
	}

	return str;
}

struct SemanticParser
{
	std::map<std::string, Contract::TypeDef> aliases;

	static constexpr inline int getLength(char c)
	{
		switch(c)
		{
		case '1': return 1;
		case '2': return 2;
		case '4': return 4;
		case '8': return 8;
		}
		return uint8_t(-1u);
	}

	static inline Contract::Primitive makePrimitive(const std::string& str) {
		return {str[0] == 'i' || str[0] == 'I', getLength(str[1])};
	}

	static inline std::string trim(std::string full, size_t initialOffset, size_t finalOffset)
	{
		std::string ws = " \t\r\n";
		assert(full.length() >= initialOffset + finalOffset);

		size_t first = 0, last = 0;
		for(size_t idx = initialOffset; idx < full.length() - finalOffset; idx++)
		{
			if(ws.find(full[idx]) == std::string::npos)
			{
				if(!first)
					first = idx;

				last = idx;
			}
		}

		if(first)
		{
			return full.substr(first, last - first + 1);
		}

		return {};
	}

	static inline std::string makeDocs(antlr4::Token *t)
	{
		if(t)
		{
			std::stringstream in(t->getText());
			std::vector<std::string> lines;
			int n = 0;

			for(std::string to; std::getline(in, to, '\n');)
			{
				lines.push_back(std::move(to));
				n++;
			}

			std::stringstream out;
			for(int i = 0; i < n; i++)
			{
				out << trim(lines[i], i == 0 ? 2 : 0, i == (n-1) ? 2 : 0);

				if(i != (n-1))
				{
					out << std::endl;
				}
			}

			return out.str();
		}

		return {};
	}

	inline Contract::Var makeVar(rpcParser::VarContext* ctx) const {
		return { validateName(ctx->name->getText()), resolveTypeRef(ctx->t), makeDocs(ctx->docs)};
	}

	template<class It> std::vector<Contract::Var> parseVarList(It begin, It end) const
	{
		std::vector<Contract::Var> ret;
		std::transform(begin, end, std::back_inserter(ret), [this](auto ctx) { return makeVar(ctx); });
		return ret;
	}

	inline Contract::Action makeCall(rpcParser::ActionContext* ctx) const {
		return {validateName(ctx->name->getText()), parseVarList(ctx->args->vars.begin(), ctx->args->vars.end())};
	}

	inline Contract::Function makeFunc(rpcParser::FunctionContext* ctx) const
	{
		if(ctx->ret)
		{
			return Contract::Function(makeCall(ctx->call), resolveTypeRef(ctx->ret));
		}
		else
		{
			return Contract::Function(makeCall(ctx->call), {});
		}
	}

	inline std::vector<Contract::Session::Item> parseSession(std::vector<rpcParser::SessionItemContext *> items) const
	{
		std::vector<Contract::Session::Item> ret;
		std::transform(items.begin(), items.end(), std::back_inserter(ret), [this](const auto& i) -> Contract::Session::Item
		{
			if(auto d = i->fwd)
			{
				return {makeDocs(i->docs), Contract::Session::ForwardCall(makeCall(d->sym))};
			}
			else if(auto d = i->bwd)
			{
				return {makeDocs(i->docs), Contract::Session::CallBack(makeCall(d->sym))};
			}
			else if(auto d = i->ctr)
			{
				return {makeDocs(i->docs), Contract::Session::Ctor(makeFunc(d))};
			}

			throw std::runtime_error("Internal error: unknown session item kind");
		});

		return ret;
	}

	inline Contract::Session makeSession(rpcParser::SessionContext* ctx) const {
		return Contract::Session{validateName(ctx->name->getText()), parseSession(ctx->items)};
	}

	inline std::string checkAlias(const std::string& name) const
	{
		if(auto it = aliases.find(name); it == aliases.end())
		{
			throw std::runtime_error("No such type alias defined: " + name);
		}

		return name;
	}

	inline Contract::TypeRef resolveTypeRef(rpcParser::TyperefContext* ctx) const
	{
		if(auto data = ctx->p)
		{
			return makePrimitive(data->kind->getText());
		}
		else if(auto data = ctx->c)
		{
			return Contract::Collection{std::make_shared<Contract::TypeRef>(resolveTypeRef(data->elementType))};
		}
		else if(auto data = ctx->n)
		{
			return checkAlias(data->getText());
		}

		throw std::runtime_error("Internal error: unknown type kind in reference");
	}

	inline Contract::TypeDef addAlias(const std::string &name, Contract::TypeDef ret)
	{
		aliases.emplace(name, ret);
		return ret;
	}

	inline Contract::TypeDef resolveTypeDef(rpcParser::TypeAliasContext* ctx)
	{
		const auto name = validateName(ctx->name->getText());

		if(auto data = ctx->p)
		{
			return addAlias(name, makePrimitive(data->kind->getText()));
		}
		else if(auto data = ctx->c)
		{
			return addAlias(name, Contract::Collection{std::make_shared<Contract::TypeRef>(resolveTypeRef(data->elementType))});
		}
		else if(auto data = ctx->n)
		{
			return addAlias(name, checkAlias(data->getText()));
		}
		else if(auto data = ctx->a)
		{
			return addAlias(name, Contract::Aggregate{parseVarList(data->members->vars.begin(), data->members->vars.end())});
		}

		throw std::runtime_error("Internal error: unknown type kind in definition");
	}

	inline Contract::Item processItem(rpcParser::ItemContext* s)
	{
		if(auto d = s->func)
		{
			return {makeDocs(s->docs), makeFunc(d)};
		}
		else if(auto d = s->alias)
		{
			return {makeDocs(s->docs), Contract::Alias{validateName(d->name->getText()), resolveTypeDef(d)}};
		}
		else if(auto d = s->sess)
		{
			return {makeDocs(s->docs), makeSession(d)};
		}

		throw std::runtime_error("Unknown contract item");
	}

public:
	static inline std::vector<Contract> parse(rpcParser::RpcContext* ctx)
	{
		std::vector<Contract> ret;

		auto it = ctx->items.begin();

		while(it != ctx->items.end())
		{
			if(!(*it)->cont)
			{
				throw std::runtime_error("No active contract definition");
			}

			auto contract = (*it++);
			auto docs = makeDocs(contract->docs);
			auto name = validateName(contract->cont->name->getText());

			SemanticParser sps;
			std::vector<Contract::Item> items;

			while(it != ctx->items.end() && !(*it)->cont)
			{
				items.push_back(sps.processItem(*it++));
			}

			ret.push_back({std::move(items), name, docs});
		}

		return ret;
	}
};

std::vector<Contract> parse(std::istream& is)
{
	if(isprint(is.peek()))
	{
		antlr4::ANTLRInputStream input(is);
		rpcLexer lexer(&input);
		antlr4::ConsoleErrorListener errorListener;
		lexer.addErrorListener(&errorListener);
		antlr4::CommonTokenStream tokens(&lexer);
		rpcParser parser(&tokens);
		parser.addErrorListener(&errorListener);
		return SemanticParser::parse(parser.rpc());
	}
	else
	{
		return deserialize(is);
	}
}
