#include "AstRansSerDesCodec.h"

#include "AstSerDes.h"
#include "RansCodec.h"

#include <functional>
#include <iterator>

static StaticConstantRansModel<char(4)> rootSelectorModel;
static StaticConstantRansModel<char(5)> typeSelectorModel;
static StaticConstantRansModel<char(8)> primitiveSelectorModel;
static StaticConstantRansModel<char(4)> sessionItemSelectorModel;
static StaticConstantRansModel<char(53)> intialIdCharModel;
static StaticConstantRansModel<char(64)> nonIntialIdCharModel;

static inline char initialCharToIndex(char c)
{
	if(c == '_')
		return 0;
	if('a' <= c && c <= 'z')
		return c - 'a' + 1;
	else if('A' <= c && c <= 'Z')
		return c - 'A' + 27;
	else
		throw std::runtime_error("Unexpected character in identifier: '" + std::string(1, c) + "'");
}

static inline char initialIndexToChar(char x)
{
	if(x == 0)
		return '_';
	if(1 <= x && x <= 26)
		return x - 1 + 'a';
	else if(27 <= x && x <= 52)
		return x - 27 + 'A';
	else
		throw std::runtime_error("Unexpected index in identifier: " + std::to_string((int)x));
}

char nonInitialCharToIndex(char c)
{
	if(c == '_')
		return 0;
	if('a' <= c && c <= 'z')
		return c - 'a' + 1;
	else if('A' <= c && c <= 'Z')
		return c - 'A' + 27;
	else if('0' <= c && c <= '9')
		return c - '0' + 53;
	else if(c == '\0')
		return 63;
	else
		throw std::runtime_error("Unexpected character in identifier: '" + std::string(1, c) + "'");
}

static inline char nonInitialIndexToChar(char x)
{
	if(x == 0)
		return '_';
	if(1 <= x && x <= 26)
		return x - 1 + 'a';
	else if(27 <= x && x <= 52)
		return x - 27 + 'A';
	else if(53 <= x && x <= 62)
		return x - 53 + '0';
	else if(x == 63)
		return 0;
	else
		throw std::runtime_error("Unexpected index in identifier: " + std::to_string((int)x));
}

struct RansSink: AstSerializer<RansSink>
{
	static constexpr const auto version = 0;
	size_t wc_size = 0;
	std::vector<std::function<void(RansEncoder &)>> items;

	inline void write(RootSelector v)
	{
		items.push_back([v](RansEncoder & enc){
			enc.put(rootSelectorModel, (int)v);
		});
	}

	inline void write(TypeSelector v)
	{
		items.push_back([v](RansEncoder & enc){
			enc.put(typeSelectorModel, (int)v);
		});
	}

	inline void write(PrimitiveSelector v)
	{
		items.push_back([v](RansEncoder & enc){
			enc.put(primitiveSelectorModel, (int)v);
		});
	}

	inline void write(SessionItemSelector v)
	{
		items.push_back([v](RansEncoder & enc){
			enc.put(sessionItemSelectorModel, (int)v);
		});
	}

	inline void write(std::string v)
	{
		wc_size += v.length() - 1;
		items.push_back([v{std::move(v)}](RansEncoder & enc){
			auto it = v.end();
			enc.put(nonIntialIdCharModel, nonInitialCharToIndex('\0'));

			for(--it; it != v.begin(); --it)
				enc.put(nonIntialIdCharModel, nonInitialCharToIndex(*it));

			enc.put(intialIdCharModel, initialCharToIndex(*it));
		});
	}

	inline std::string result()
	{
		RansEncoder enc(items.size() + wc_size);

		for(auto it = items.rbegin(); it != items.rend(); it++)
		{
			(*it)(enc);
		}

		std::stringstream ss;
		unsigned char v = 0xff - version;
		ss.write((char*)&v, 1);
		ss << enc.done();
		return ss.str();
	}
};

std::string serialize(const Ast& ast)
{
	RansSink snk;
	snk.traverse(ast);
	return snk.result();
}

struct RansSource: AstDeserializer<RansSource>, RansDecoder<std::istream_iterator<char>>
{
	RansSource(std::istream &input): RansDecoder(std::istream_iterator<char>(input)) {}

	inline void read(RootSelector &v) {
		v = (RootSelector)this->get(rootSelectorModel);
	}

	inline void read(TypeSelector &v) {
		v = (TypeSelector)this->get(typeSelectorModel);
	}

	inline void read(PrimitiveSelector &v) {
		v = (PrimitiveSelector)this->get(primitiveSelectorModel);
	}

	inline void read(SessionItemSelector &v) {
		v = (SessionItemSelector)this->get(sessionItemSelectorModel);
	}

	inline void read(std::string &v)
	{
		std::stringstream ss;

		ss << initialIndexToChar(this->get(intialIdCharModel));

		while(char c = nonInitialIndexToChar(this->get(nonIntialIdCharModel)))
		{
			ss << c;
		}

		v = ss.str();
	}
};

Ast deserialize(std::istream& input)
{
	unsigned char v;
	input.read((char*)&v, 1);
	const auto version = 0xff - v;

	switch(version)
	{
	case 0:
		return RansSource(input).build();
	default:
		throw std::runtime_error("Unsupported version: " + std::to_string((int)v));
	}
}


#if 0
void selftest()
{
	static constexpr const char initialChars[] =
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'_'
	};

	static constexpr const char nonInitialChars[] =
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'
	};

	for(char c: initialChars)
	{
		const auto idx = initialCharToIndex(c);
		assert(c == initialIndexToChar(idx));
		const auto range = intialIdCharModel.predict(idx);

		{
			char r;
			intialIdCharModel.identify(range.cummulated, r);
			assert(r == idx);
		}

		{
			char r;
			intialIdCharModel.identify(range.cummulated + range.width - 1, r);
			assert(r == idx);
		}
	}

	for(char c: nonInitialChars)
	{
		const auto idx = nonInitialCharToIndex(c);
		assert(c == nonInitialIndexToChar(idx));
		const auto range = nonIntialIdCharModel.predict(idx);

		{
			char r;
			nonIntialIdCharModel.identify(range.cummulated, r);
			assert(r == idx);
		}

		{
			char r;
			nonIntialIdCharModel.identify(range.cummulated + range.width - 1, r);
			assert(r == idx);
		}
	}
}
#endif
