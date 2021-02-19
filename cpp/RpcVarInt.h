#ifndef _RPCVARINT_H_
#define _RPCVARINT_H_

#include <stddef.h>
#include <stdint.h>

namespace rpc {

struct VarUint4
{
	static constexpr inline size_t size(uint32_t c) 
	{
		if(c < 128)
			return 1;
		else if(c < 128 * 128)
			return 2;
		else if(c < 128 * 128 * 128)
			return 3;
		else if(c < 128 * 128 * 128 * 128)
			return 4;

		return 5;
	}

	template<class S> static inline bool write(S& s, uint32_t v) 
	{
		while(v >= 0x80)
		{
			if(!s.write(uint8_t(v | 0x80)))
				return false;

			v >>= 7;
		}

		return s.write(uint8_t(v));
	}

	template<class S> static inline bool read(S& s, uint32_t &v)
	{
		uint8_t nBytes = 0;

		while(true)
		{
			uint8_t d;
			if(!s.read(d))
				return false;

			if(++nBytes < 5)
			{
				v = (v >> 7) | ((uint32_t)d << (32 - 7));

				if(!(d & 0x80))
				{
					v >>= 32 - 7 * nBytes;
					return true;
				}
			}
			else
			{
				v = (v >> 4) | (((uint32_t)d) << 28);
				return true;
			}
		}
	}
};

}

#endif /* _RPCVARINT_H_ */