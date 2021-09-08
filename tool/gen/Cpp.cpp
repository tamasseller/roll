#include "Cpp.h"

const CodeGenCpp CodeGenCpp::instance;

std::string CodeGenCpp::generateClient(const Contract&) const
{
	return "c++ client";
}

std::string CodeGenCpp::generateServer(const Contract&) const
{
	return "c++ server";
}
