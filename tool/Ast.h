#ifndef RPC_TOOL_AST_H_
#define RPC_TOOL_AST_H_

#include <vector>
#include <string>
#include <memory>
#include <istream>
#include <variant>

struct Ast
{
	struct Primitive;  	//< Primitive integers (like char/short/int/long).
	struct Collection; 	//< Variably sized array (like std::vector).
	struct Aggreagete; 	//< Structured data (like a struct)
	struct Var; 		//< A named slot for a value of a predetermined type.

	using Type = std::pair<std::string, std::variant<Primitive, Collection, Aggreagete>>;

	/// Single 1/2/4/8 byte signed/unsigned word (primitive integers).
	struct Primitive
	{
		const bool isSigned;
		const int length;
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
	};

	struct Call
	{
		const std::string name;
		const std::vector<Var> args;
	};

	struct Pull: Call
	{
		const Type returnType;
		inline Pull(Call call, Type returnType): Call(call), returnType(returnType) {}
	};

	struct Alias: Type {};

	struct Session
	{
		struct ForwardCall: Call { inline ForwardCall(Call c): Call(c) {}; };
		struct CallBack: Call { inline CallBack(Call c): Call(c) {}; };

		using Item = std::variant<ForwardCall, CallBack>;

		const std::string name;
		const std::vector<Item> items;
	};

	using Item = std::variant<Call, Pull, Alias, Session>;
	const std::vector<Item> items;

	static Ast fromText(std::istream&);
};

#endif /* RPC_TOOL_AST_H_ */
