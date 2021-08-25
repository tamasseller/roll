#include "Ast.h"

#include <map>
#include <algorithm>

struct Ast::SemanticParserState
{
	std::map<std::string, Type> aliases;

	template<class It> std::vector<Var> parseVarList(It begin, It end) const
	{
		std::vector<Var> ret;
		std::transform(begin, end, std::back_inserter(ret), [this](auto ctx) { return Var(*this, ctx); });
		return ret;
	}

	inline Type resolveType(rpcParser::TypeContext* ctx, std::string name = "") const
	{
		if(auto data = ctx->p)
		{
			return {name, Primitive(data->kind->getText())};
		}
		else if(auto data = ctx->c)
		{
			return {name, Collection{std::make_shared<Type>(resolveType(data->elementType))}};
		}
		else if(auto data = ctx->a)
		{
			return {name, Aggreagete{parseVarList(data->members->vars.begin(), data->members->vars.end())}};
		}
		else if(auto data = ctx->n)
		{
			const auto name = data->getText();
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

	inline Type addAlias(rpcParser::TypeAliasContext* ctx)
	{
		const auto name = ctx->name->getText();
		auto ret = resolveType(ctx->value, name);
		aliases.emplace(name, ret);
		return ret;
	}
};

constexpr inline int Ast::Primitive::getLength(char c)
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

Ast::Primitive::Primitive(const std::string& str):
	isSigned(str[0] == 'i' || str[0] == 'I'),
	length(getLength(str[1])){}


Ast::Var::Var(const SemanticParserState &sps, rpcParser::VarContext* ctx):
	name(ctx->name->getText()),
	type(sps.resolveType(ctx->t)) {}

Ast::Call::Call(const SemanticParserState &sps, rpcParser::SymbolContext* ctx): name(ctx->name->getText()), args(sps.parseVarList(ctx->args->vars.begin(), ctx->args->vars.end())) {}

Ast::Pull::Pull(const SemanticParserState &sps, rpcParser::GetterContext* ctx): Call(sps, ctx->sym), returnType(sps.resolveType(ctx->ret)) {}

inline auto Ast::Session::parse(const SemanticParserState &sps, const std::vector<rpcParser::SessionItemContext*> &items)
{
	std::vector<Item> ret;
	std::transform(items.begin(), items.end(), std::back_inserter(ret), [sps](const auto& i) -> Item
	{
		if(auto d = i->fwd)
		{
			return ForwardCall(sps, d->sym);
		}
		else if(auto d = i->bwd)
		{
			return CallBack(sps, d->sym);
		}

		throw std::runtime_error("Internal error: unknown session item kind");
	});
	return ret;
}

Ast::Session::Session(const SemanticParserState &sps, rpcParser::SessionContext* ctx):
	name(ctx->name->getText()),
	items(parse(sps, ctx->items)){}

Ast Ast::from(rpcParser::RpcContext* ctx)
{
	std::remove_const_t<decltype(items)> items;
	SemanticParserState sps;

	for(auto s: ctx->items)
	{
		if(auto d = s->push)
		{
			items.push_back(Call{sps, d});
		}
		else if(auto d = s->pull)
		{
			items.push_back(Pull{sps, d});
		}
		else if(auto d = s->alias)
		{
			items.push_back(Alias{sps.addAlias(d)});
		}
		else if(auto d = s->sess)
		{
			items.push_back(Session{sps, d});
		}
	}

	return {std::move(items)};
}

