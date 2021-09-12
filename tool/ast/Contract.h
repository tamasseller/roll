#ifndef RPC_TOOL_AST_H_
#define RPC_TOOL_AST_H_

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <optional>

struct Contract
{
	struct Collection; 	//< Variably sized array (like std::vector).
	struct Aggregate; 	//< Structured data (like a struct)
	struct Var; 		//< A named slot for a value of a predetermined type.

	/// Single 1/2/4/8 byte signed/unsigned word (primitive integers).
	enum class Primitive
	{
		Bool, I1, U1, I2, U2, I4, U4, I8, U8
	};

	using TypeRef = std::variant<Primitive, Collection, std::string>;
	using TypeDef = std::variant<Primitive, Collection, Aggregate, std::string>;

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
	const std::string name, docs;

	inline bool operator==(const Contract& o) const {
		return items == o.items;
	}

	static inline std::string mapPrimitive(Contract::Primitive p)
	{
		switch(p)
		{
		case Contract::Primitive::Bool: return "bool";
		case Contract::Primitive::I1: return "i1";
		case Contract::Primitive::U1: return "u1";
		case Contract::Primitive::I2: return "i2";
		case Contract::Primitive::U2: return "u2";
		case Contract::Primitive::I4: return "i4";
		case Contract::Primitive::U4: return "u4";
		case Contract::Primitive::I8: return "i8";
		case Contract::Primitive::U8: return "u8";
		default: throw std::runtime_error("unknown primitive type: " + std::to_string((int)p));
		}
	}
};

#endif /* RPC_TOOL_AST_H_ */
