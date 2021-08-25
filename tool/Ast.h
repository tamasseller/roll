#ifndef RPC_TOOL_AST_H_
#define RPC_TOOL_AST_H_

#include "rpcParser.h"
#include "rpcLexer.h"

#include <vector>
#include <string>
#include <memory>
#include <variant>

struct Ast
{
	struct Primitive;  	//< Primitive integers (like char/short/int/long).
	struct Collection; 	//< Variably sized array (like std::vector).
	struct Aggreagete; 	//< Structured data (like a struct)
	struct Var; 		//< A named slot for a value of a predetermined type.

	using Type = std::pair<std::string, std::variant<Primitive, Collection, Aggreagete>>;

	struct SemanticParserState; // Internal state.

	/// Single 1/2/4/8 byte signed/unsigned word (primitive integers).
	struct Primitive
	{
		const bool isSigned;
		const int length;

		static constexpr inline int getLength(char c);
		Primitive(const std::string& str);
	};

	/// Zero or more elements of the same type (dynamic array).
	struct Aggreagete {
		const std::vector<Var> members;
	};

	/// A heterogeneous list of named elements with fixed order (structure).
	struct Collection {
		const std::shared_ptr<Type> elementType;
	};

	/// A name+type pair (like invocation arguments or aggregate members).
	struct Var
	{
		const std::string name;
		const Type type;

		Var(const SemanticParserState &sps, rpcParser::VarContext* ctx);
	};

	struct Call
	{
		const std::string name;
		const std::vector<Var> args;

		Call(const SemanticParserState &sps, rpcParser::SymbolContext* ctx);
	};

	struct Pull: Call
	{
		const Type returnType;
		Pull(const SemanticParserState &sps, rpcParser::GetterContext* ctx);
	};

	struct Alias: Type {};

	struct Session
	{
		struct ForwardCall: Call { using Call::Call; };
		struct CallBack: Call { using Call::Call; };

		using Item = std::variant<ForwardCall, CallBack>;

		const std::string name;
		const std::vector<Item> items;

		static inline auto parse(const SemanticParserState &sps, const std::vector<rpcParser::SessionItemContext*> &items);
		Session(const SemanticParserState &sps, rpcParser::SessionContext* ctx);
	};

	using Item = std::variant<Call, Pull, Alias, Session>;

	const std::vector<Item> items;

	static Ast from(rpcParser::RpcContext* ctx);
};

#endif /* RPC_TOOL_AST_H_ */
