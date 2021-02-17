#include "RpcSerdes.h"

#include <string.h>

void RpcSerializer::serialize(char* &p, char* const end, const char* &&v)
{
	auto l = strlen(v);
	assert(l <= 255);

	auto newp = p + (1 + l);
	assert(newp <= end);

	memcpy(p, v, l + 1);

	p = newp;
}

void RpcDeserializer::deserialize(char* &p, char* const end, const char* &&v)
{
	auto l = strlen(p);
	assert(l <= 255);

	auto newp = p + (1 + l);
	assert(newp <= end);

	v = p;

	p = newp;
}
