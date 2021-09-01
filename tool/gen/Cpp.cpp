#include "Cpp.h"

const CodeGenCpp CodeGenCpp::instance;

std::string CodeGenCpp::generateClient(const Ast&) const
{
	return "c++ client";
}

std::string CodeGenCpp::generateServer(const Ast&) const
{
	return "c++ server";
}
