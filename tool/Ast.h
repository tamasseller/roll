#ifndef RPC_TOOL_AST_H_
#define RPC_TOOL_AST_H_

#include <vector>
#include <string>
#include <memory>
#include <variant>

#include "rpcParser.h"
#include "rpcLexer.h"

struct Ast
{
	struct Primitive;  	//< Primitive integers (like char/short/int/long).
	struct Collection; 	//< Variably sized array (like std::vector).
	struct Aggreagete; 	//< Structured data (like a struct)
	struct Type; 		//< Any of a primitive, aggregate or collection.
	struct Var; 		//< A named slot for a value of a predetermined type.

	/// Single 1/2/4/8 byte signed/unsigned word (primitive integers).
	struct Primitive
	{
		const bool isSigned;
		const int length;

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

		Primitive(const std::string& str):
			isSigned(str[0] == 'i' || str[0] == 'I'),
			length(getLength(str[1])){}
	};

	/// Zero or more elements of the same type (dynamic array).
	struct Aggreagete {
		const std::vector<Var> members;
	};

	/// A heterogeneous list of named elements with fixed order (structure).
	struct Collection {
		const std::shared_ptr<Type> elementType;
	};

	/// Any of the above.
	struct Type: std::variant<std::monostate, Primitive, Collection, Aggreagete>
	{
		std::variant<std::monostate, Primitive, Collection, Aggreagete> classify(rpcParser::TypeContext* ctx)
		{
			if(auto data = ctx->p)
			{
				return Primitive(data->kind->getText());
			}
			else if(auto data = ctx->c)
			{
				return Collection{std::make_shared<Type>(data->elementType)};
			}
			else if(auto data = ctx->a)
			{
				return Aggreagete{{data->members->vars.begin(), data->members->vars.end()}};
			}

			return std::monostate{};
		}

		Type(rpcParser::TypeContext* ctx): variant(classify(ctx)) {}
	};

	/// A name+type pair (like invocation arguments or aggregate members).
	struct Var
	{
		const std::string name;
		const Type type;

		Var(rpcParser::VarContext* ctx): name(ctx->name->getText()), type(ctx->t) {}
	};

	struct Call
	{
		const std::string name;
		const std::vector<Var> args;

		Call(rpcParser::SymbolContext* ctx): name(ctx->name->getText()), args{ctx->args->vars.begin(), ctx->args->vars.end()} {}
	};

	struct Pull: Call
	{
		const Type returnType;
		Pull(rpcParser::GetterContext* ctx): Call(ctx->sym), returnType(ctx->ret) {}
	};

	const std::vector<Call> push;
	const std::vector<Pull> pull;

	static Ast from(rpcParser::RpcContext* ctx)
	{
		std::remove_const_t<decltype(push)> push;
		std::remove_const_t<decltype(pull)> pull;

		for(auto s: ctx->items)
		{
			if(s->push)
			{
				push.push_back({s->push});
			}
			else if(s->pull)
			{
				pull.push_back({s->pull});
			}
		}

		return {std::move(push), std::move(pull)};
	}
};

#endif /* RPC_TOOL_AST_H_ */
