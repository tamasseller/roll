#ifndef ROLL_CPP_FRAMEWORK_TRACKER_H_
#define ROLL_CPP_FRAMEWORK_TRACKER_H_

#include "Fail.h"

namespace rpc {

class Tracker
{
public:
	class Subobject
	{
		static inline constexpr uintptr_t notInListValue = -1u;

		friend Tracker;

	public:
		inline Subobject *asSubobject() {
			return this;
		}

	private:
		class Subobject* nextSubobject = (Subobject *)notInListValue;
		rpc::Call<> close;

	protected:
		Subobject(const Subobject&) = delete;
		Subobject& operator=(const Subobject&) = delete;
		Subobject(Subobject&&) = delete;
		Subobject& operator=(Subobject&&) = delete;

		inline Subobject() = default;

		inline ~Subobject()
		{
			if(nextSubobject != (Subobject *)notInListValue)
			{
				rpc::fail("Registered sub-object destroyed"); /* GCOV_EXCL_LINE */
			}
		}
	};

private:
	Subobject *first = nullptr;

protected:
	inline void addSubobject(Subobject* o, rpc::Call<> close)
	{
		if(o->nextSubobject != (Subobject *)Subobject::notInListValue)
		{
			rpc::fail("Re-registering sub-object"); /* GCOV_EXCL_LINE */
		}

		o->close = close;
		o->nextSubobject = first;
		first = o;
	}

	inline void removeSubobject(Subobject* o)
	{
		if(o->nextSubobject == (Subobject *)Subobject::notInListValue)
		{
			rpc::fail("Removing unregistering sub-object"); /* GCOV_EXCL_LINE */
		}

		for(auto i = &first; *i != nullptr; i = &((*i)->nextSubobject))
		{
			if(*i == o)
			{
				*i = o->nextSubobject;
				o->nextSubobject = (Subobject *)Subobject::notInListValue;
				return;
			}
		}

		rpc::fail("Sub-object not found"); /* GCOV_EXCL_LINE */
	}

	template<class Ep>
	inline void notifySubobjects(Ep& ep)
	{
		while(first)
		{
			ep.simulateCall(first->close);
		}
	}

	inline ~Tracker()
	{
		if(first)
		{
			rpc::fail("Sub-object(s) still registered when destroying tracker"); /* GCOV_EXCL_LINE */
		}
	}
};

}

#endif /* ROLL_CPP_FRAMEWORK_TRACKER_H_ */
