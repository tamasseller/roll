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

template<unsigned char count>
struct StaticConstantRansModel: RansCodec
{
	static inline uint32_t start(uint32_t idx) {
		return (idx << (rangeScaleBits)) / (uint32_t)count;
	}

	static inline uint32_t approx(uint32_t idx) {
		return (idx * count) >> (rangeScaleBits);
	}

	struct Symstat
	{
		size_t cummulated, width;

		inline Symstat(unsigned int idx):
			cummulated(start(idx)),
			width(start(idx + 1) - cummulated) {}
	};

	static inline Symstat predict(char c)
	{
		assert(0 <= c && c < count);
		return Symstat((unsigned char)c);
	}

	static inline Symstat identify(size_t in, char &c)
	{
		auto u = approx(in);
		c = ((start(u + 1) <= in) ? (u + 1) : u);

		assert(start(c) <= in && in < start(c + 1));
		return predict(c);
	}
};

class RansDecoder: RansCodec
{
	uint32_t x;
	std::istream &in;

	unsigned char read()
	{
		unsigned char ret;
		in.read((char*)&ret, 1);
		return ret;
	}

public:
	inline RansDecoder(std::istream &in): in(in)
	{
		x = read() << 0;
		x |= read() << 8;
		x |= read() << 16;
		x |= read() << 24;
	}

	template<class Model>
	inline char get(Model& model)
	{
		char ret;
		auto low = x & mask;
		auto r = model.identify(low, ret);

		x = r.width * (x >> rangeScaleBits) + low - r.cummulated;

		while (x < normalizedLowerBound)
			x = (x << 8) | read();

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
