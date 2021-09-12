#include "ContractTextCodec.h"

#include "ContractSerDes.h"

template<class> struct Mapping;

template<>struct Mapping<ContractSerDes::RootSelector>: ContractSerDes
{
	static constexpr char funcChar = '(';
	static constexpr char typeChar = '=';
	static constexpr char sessChar = '<';
	static constexpr char noneChar = '\n';

	static inline char encode(RootSelector v)
	{
		switch(v)
		{
			case RootSelector::Func: return funcChar;
			case RootSelector::Type: return typeChar;
			case RootSelector::Session: return sessChar;
			case RootSelector::None: return noneChar;
			default: throw std::runtime_error("Invalid input to encoder");
		}
	}

	static inline RootSelector decode(char c)
	{
		switch(c)
		{
			case funcChar: return RootSelector::Func;
			case typeChar: return RootSelector::Type;
			case sessChar: return RootSelector::Session;
			case noneChar: return RootSelector::None;
			default: throw std::runtime_error("Invalid code found during decoding");
		}
	}
};

template<>struct Mapping<ContractSerDes::TypeRefSelector>: ContractSerDes
{
	static constexpr char nameChar = '?';
	static constexpr char collChar = '[';
	static constexpr char primChar = '$';
	static constexpr char noneChar = ',';

	static inline char encode(TypeRefSelector v)
	{
		switch(v)
		{
			case TypeRefSelector::Alias: return nameChar;
			case TypeRefSelector::Collection: return collChar;
			case TypeRefSelector::Primitive: return primChar;
			case TypeRefSelector::None: return noneChar;
			default: throw std::runtime_error("Invalid input to encoder");
		}
	}

	static inline TypeRefSelector decode(char c)
	{
		switch(c)
		{
			case nameChar: return TypeRefSelector::Alias;
			case collChar: return TypeRefSelector::Collection;
			case primChar: return TypeRefSelector::Primitive;
			case noneChar: return TypeRefSelector::None;
			default: throw std::runtime_error("Invalid code found during decoding");
		}
	}
};

template<>struct Mapping<ContractSerDes::TypeDefSelector>: ContractSerDes
{
	static constexpr char nameChar = ':';
	static constexpr char collChar = ']';
	static constexpr char primChar = '#';
	static constexpr char aggrChar = '{';

	static inline char encode(TypeDefSelector v)
	{
		switch(v)
		{
			case TypeDefSelector::Alias: return nameChar;
			case TypeDefSelector::Collection: return collChar;
			case TypeDefSelector::Primitive: return primChar;
			case TypeDefSelector::Aggregate: return aggrChar;
			default: throw std::runtime_error("Invalid input to encoder");
		}
	}

	static inline TypeDefSelector decode(char c)
	{
		switch(c)
		{
			case nameChar: return TypeDefSelector::Alias;
			case collChar: return TypeDefSelector::Collection;
			case primChar: return TypeDefSelector::Primitive;
			case aggrChar: return TypeDefSelector::Aggregate;
			default: throw std::runtime_error("Invalid code found during decoding");
		}
	}
};


template<>struct Mapping<ContractSerDes::SessionItemSelector>: ContractSerDes
{
	static constexpr char callChar = '>';
	static constexpr char backChar = '<';
	static constexpr char ctorChar = '-';
	static constexpr char noneChar = '\t';

	static inline char encode(SessionItemSelector v)
	{
		switch(v)
		{
			case SessionItemSelector::ForwardCall: return callChar;
			case SessionItemSelector::CallBack: return backChar;
			case SessionItemSelector::Constructor: return ctorChar;
			case SessionItemSelector::None: return noneChar;
			default: throw std::runtime_error("Invalid input to encoder");
		}
	}

	static inline SessionItemSelector decode(char c)
	{
		switch(c)
		{
			case callChar: return SessionItemSelector::ForwardCall;
			case backChar: return SessionItemSelector::CallBack;
			case ctorChar: return SessionItemSelector::Constructor;
			case noneChar: return SessionItemSelector::None;
			default: throw std::runtime_error("Invalid code found during decoding");
		}
	}
};

template<>struct Mapping<Contract::Primitive>: ContractSerDes
{
	static inline char encode(Contract::Primitive v)
	{
		switch(v)
		{
			case Contract::Primitive::I1: return '0';
			case Contract::Primitive::U1: return '1';
			case Contract::Primitive::I2: return '2';
			case Contract::Primitive::U2: return '3';
			case Contract::Primitive::I4: return '4';
			case Contract::Primitive::U4: return '5';
			case Contract::Primitive::I8: return '6';
			case Contract::Primitive::U8: return '7';
			case Contract::Primitive::Bool: return '8';
			default: throw std::runtime_error("Invalid input to encoder");
		}
	}

	static inline Contract::Primitive decode(char c)
	{
		switch(c)
		{
			case '0': return Contract::Primitive::I1;
			case '1': return Contract::Primitive::U1;
			case '2': return Contract::Primitive::I2;
			case '3': return Contract::Primitive::U2;
			case '4': return Contract::Primitive::I4;
			case '5': return Contract::Primitive::U4;
			case '6': return Contract::Primitive::I8;
			case '7': return Contract::Primitive::U8;
			case '8': return Contract::Primitive::Bool;
			default: throw std::runtime_error("Invalid code found during decoding");
		}
	}
};

struct TextSink: ContractSerializer<TextSink>
{
	std::stringstream ss;

	template<class S>
	inline void write(S v)
	{
		const char c = Mapping<S>::encode(v);
		ss.write(&c, 1);
	}

	inline void writeString(const std::string &v)
	{
		ss.write(v.c_str(), v.length() + 1);
	}

	inline void writeIdentifier(std::string v) {
		writeString(v);
	}

	inline void writeText(std::string v) {
		writeString(v);
	}
};

struct TextSource: ContractDeserializer<TextSource>
{
	std::istream &is;
	TextSource(std::istream &input): is(input) {}

	template<class S>
	inline void read(S &v)
	{
		char c;
		is.read(&c, 1);
		v = Mapping<S>::decode(c);
	}

	inline std::string readString()
	{
		std::stringstream ss;

		while(true)
		{
			char c = 0;
			is.read(&c, 1);

			if(!c)
			{
				break;
			}

			ss.write(&c, 1);
		}

		return ss.str();
	}

	inline void readIdentifier(std::string &v) {
		v = readString();
	}

	inline void readText(std::string &v) {
		v = readString();
	}

};

std::string serializeText(const std::vector<Contract>& ast)
{
	static constexpr const auto version = 0;

	TextSink snk;
	snk.traverse(ast);

	std::stringstream ss;
	unsigned char v = 0xff - version;
	ss.write((char*)&v, 1);
	const auto data = snk.ss.str();
	ss.write(data.c_str(), data.length());

	return ss.str();
}

std::vector<Contract> deserializeText(std::istream& input)
{
	unsigned char v;
	input.read((char*)&v, 1);
	const auto version = 0xff - v;

	switch(version)
	{
	case 0:
		return TextSource(input).build();
	default:
		throw std::runtime_error("Unsupported version: " + std::to_string((int)v));
	}
}
