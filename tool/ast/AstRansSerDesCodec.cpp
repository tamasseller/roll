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
	size_t wcTotalStringOverhead = 0;
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
		wcTotalStringOverhead += v.length() - 1;

		items.push_back([v{std::move(v)}](RansEncoder & enc)
		{
			auto it = v.end();
			enc.put(nonIntialIdCharModel, nonInitialCharToIndex('\0'));

			for(--it; it != v.begin(); --it)
				enc.put(nonIntialIdCharModel, nonInitialCharToIndex(*it));

			enc.put(intialIdCharModel, initialCharToIndex(*it));
		});
	}

	inline std::string result()
	{
		RansEncoder enc(100 *(items.size() + wcTotalStringOverhead));

		for(auto it = items.rbegin(); it != items.rend(); it++)
		{
			(*it)(enc);
		}

		auto s = enc.done();
		return {s.begin(), s.end()};
	}
};

struct RansSource: AstDeserializer<RansSource>, RansDecoder
{
	RansSource(std::istream &input): RansDecoder(input) {}

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

	static_assert(sizeof(initialChars) / sizeof(initialChars[0]) == 53);

	for(char c: initialChars)
	{
		const auto idx = initialCharToIndex(c);
		assert(c == initialIndexToChar(idx));
		const auto range = intialIdCharModel.predict(idx);

		char r, s, t;
		intialIdCharModel.identify(range.cummulated, r);
		intialIdCharModel.identify(range.cummulated + range.width / 2,  s);
		intialIdCharModel.identify(range.cummulated + range.width - 1, t);
		assert(r == idx && s == idx && t == idx);
	}

	static constexpr const char nonInitialChars[] =
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'
	};

	static_assert(sizeof(nonInitialChars) / sizeof(nonInitialChars[0]) == 64);

	for(char c: nonInitialChars)
	{
		const auto idx = nonInitialCharToIndex(c);
		assert(c == nonInitialIndexToChar(idx));
		const auto range = nonIntialIdCharModel.predict(idx);

		char r, s, t;
		nonIntialIdCharModel.identify(range.cummulated, r);
		nonIntialIdCharModel.identify(range.cummulated + range.width / 2, s);
		nonIntialIdCharModel.identify(range.cummulated + range.width - 1, t);
		assert(r == idx && s == idx && t == idx);
	}

	AstSerDes::RootSelector rootSelectorValues[] = {AstSerDes::RootSelector::Func, AstSerDes::RootSelector::Type, AstSerDes::RootSelector::Session, AstSerDes::RootSelector::None };
	static_assert(sizeof(rootSelectorValues) / sizeof(rootSelectorValues[0]) == 4);

	for(auto c: rootSelectorValues)
	{
		const auto idx = (char)(int)c;
		const auto range = rootSelectorModel.predict(idx);

		char r, s, t;
		rootSelectorModel.identify(range.cummulated, r);
		rootSelectorModel.identify(range.cummulated + range.width / 2, s);
		rootSelectorModel.identify(range.cummulated + range.width - 1, t);
		assert(r == idx && s == idx && t == idx);
	}

	AstSerDes::TypeSelector typeSelectorValues[] = {AstSerDes::TypeSelector::Primitive, AstSerDes::TypeSelector::Collection, AstSerDes::TypeSelector::Aggregate, AstSerDes::TypeSelector::Alias, AstSerDes::TypeSelector::None};
	static_assert(sizeof(typeSelectorValues) / sizeof(typeSelectorValues[0]) == 5);

	for(auto c: typeSelectorValues)
	{
		const auto idx = (char)(int)c;
		const auto range = typeSelectorModel.predict(idx);

		char r, s, t;
		typeSelectorModel.identify(range.cummulated, r);
		typeSelectorModel.identify(range.cummulated + range.width / 2, s);
		typeSelectorModel.identify(range.cummulated + range.width - 1, t);
		assert(r == idx && s == idx && t == idx);
	}

	AstSerDes::PrimitiveSelector primitiveSelectorValues[] = {AstSerDes::PrimitiveSelector::I1, AstSerDes::PrimitiveSelector::U1, AstSerDes::PrimitiveSelector::I2, AstSerDes::PrimitiveSelector::U2, AstSerDes::PrimitiveSelector::I4, AstSerDes::PrimitiveSelector::U4, AstSerDes::PrimitiveSelector::I8, AstSerDes::PrimitiveSelector::U8};
	static_assert(sizeof(primitiveSelectorValues) / sizeof(primitiveSelectorValues[0]) == 8);

	for(auto c: primitiveSelectorValues)
	{
		const auto idx = (char)(int)c;
		const auto range = primitiveSelectorModel.predict(idx);

		char r, s, t;
		primitiveSelectorModel.identify(range.cummulated, r);
		primitiveSelectorModel.identify(range.cummulated + range.width / 2, s);
		primitiveSelectorModel.identify(range.cummulated + range.width - 1, t);
		assert(r == idx && s == idx && t == idx);
	}

	AstSerDes::SessionItemSelector sessionItemSelectorValues[] = {AstSerDes::SessionItemSelector::Constructor, AstSerDes::SessionItemSelector::ForwardCall, AstSerDes::SessionItemSelector::CallBack, AstSerDes::SessionItemSelector::None};
	static_assert(sizeof(sessionItemSelectorValues) / sizeof(sessionItemSelectorValues[0]) == 4);

	for(auto c: sessionItemSelectorValues)
	{
		const auto idx = (char)(int)c;
		const auto range = sessionItemSelectorModel.predict(idx);

		char r, s, t;
		sessionItemSelectorModel.identify(range.cummulated, r);
		sessionItemSelectorModel.identify(range.cummulated + range.width / 2, s);
		sessionItemSelectorModel.identify(range.cummulated + range.width - 1, t);
		assert(r == idx && s == idx && t == idx);
	}
}

std::string serialize(const Ast& ast)
{
	selftest();

	static constexpr const auto version = 0;

	RansSink snk;
	snk.traverse(ast);

	std::stringstream ss;
	unsigned char v = 0xff - version;
	ss.write((char*)&v, 1);
	ss << snk.result();

	return ss.str();
}

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
