#include "CppCommon.h"

std::string printDocs(const std::string& str, const int n)
{
	std::stringstream ss;
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

	return ss.str();
}

bool writeBlock(std::stringstream& ss, const std::string &header, const std::vector<std::string>& strs, const int n)
{
	bool first = true;
	bool prevMultiLine;
	for(const auto& s: strs)
	{
		if(s.length())
		{
			bool multiline = s.find_first_of('\n') != std::string::npos;

			if(first)
			{
				first = false;
				ss << indent(n) << header << std::endl;
				ss << indent(n) << "{" << std::endl;
			}
			else
			{
				if(multiline || prevMultiLine)
				{
					ss << std::endl;
				}
			}

			ss << s << std::endl;
			prevMultiLine = multiline;
		}
	}

	if(!first)
	{
		ss << indent(n) << "}";
		return true;
	}

	return false;
}

void writeTopLevelBlock(std::stringstream& ss, const std::string &header, const std::vector<std::string>& strs, bool addSemi)
{
	if(writeBlock(ss, header, strs, 0))
	{
		ss << (addSemi ? ";" : "") << std::endl << std::endl;
	}
}
