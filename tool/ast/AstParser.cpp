#include "AstParser.h"
#include "AstRansSerDesCodec.h"

#include "rpcParser.h"
#include "rpcLexer.h"

#include <map>
#include <algorithm>
#include <cassert>

struct SemanticParser
{
	std::map<std::string, Ast::Type> aliases;

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

	static inline Ast::Primitive makePrimitive(const std::string& str) {
		return {str[0] == 'i' || str[0] == 'I', getLength(str[1])};
	}


	inline std::string makeDocs(antlr4::Token *t) const
	{
		if(t)
		{
			std::string ws = " \t\r\n";
			const std::string full = t->getText();
			assert(full.length() >= 4);

			size_t first = 0, last = 0;
			for(size_t idx = 2; idx < full.length() - 2; idx++)
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
		}

		return {};
	}

	inline Ast::Var makeVar(rpcParser::VarContext* ctx) const {
		return { ctx->name->getText(), resolveType(ctx->t), makeDocs(ctx->docs)};
	}

	template<class It> std::vector<Ast::Var> parseVarList(It begin, It end) const
	{
		std::vector<Ast::Var> ret;
		std::transform(begin, end, std::back_inserter(ret), [this](auto ctx) { return makeVar(ctx); });
		return ret;
	}

	inline Ast::Action makeCall(rpcParser::ActionContext* ctx) const {
		return {ctx->name->getText(), parseVarList(ctx->args->vars.begin(), ctx->args->vars.end())};
	}

	inline Ast::Function makeFunc(rpcParser::FunctionContext* ctx) const
	{
		if(ctx->ret)
		{
			return Ast::Function(makeCall(ctx->call), resolveType(ctx->ret));
		}
		else
		{
			return Ast::Function(makeCall(ctx->call));
		}
	}

	inline Ast::Session makeSession(rpcParser::SessionContext* ctx) const {
		return Ast::Session{ctx->name->getText(), parseSession(ctx->items)};
	}

	inline Ast::Type resolveType(rpcParser::TypeContext* ctx, std::string name = "") const
	{
		if(auto data = ctx->p)
		{
			return {name, makePrimitive(data->kind->getText())};
		}
		else if(auto data = ctx->c)
		{
			return {name, Ast::Collection{std::make_shared<Ast::Type>(resolveType(data->elementType))}};
		}
		else if(auto data = ctx->a)
		{
			return {name, Ast::Aggregate{parseVarList(data->members->vars.begin(), data->members->vars.end())}};
		}
		else
		{
			const auto name = ctx->n->getText();

			if(auto it = aliases.find(name); it != aliases.end())
			{
				return it->second;
			}
			else
			{
				throw std::runtime_error("No such type alias defined: " + name);
			}
		}

		throw std::runtime_error("Internal error: unknown type kind");
	}

	inline Ast::Type addAlias(rpcParser::TypeAliasContext* ctx)
	{
		const auto name = ctx->name->getText();
		auto ret = resolveType(ctx->value, name);
		aliases.emplace(name, ret);
		return ret;
	}

	inline std::vector<Ast::Session::Item> parseSession(std::vector<rpcParser::SessionItemContext *> items) const
	{
		std::vector<Ast::Session::Item> ret;
		std::transform(items.begin(), items.end(), std::back_inserter(ret), [this](const auto& i) -> Ast::Session::Item
		{
			if(auto d = i->fwd)
			{
				return {makeDocs(i->docs), Ast::Session::ForwardCall(makeCall(d->sym))};
			}
			else if(auto d = i->bwd)
			{
				return {makeDocs(i->docs), Ast::Session::CallBack(makeCall(d->sym))};
			}
			else if(auto d = i->ctr)
			{
				return {makeDocs(i->docs), Ast::Session::CallBack(makeFunc(d))};
			}

			throw std::runtime_error("Internal error: unknown session item kind");
		});

		return ret;
	}

	inline Ast::Item processItem(rpcParser::ItemContext* s)
	{
		if(auto d = s->func)
		{
			return {makeDocs(s->docs), makeFunc(d)};
		}
		else if(auto d = s->alias)
		{
			return {makeDocs(s->docs), Ast::Alias{d->name->getText(), addAlias(d)}};
		}
		else
		{
			return {makeDocs(s->docs), makeSession(s->sess)};
		}
	}

public:
	static inline Ast parse(rpcParser::RpcContext* ctx)
	{
		SemanticParser sps;
		std::vector<Ast::Item> items;
		std::transform(ctx->items.begin(), ctx->items.end(), std::back_inserter(items), [&sps](auto s){return sps.processItem(s); });
		return {std::move(items)};
	}
};

Ast parse(std::istream& is)
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
