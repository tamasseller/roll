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
			child()->writeIdentifier(i.name);
			child()->writeText(i.docs);
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
			child()->writeIdentifier(t.first);
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
		child()->writeIdentifier(f.name);
		retType(f.returnType);
		varList(f.args);
	}

	inline void processItem(const Ast::Alias& a)
	{
		child()->write(RootSelector::Type);
		child()->writeIdentifier(a.name);
		type(a.type, a.name);
	}

	template<auto start = RootSelector::Func>
	inline void processAction(const Ast::Action& a)
	{
		child()->write(start);
		child()->writeIdentifier(a.name);
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
		child()->writeIdentifier(sess.name);

		for(const auto& i: sess.items)
		{
			std::visit([this](const auto &i){return processSessionItem(i); }, i.second);
			child()->writeText(i.first);
		}

		child()->write(SessionItemSelector::None);
	}

public:
	void traverse(const Ast &ast)
	{
		for(const auto& i: ast.items)
		{
			std::visit([this](const auto &i){return processItem(i); }, i.second);
			child()->writeText(i.first);
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
			child()->readIdentifier(name);

			std::string docs;
			child()->readText(docs);

			ret.emplace_back(name, *t, docs);
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
				child()->readIdentifier(key);
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
		child()->readIdentifier(name);
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
		child()->readIdentifier(name);
		auto args = varList();
		return Ast::Action{name, args};
	}

	Ast::Alias alias()
	{
		std::string name;
		child()->readIdentifier(name);
		const auto t = *type(name);
		aliases.insert({name, t});
		return Ast::Alias{name, t};
	}

	template<class R>
	R readTheDocs(decltype(std::declval<R>().second) content)
	{
		std::string docs;
		child()->readText(docs);
		return {docs, content};
	}

	Ast::Session session()
	{
		std::string name;
		child()->readIdentifier(name);

		std::vector<Ast::Session::Item> items;

		while(true)
		{
			SessionItemSelector s;
			child()->read(s);

			switch(s)
			{
			case SessionItemSelector::ForwardCall:
				items.push_back(readTheDocs<Ast::Session::Item>(Ast::Session::ForwardCall(action())));
				break;
			case SessionItemSelector::CallBack:
				items.push_back(readTheDocs<Ast::Session::Item>(Ast::Session::CallBack(action())));
				break;
			case SessionItemSelector::Constructor:
				items.push_back(readTheDocs<Ast::Session::Item>(Ast::Session::Ctor(func())));
				break;
			default:
				return {name, std::move(items)};
			}
		}
	}

public:
	Ast build()
	{
		std::vector<Ast::Item> items;

		while(true)
		{
			RootSelector s;
			child()->read(s);

			switch(s)
			{
			case RootSelector::Func:
				items.push_back(readTheDocs<Ast::Item>(func()));
				break;
			case RootSelector::Type:
				items.push_back(readTheDocs<Ast::Item>(alias()));
				break;
			case RootSelector::Session:
				items.push_back(readTheDocs<Ast::Item>(session()));
				break;
			default:
				return {std::move(items)};
			}
		}
	}
};

#endif /* RPC_TOOL_ASTSERDES_H_ */
