#ifndef RPC_TOOL_AST_H_
#define RPC_TOOL_AST_H_

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <optional>

struct Ast
{
	struct Primitive;  	//< Primitive integers (like char/short/int/long).
	struct Collection; 	//< Variably sized array (like std::vector).
	struct Aggregate; 	//< Structured data (like a struct)
	struct Var; 		//< A named slot for a value of a predetermined type.

	using TypeRef = std::variant<Primitive, Collection, std::string>;
	using TypeDef = std::variant<Primitive, Collection, Aggregate, std::string>;

	/// Single 1/2/4/8 byte signed/unsigned word (primitive integers).
	struct Primitive
	{
		const bool isSigned;
		const int length;

		inline bool operator==(const Primitive& o) const {
			return isSigned == o.isSigned && length == o.length;
		}
	};

	/// Zero or more elements of the same type (dynamic array).
	struct Aggregate
	{
		const std::vector<Var> members;

		inline bool operator==(const Aggregate& o) const {
			return members == o.members;
		}
	};

	/// A heterogeneous list of named elements with fixed order (structure).
	struct Collection
	{
		const std::shared_ptr<TypeRef> elementType;

		inline bool operator==(const Collection& o) const {
			return *elementType == *o.elementType;
		}
	};

	/// A name+type pair (like invocation arguments or aggregate members).
	struct Var
	{
		const std::string name;
		const TypeRef type;
		const std::string docs;

		inline Var(const std::string &name, TypeRef type, std::string docs): name(name), type(type), docs(docs) {}

		inline bool operator==(const Var& o) const {
			return name == o.name && type == o.type;
		}
	};

	struct Action
	{
		const std::string name;
		const std::vector<Var> args;

		inline bool operator==(const Action& o) const {
			return name == o.name && args == o.args;
		}
	};

	struct Function: Action
	{
		const std::optional<TypeRef> returnType;
		inline Function(Action call, std::optional<TypeRef> returnType): Action(call), returnType(returnType) {}

		inline bool operator==(const Function& o) const {
			return *((Action*)this) == (const Action&)o;
		}
	};

	struct Alias
	{
		const TypeDef type;
		const std::string name;
		inline Alias(const std::string &name, TypeDef type): type(type), name(name) {}

		inline bool operator==(const Alias& o) const {
			return type == o.type && name == o.name;
		}
	};

	struct Session
	{
		struct ForwardCall: Action { inline ForwardCall(Action c): Action(c) {} };
		struct CallBack: Action { inline CallBack(Action c): Action(c) {} };
		struct Ctor: Function { inline Ctor(Function f): Function(f) {} };

		using Item = std::pair<std::string, std::variant<ForwardCall, CallBack, Ctor>>;

		const std::string name;
		const std::vector<Item> items;

		inline bool operator==(const Session& o) const {
			return name == o.name && items == o.items;
		}
	};

	using Item = std::pair<std::string, std::variant<Function, Alias, Session>>;
	const std::vector<Item> items;

	inline bool operator==(const Ast& o) const {
		return items == o.items;
	}
};

#endif /* RPC_TOOL_AST_H_ */
