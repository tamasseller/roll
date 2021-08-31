#ifndef RPC_TOOL_RANSCODEC_H_
#define RPC_TOOL_RANSCODEC_H_

#include <cstdint>
#include <cassert>

class RansCodec
{
protected:
	static constexpr uint32_t rangeScaleBits = 16;
	static constexpr uint32_t normalizedLowerBound = 1u << 24;
	static constexpr uint32_t maxMult = 1u << (32 - rangeScaleBits);
	static constexpr uint32_t mask = (1u << rangeScaleBits) - 1;
};

template<char count>
struct StaticConstantRansModel
{
	struct Symstat
	{
		size_t cummulated, width;

		inline Symstat(unsigned int idx):
			cummulated(0x10000u * (idx + 0) / (unsigned int)(unsigned char)count),
			width((0x10000u * (idx + 1) / (unsigned int)(unsigned char)count) - cummulated) {}
	};

	static inline Symstat predict(char c) {
		assert(c < count);
		return Symstat((unsigned char)c);
	}

	static inline Symstat identify(size_t in, char &c)
	{
		c = (in * count + 0x7fffu) / 0x10000u;

		if (const auto thres = (0x10000u * c) / count;in < thres)
		{
			c--;
		}

		return predict(c);
	}
};

template<class It>
class RansDecoder: RansCodec
{
	uint32_t x;
	It in;

public:
	inline RansDecoder(It&& in): in(in)
	{
		x = (unsigned char)*this->in++ << 0;
		x |= (unsigned char)*this->in++ << 8;
		x |= (unsigned char)*this->in++ << 16;
		x |= (unsigned char)*this->in++ << 24;
	}

	template<class Model>
	inline char get(Model& model) {
		char ret;
		auto low = x & mask;
		auto r = model.identify(low, ret);

		x = r.width * (x >> rangeScaleBits) + low - r.cummulated;

		while (x < normalizedLowerBound)
			x = (x << 8) | (unsigned char)*in++;

		return ret;
	}

	inline bool check() {
		return x == normalizedLowerBound;
	}
};

class RansEncoder: RansCodec, public std::unique_ptr<char[]>
{
	const size_t length;
	uint32_t x = normalizedLowerBound;
	char *o;

public:
	inline RansEncoder(size_t length):
		unique_ptr(new char[length]),
		length(length),
		o(get() + length - 1) {}

	template<class Model>
	inline void put(Model &model, char data)
	{
		auto r = model.predict(data);

		for(; x >= maxMult * r.width; x >>= 8)
		{
			*o-- = x;
		}

		x = ((x / r.width) << rangeScaleBits) + (x % r.width) + r.cummulated;
	}

	inline std::string_view done()
	{
		*o-- = x >> 24;
		*o-- = x >> 16;
		*o-- = x >> 8;
		*o = x >> 0;

		auto len = get() + length - o;
		return {o, (size_t)len};
	}
};

#endif /* RPC_TOOL_RANSCODEC_H_ */
