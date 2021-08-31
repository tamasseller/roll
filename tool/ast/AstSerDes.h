#ifndef RPC_TOOL_ASTSERDES_H_
#define RPC_TOOL_ASTSERDES_H_

#include "Ast.h"

#include <map>
#include <sstream>

struct AstSerDes
{
	enum class RootSelector {
		Func, Type, Session, None
	};

	enum class TypeSelector {
		Primitive, Collection, Aggregate, Alias, None
	};

	enum class PrimitiveSelector {
		I1, U1, I2, U2, I4, U4, I8, U8
	};

	enum class SessionItemSelector {
		Constructor, ForwardCall, CallBack, None
	};
};

template<class Child>
class AstSerializer: public AstSerDes
{
	constexpr inline Child* child() {
		return static_cast<Child*>(this);
	}

	inline void varList(const std::vector<Ast::Var> &items)
	{
		for(const auto &i: items)
		{
			type(i.type);
			child()->write(i.name);
		}

		child()->write(TypeSelector::None);
	}

	inline void kind(const Ast::Aggregate& a)
	{
		child()->write(TypeSelector::Aggregate);
		varList(a.members);
	}

	inline void kind(const Ast::Collection& a)
	{
		child()->write(TypeSelector::Collection);
		type(*a.elementType);
	}

	inline void kind(const Ast::Primitive& a)
	{
		child()->write(TypeSelector::Primitive);

		if(a.isSigned)
		{
			if(a.length == 1)
				child()->write(PrimitiveSelector::I1);
			else if(a.length == 2)
				child()->write(PrimitiveSelector::I2);
			else if(a.length == 4)
				child()->write(PrimitiveSelector::I4);
			else
				child()->write(PrimitiveSelector::I8);
		}
		else
		{
			if(a.length == 1)
				child()->write(PrimitiveSelector::U1);
			else if(a.length == 2)
				child()->write(PrimitiveSelector::U2);
			else if(a.length == 4)
				child()->write(PrimitiveSelector::U4);
			else
				child()->write(PrimitiveSelector::U8);
		}
	}

	inline void type(const Ast::Type &t, std::string forbiddenName = {})
	{
		if(t.first != forbiddenName)
		{
			child()->write(TypeSelector::Alias);
			child()->write(t.first);
		}
		else
		{
			std::visit([this](const auto& t){kind(t);}, t.second);
		}
	}

	inline void retType(std::optional<Ast::Type> t)
	{
		if(t.has_value())
		{
			type(t.value());
		}
		else
		{
			child()->write(TypeSelector::None);
		}
	}

	template<auto start = RootSelector::Func>
	inline void processItem(const Ast::Function& f)
	{
		child()->write(start);
		child()->write(f.name);
		retType(f.returnType);
		varList(f.args);
	}

	inline void processItem(const Ast::Alias& a)
	{
		child()->write(RootSelector::Type);
		child()->write(a.name);
		type(a.type, a.name);
	}

	template<auto start = RootSelector::Func>
	inline void processAction(const Ast::Action& a)
	{
		child()->write(start);
		child()->write(a.name);
		varList(a.args);
	}

	inline void processSessionItem(const Ast::Session::ForwardCall& a) {
		processAction<SessionItemSelector::ForwardCall>((Ast::Action&)a);
	}

	inline void processSessionItem(const Ast::Session::CallBack& a) {
		processAction<SessionItemSelector::CallBack>((Ast::Action&)a);
	}

	inline void processSessionItem(const Ast::Session::Ctor& f) {
		processItem<SessionItemSelector::Constructor>((Ast::Function&)f);
	}

	inline void processItem(const Ast::Session& sess)
	{
		child()->write(RootSelector::Session);
		child()->write(sess.name);

		for(const auto& i: sess.items)
		{
			std::visit([this](const auto &i){return processSessionItem(i); }, i);
		}

		child()->write(SessionItemSelector::None);
	}

public:
	void traverse(const Ast &ast)
	{
		for(const auto& i: ast.items)
		{
			std::visit([this](const auto &i){return processItem(i); }, i);
		}

		child()->write(RootSelector::None);
	}
};

template<class Child>
class AstDeserializer: public AstSerDes
{
	constexpr inline Child* child() {
		return static_cast<Child*>(this);
	}

	std::map<std::string, Ast::Type> aliases;

	Ast::Primitive primitive()
	{
		PrimitiveSelector p;
		child()->read(p);

		switch(p)
		{
			case PrimitiveSelector::I1: return Ast::Primitive{true, 1};
			case PrimitiveSelector::U1: return Ast::Primitive{false, 1};
			case PrimitiveSelector::I2: return Ast::Primitive{true, 2};
			case PrimitiveSelector::U2: return Ast::Primitive{false, 2};
			case PrimitiveSelector::I4: return Ast::Primitive{true, 4};
			case PrimitiveSelector::U4: return Ast::Primitive{false, 4};
			case PrimitiveSelector::I8: return Ast::Primitive{true, 8};
			default: return Ast::Primitive{false, 8};
		}
	}

	Ast::Collection collection()
	{
		if(auto t = type())
		{
			return Ast::Collection{std::make_shared<Ast::Type>(*t)};
		}
		else
		{
			throw std::runtime_error("Invalid collection element type");
		}
	}

	std::vector<Ast::Var> varList()
	{
		std::vector<Ast::Var> ret;

		while(auto t = this->type())
		{
			std::string name;
			child()->read(name);
			ret.emplace_back(name, *t);
		}

		return ret;
	}

	Ast::Aggregate aggregate() {
		return Ast::Aggregate{varList()};
	}

	Ast::Type doType(TypeSelector kind, std::string name = "")
	{
		switch(kind)
		{
		case TypeSelector::Primitive:
			return {name, primitive()};
		case TypeSelector::Collection:
			return {name, collection()};
		case TypeSelector::Aggregate:
			return {name, aggregate()};
		default:
			{
				std::string key;
				child()->read(key);
				if(auto it = aliases.find(key); it != aliases.end())
				{
					return it->second;
				}
				else
				{
					throw std::runtime_error("unknown type: '" + key + "'");
				}
			}
		}
	}

	std::optional<Ast::Type> type(std::string name = "")
	{
		TypeSelector kind;
		child()->read(kind);
		return (kind == TypeSelector::None) ? std::nullopt : std::optional<Ast::Type>{doType(kind, name)};
	}

	Ast::Function func()
	{
		std::string name;
		child()->read(name);
		auto ret = type();
		auto args = varList();

		if(ret.has_value())
		{
			return Ast::Function({name, args}, ret.value());
		}
		else
		{
			return Ast::Function({name, args});
		}
	}

	Ast::Function action()
	{
		std::string name;
		child()->read(name);
		auto args = varList();
		return Ast::Action{name, args};
	}

	Ast::Alias alias()
	{
		std::string name;
		child()->read(name);
		const auto t = *type(name);
		aliases.insert({name, t});
		return Ast::Alias{name, t};
	}

	Ast::Session session()
	{
		std::string name;
		child()->read(name);

		std::vector<Ast::Session::Item> items;

		bool done = false;
		while(!done)
		{
			SessionItemSelector s;
			child()->read(s);

			switch(s)
			{
			case SessionItemSelector::ForwardCall:
				items.push_back(Ast::Session::ForwardCall(action()));
				break;
			case SessionItemSelector::CallBack:
				items.push_back(Ast::Session::CallBack(action()));
				break;
			case SessionItemSelector::Constructor:
				items.push_back(Ast::Session::Ctor(func()));
				break;
			default:
				done = true;
			}
		}

		return {name, std::move(items)};
	}

public:
	Ast build()
	{
		std::vector<Ast::Item> items;

		bool done = false;
		while(!done)
		{
			RootSelector s;
			child()->read(s);

			switch(s)
			{
			case RootSelector::Func:
				items.push_back(func());
				break;
			case RootSelector::Type:
				items.push_back(alias());
				break;
			case RootSelector::Session:
				items.push_back(session());
				break;
			default:
				done = true;
			}
		}

		return {std::move(items)};
	}
};

#endif /* RPC_TOOL_ASTSERDES_H_ */
