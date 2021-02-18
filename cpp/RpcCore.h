#ifndef COMMON_RPCCORE_H_
#define COMMON_RPCCORE_H_

#include "meta/Tuple.h"
#include "ubiquitous/Delegate.h"

#include <unordered_map>
#include <memory>
#include <assert.h>
#include <string.h>

template<class Serializer, class Deserializer>
class RpcCore: Serializer, Deserializer
{
	class CallItemManipulatorBase
	{
	protected:
		char* p, * const end;
		RpcCore& core;

		inline CallItemManipulatorBase(RpcCore& core, char* start, char *end): p(start), end(end), core(core) {}
	};

	template<class Io>
	class FieldIterator
	{
		inline void processField() {}

		template<class FirstArg, class... RestOfArgs>
		inline void processField(FirstArg&& first, RestOfArgs&&... rest)
		{
			static_cast<Io*>(this)->Io::workField(std::forward<FirstArg>(first));
			processField(rest...);
		}

	protected:
		template<class... Types>
		void processTuple(pet::Tuple<Types...> &&t)
		{
			pet::move(t).moveApply([this](Types&&... t){
				processField(pet::move(t)...);
			});
		}
	};

	class Marshaller: CallItemManipulatorBase, public FieldIterator<Marshaller>
	{
		friend FieldIterator<Marshaller>;

		template<class T>
		inline void workField(T&& v) {
			this->core.Serializer::serialize(this->p, this->end, std::move(v));
		}

	public:
		inline Marshaller(RpcCore& core, char* start, char *end): CallItemManipulatorBase(core, start, end) {}
		using Marshaller::FieldIterator::processTuple;
	};

	class Unmarshaller: CallItemManipulatorBase, public FieldIterator<Unmarshaller>
	{
		friend FieldIterator<Unmarshaller>;

		template<class T>
		inline void workField(T&& v) {
			this->core.Deserializer::deserialize(this->p, this->end, std::move(v));
		}

	public:
		inline Unmarshaller(RpcCore& core, char* start, char *end): CallItemManipulatorBase(core, start, end) {}
		using Unmarshaller::FieldIterator::processTuple;
	};

	struct IInvoker {
		virtual void invoke(RpcCore &core, char* start, char *end) = 0;
		inline virtual ~IInvoker() = default;
	};

	template<class... Args>
	struct Invoker: IInvoker
	{
		pet::Delegate<void(Args...)> target;
		Invoker(pet::Delegate<void(Args...)> &&target): target(std::move(target)) {}

		virtual void invoke(RpcCore &core, char* start, char *end) override
		{
			pet::Tuple<Args...> args;
			Unmarshaller(core, start, end).processTuple(std::move(args));
			std::move(args).moveApply(target);
		}

		inline virtual ~Invoker() = default;
	};

	std::unordered_map<uint32_t, std::unique_ptr<IInvoker>> lookupTable;
	int maxId = 0;

public:
	bool execute(CallItem* ci)
	{
		int id = *((int*)ci->data);

		auto it = lookupTable.find(id);

		if(it != lookupTable.end())
		{
			it->second->invoke(*this, ci->data + sizeof(int), ci->data + sizeof(ci->data));
			return true;
		}

		return false;
	}

	template<class... Args>
	uint32_t registerCall(pet::Delegate<void(Args...)>&& call)
	{
		int id;
		do
		{
			id = maxId++;
		}
		while(lookupTable.find(id) != lookupTable.end());

		auto callRegistered = lookupTable.insert(std::make_pair(id, std::make_unique<Invoker<Args...>>(std::move(call)))).second;
		assert(callRegistered);
		return id;
	}

	template<class... Args>
	bool registerCall(uint32_t id, pet::Delegate<void(Args...)>&& call)
	{
		if(lookupTable.find(id) != lookupTable.end())
				return false;

		auto callRegistered = lookupTable.insert(std::make_pair(id, std::make_unique<Invoker<Args...>>(std::move(call)))).second;
		assert(callRegistered);
		return true;
	}

	bool removeCall(uint32_t id)
	{
		auto it = lookupTable.find(id);

		if(it == lookupTable.end())
			return false;

		lookupTable.erase(it);
		return true;
	}

	template<class... Args>
	inline void makeCallItem(CallItem* ci, uint32_t id, pet::Tuple<Args...>&& args)
	{
		*((int*)ci->data) = id;
		Marshaller(*this, ci->data + sizeof(int), ci->data + sizeof(ci->data)).processTuple(std::move(args));
	}

	inline RpcCore(Serializer&& ser = Serializer{}, Deserializer&& des = Deserializer{}):
		Serializer(std::move(ser)), Deserializer(std::move(des)){}
};

#endif /* COMMON_RPCCORE_H_ */
