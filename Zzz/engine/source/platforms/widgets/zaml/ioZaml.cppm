#include "pch.h"
export module iozaml;

import result;

using namespace zzz;
using namespace zzz::result;

export namespace zzz::zaml
{
	class zamlNode
	{
	public:
		std::string name;
		std::unordered_map<std::string, std::string> attributes;
		std::vector<zamlNode> children;
		std::string content;

		zResult<const std::string*> GetAttribute(const std::string& key) const;
		zResult<const zamlNode*> FindFirst(const std::string& tagName) const;
		zResult<std::vector<const zamlNode*>> FindAll(const std::string& tagName) const;
	};

	zResult<const std::string*> zamlNode::GetAttribute(const std::string& key) const
	{
		auto it = attributes.find(key);

		if (it != attributes.end())
			return &it->second;
			
		return Unexpected(eResult::no_find);
	}

	zResult<const zamlNode*> zamlNode::FindFirst(const std::string& tagName) const
	{
		for (const auto& child : children)
			if (child.name == tagName)
				return &child;

		return Unexpected(eResult::no_find);
	}

	zResult<std::vector<const zamlNode*>> zamlNode::FindAll(const std::string& tagName) const
	{
		std::vector<const zamlNode*> result;
		for (const auto& child : children)
			if (child.name == tagName)
				result.push_back(&child);

		if (result.empty())
			return Unexpected(eResult::no_find);

		return result;
	}

	class ioZaml
	{
	public:
		zResult<zamlNode> LoadFromFile(const std::wstring& filename)
		{
			std::ifstream file(filename);
			if (!file)
				return Unexpected(eResult::io_error_open_file, L">>>>> [ioZaml.LoadFromFile( ... )]. Failed to open file: " + std::wstring(filename));

			return ParseNode(file);
		}

		zResult<> SaveToFile(const std::wstring& filename, const zamlNode& node, int indent = 0)
		{
			std::ofstream file(filename);
			if (!file)
				return Unexpected(eResult::io_error_open_file,  L">>>>> [ioZaml.SaveToFile( ... )]. Failed to open file: " + std::wstring(filename));

			WriteNode(file, node, indent);

			return {};
		}

	private:
		std::string Trim(const std::string& s)
		{
			size_t start = s.find_first_not_of(" \t\r\n");
			size_t end = s.find_last_not_of(" \t\r\n");

			return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
		}

		std::unordered_map<std::string, std::string> ParseAttributes(const std::string& str)
		{
			std::unordered_map<std::string, std::string> attrs;
			size_t pos = 0;
			while (pos < str.size())
			{
				while (pos < str.size() && std::isspace(str[pos])) ++pos;

				size_t nameStart = pos;
				while (pos < str.size() && str[pos] != '=' && !std::isspace(str[pos])) ++pos;

				std::string name = str.substr(nameStart, pos - nameStart);
				while (pos < str.size() && (std::isspace(str[pos]) || str[pos] == '=')) ++pos;
				if (pos < str.size() && str[pos] == '"')
				{
					++pos;
					size_t valueStart = pos;
					size_t valueEnd = str.find('"', pos);
					if (valueEnd != std::string::npos)
					{
						attrs[name] = str.substr(valueStart, valueEnd - valueStart);
						pos = valueEnd + 1;
					}
				}
			}

			return attrs;
		}

		zResult<zamlNode> ParseNode(std::istream& in)
		{
			std::string line;
			while (std::getline(in, line))
			{
				line = Trim(line);

				if (line.empty())
					continue;

				if (line.starts_with("<") && !line.starts_with("</"))
				{
					zamlNode node;
					bool selfClosing = false;
					size_t nameEnd = line.find_first_of(" />", 1);
					node.name = line.substr(1, nameEnd - 1);
					size_t attrStart = nameEnd;
					size_t tagEnd = line.find('>', attrStart);
					if (line[tagEnd - 1] == '/')
					{
						selfClosing = true;
						tagEnd--;
					}
					std::string attrStr = line.substr(attrStart, tagEnd - attrStart);
					node.attributes = ParseAttributes(attrStr);

					if (selfClosing)
						return node;

					while (true)
					{
						auto pos = in.tellg();
						std::string nextLine;

						if (!std::getline(in, nextLine))
							break;

						nextLine = Trim(nextLine);

						if (nextLine.starts_with("</" + node.name))
							break;

						in.seekg(pos);
						auto resNode = ParseNode(in);
						if (resNode)
							node.children.push_back(resNode.value());
						else
							return Unexpected(eResult::failure, L">>>>> [ioZaml.ParseNode( ... )]. Failed to parse child node: " + std::wstring(resNode.error().getMessage()));
					}
					return node;
				}
			}

			return Unexpected(eResult::failure, L" [ioZaml.ParseNode( ... )]. End method");
		}

		void WriteNode(std::ostream& out, const zamlNode& node, int indent)
		{
			std::string ind(indent, ' ');
			out << ind << '<' << node.name;
			for (const auto& [k, v] : node.attributes)
				out << ' ' << k << "=\"" << v << "\"";

			if (node.children.empty() && node.content.empty())
			{
				out << " />\n";
				return;
			}

			out << ">";
			if (!node.content.empty())
				out << node.content;

			if (!node.children.empty())
				out << "\n";

			for (const auto& child : node.children)
				WriteNode(out, child, indent + 2);

			if (!node.children.empty())
				out << ind;

			out << "</" << node.name << ">\n";
		}
	};
}