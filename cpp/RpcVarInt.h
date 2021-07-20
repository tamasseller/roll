#ifndef _RPCVARINT_H_
#define _RPCVARINT_H_

#include <stddef.h>
#include <stdint.h>

namespace rpc {

/**
 * Variable length encoding for unsigned 32 bit values.
 * 
 * Uses little endian base 128 (LEB128) encoding. 
 */
struct VarUint4
{
	/**
	 * Determine the corresponding encoded byte sequence length for a value.
	 */
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

	/**
	 * Write 32 bit unsigned value to stream using variable length encoding.
	 */
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

	/**
	 * Read 32 bit unsigned variable length coded value from stream.
	 */
	template<class S> static inline bool read(S& s, uint32_t &v)
	{
		uint8_t nBytes = 0;
		v = 0;

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

	/**
	 * Skip a variable length encodied value in stream.
	 */
	template<class S> static inline bool skip(S& s)
	{
		uint8_t d, nBytes = 0;

		while(s.read(d))
		{
			if(++nBytes == 5)
				return true;

			if(!(d & 0x80))
				return true;
		}

		return false;
	}

	/**
	 * Streaming variable length decoder state machine.
	 */
	class Reader
	{
		static constexpr auto lastShift = (32 - 4 * 7);
		static constexpr auto endMask = 1u << (lastShift - 1);
		uint32_t data = 1u << 31;

	public:
		inline bool process(char d)
		{
			if(data & endMask)
			{
				data = (data >> 4) | ((uint32_t)d << (32 - 4));
				return true;
			}
			else
			{
				data = (data >> 7) | ((uint32_t)d << (32 - 7));

				if(!(d & 0x80))
				{
					while(!(data & endMask))
						data >>= 7;

					data >>= lastShift;
					return true;
				}
			}

			return false;
		}

		inline auto getResult() const {
			return data;
		}
	};
};

}

#endif /* _RPCVARINT_H_ */
