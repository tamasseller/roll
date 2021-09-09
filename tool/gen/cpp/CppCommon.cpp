#include "CppCommon.h"

void printDocs(std::stringstream &ss, const std::string& str, const int n)
{
	if(str.length())
	{
		std::stringstream in(str);

		std::string line;
		bool gotLine = (bool)std::getline(in, line, '\n');
		assert(gotLine);

		bool first = true;
		while(true)
		{
			if(first)
			{
				ss << indent(n) << "/* ";
				first = false;
			}
			else if(line.length())
			{
				if(line[0] == '*')
				{
					ss << " ";
				}
				else
				{
					ss << "   ";
				}
			}

			ss << line;

			std::string next(1, '\0');
			if(std::getline(in, next, '\n') || !(next.length() == 1 && next[0] == '\0'))
			{
				ss << std::endl << indent(n);
				line = std::move(next);
			}
			else
			{
				break;
			}
		}

		ss << " */" << std::endl;
	}
}
