#ifndef ROLL_CPP_TYPES_STDFORWARDLISTTYPEINFO_H_
#define ROLL_CPP_TYPES_STDFORWARDLISTTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <forward_list>

namespace rpc {

/**
 * Serialization rules for std::forward_list.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::forward_list<T>>: StlCollection<std::forward_list<T>, T>
{
    template<class S> static inline bool write(S& s, const std::forward_list<T>& v) {
        return TypeInfo::StlCollection::writeLengthAndContent(s, std::distance(v.begin(), v.end()), v);
    }

    template<class S> static inline bool read(S& s, std::forward_list<T>& v)
    {
        bool ok = TypeInfo::StlCollection::read(s, v, [](uint32_t count, std::forward_list<T>& v){
            return std::front_insert_iterator<std::forward_list<T>>(v);
        });

        if(ok)
        {
            v.reverse();
            return true;
        }

        return false;
    }

    template<class C> static constexpr inline size_t size(const C& v)
    {
        size_t contentSize = 0;
        uint32_t count = 0;

        for(const auto &x: v)
        {
            contentSize += TypeInfo<T>::size(x);
            count++;
        }

        return contentSize + VarUint4::size(count);
    }
};

}

#endif /* ROLL_CPP_TYPES_STDFORWARDLISTTYPEINFO_H_ */
