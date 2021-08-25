#include <iostream>
#include <fstream>
#include <map>

#include "antlr4-runtime/antlr4-runtime.h"

#include "DumpMode.h"

std::map<std::string, std::pair<std::string, const ModeInterface*>> modes =
{
	{"dump", {"parse and display descriptor", &DumpMode::instance}}
};

void displayModes(std::string name)
{
	std::cerr << "Usage: " << name << " <mode> <input> [options]"<< std::endl;

	std::cerr << "Available modes:" << std::endl;

	for(auto m: modes)
	{
		std::cerr << "\t" << m.first << "\t" << m.second.first << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		std::cerr << "No mode selected." << std::endl;
		displayModes(argv[0]);
		return -1;
	}
	else
	{
		if(auto it = modes.find(argv[1]); it == modes.end())
		{
			std::cerr << "Unknown mode: " << argv[1] << "." << std::endl;
			displayModes(argv[0]);
			return -1;
		}
		else if(argc < 3)
		{
			std::cerr << "No input specified." << std::endl;
			displayModes(argv[0]);
			return -1;
		}
		else
		{
			try
			{
				std::ifstream file(argv[2]);

				if(!file)
				{
					throw std::runtime_error("The file does not exist");
				}

				antlr4::ANTLRInputStream input(file);
				rpcLexer lexer(&input);
				antlr4::ConsoleErrorListener errorListener;
				lexer.addErrorListener(&errorListener);
				antlr4::CommonTokenStream tokens(&lexer);
				rpcParser parser(&tokens);
				parser.addErrorListener(&errorListener);

				return it->second.second->run(Ast::from(parser.rpc()), {argv + 2, argv + argc});
			}
			catch(const std::exception& exc)
			{
				std::cerr << "Could not read " << argv[2] << ": " << exc.what() << std::endl;
				displayModes(argv[0]);
				return -1;
			}
		}
	}
}
