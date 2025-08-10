#include "pch.h"
export module iozaml;

import result;

using namespace zzz;

export namespace zzz::zaml
{
	class zamlNode
	{
	public:
		std::wstring name;
		std::unordered_map<std::wstring, std::wstring> attributes;
		std::vector<zamlNode> children;

		result<const std::wstring*> GetAttribute(const std::wstring& key) const;
		result<const zamlNode*> FindFirst(const std::wstring& tagName) const;
		result<std::vector<const zamlNode*>> FindAll(const std::wstring& tagName) const;
	};

	result<const std::wstring*> zamlNode::GetAttribute(const std::wstring& key) const
	{
		auto it = attributes.find(key);
		if (it != attributes.end())
			return &it->second;
		return Unexpected(eResult::no_find);
	}

	result<const zamlNode*> zamlNode::FindFirst(const std::wstring& tagName) const
	{
		for (const auto& child : children)
			if (child.name == tagName)
				return &child;
		return Unexpected(eResult::no_find);
	}

	result<std::vector<const zamlNode*>> zamlNode::FindAll(const std::wstring& tagName) const
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
		result<zamlNode> LoadFromFile(const std::wstring& filename);
		result<> SaveToFile(const std::wstring& filename, const zamlNode& node, int indent = 0);

	private:
		std::wstring EscapeXML(const std::wstring& s);
		std::wstring UnescapeXML(const std::wstring& s);
		std::wstring Trim(const std::wstring& s);
		std::unordered_map<std::wstring, std::wstring> ParseAttributes(const std::wstring& str);
		result<zamlNode> ParseNode(std::wistream& in);
		result<> WriteNode(std::wostream& out, const zamlNode& node, int indent);
	};

	result<zamlNode> ioZaml::LoadFromFile(const std::wstring& filename)
	{
		std::wifstream file(filename);
		if (!file)
			return Unexpected(eResult::io_error_open_file, L"[ioZaml.LoadFromFile] Failed to open file: " + filename);

		return ParseNode(file);
	}

	result<> ioZaml::SaveToFile(const std::wstring& filename, const zamlNode& node, int indent)
	{
		std::wofstream file(filename);
		if (!file)
			return Unexpected(eResult::io_error_open_file, L"[ioZaml.SaveToFile] Failed to open file: " + filename);

		return WriteNode(file, node, indent);
	}

	std::wstring ioZaml::EscapeXML(const std::wstring& s)
	{
		std::wstring out;
		for (wchar_t c : s)
		{
			switch (c)
			{
			case L'&':  out += L"&amp;"; break;
			case L'<':  out += L"&lt;"; break;
			case L'>':  out += L"&gt;"; break;
			case L'\"': out += L"&quot;"; break;
			case L'\'': out += L"&apos;"; break;
			default:    out += c; break;
			}
		}
		return out;
	}

	std::wstring ioZaml::UnescapeXML(const std::wstring& s)
	{
		std::wstring out;
		size_t i = 0;
		while (i < s.length())
		{
			if (s[i] == L'&')
			{
				if (s.compare(i, 5, L"&amp;") == 0) { out += L'&'; i += 5; }
				else if (s.compare(i, 4, L"&lt;") == 0) { out += L'<'; i += 4; }
				else if (s.compare(i, 4, L"&gt;") == 0) { out += L'>'; i += 4; }
				else if (s.compare(i, 6, L"&quot;") == 0) { out += L'\"'; i += 6; }
				else if (s.compare(i, 6, L"&apos;") == 0) { out += L'\''; i += 6; }
				else { out += s[i++]; }
			}
			else { out += s[i++]; }
		}
		return out;
	}

	std::wstring ioZaml::Trim(const std::wstring& s)
	{
		size_t start = s.find_first_not_of(L" \t\r\n");
		size_t end = s.find_last_not_of(L" \t\r\n");
		return (start == std::wstring::npos) ? L"" : s.substr(start, end - start + 1);
	}

	std::unordered_map<std::wstring, std::wstring> ioZaml::ParseAttributes(const std::wstring& str)
	{
		std::unordered_map<std::wstring, std::wstring> attrs;
		size_t pos = 0;
		while (pos < str.size())
		{
			while (pos < str.size() && std::isspace(str[pos])) ++pos;
			size_t nameStart = pos;
			while (pos < str.size() && str[pos] != L'=' && !std::isspace(str[pos])) ++pos;
			std::wstring name = str.substr(nameStart, pos - nameStart);
			while (pos < str.size() && (std::isspace(str[pos]) || str[pos] == '=')) ++pos;
			if (pos < str.size() && str[pos] == '"')
			{
				++pos;
				size_t valueStart = pos;
				size_t valueEnd = str.find('"', pos);
				if (valueEnd != std::wstring::npos)
				{
					attrs[name] = UnescapeXML(str.substr(valueStart, valueEnd - valueStart));
					pos = valueEnd + 1;
				}
			}
		}
		return attrs;
	}

	result<zamlNode> ioZaml::ParseNode(std::wistream& in)
	{
		std::wstring line;
		std::wstreampos lastPos;
		while (std::getline(in, line))
		{
			line = Trim(line);
			if (line.empty()) continue;

			if (line.starts_with(L"<") && !line.starts_with(L"</"))
			{
				zamlNode node;
				bool selfClosing = false;
				size_t nameEnd = line.find_first_of(L" />", 1);
				node.name = line.substr(1, nameEnd - 1);
				size_t attrStart = nameEnd;
				size_t tagEnd = line.find('>', attrStart);
				if (tagEnd == std::string::npos)
					return Unexpected(eResult::failure, L"Malformed tag");
				if (line[tagEnd - 1] == '/')
				{
					selfClosing = true;
					tagEnd--;
				}
				node.attributes = ParseAttributes(line.substr(attrStart, tagEnd - attrStart));
				if (selfClosing)
					return node;
				while (true)
				{
					lastPos = in.tellg();
					std::wstring nextLine;
					if (!std::getline(in, nextLine))
						return Unexpected(eResult::failure, L"Unexpected end of input");
					nextLine = Trim(nextLine);
					if (nextLine == L"</" + node.name + L">")
						break;
					in.seekg(lastPos);
					auto resNode = ParseNode(in);
					if (resNode)
						node.children.push_back(resNode.value());
					else
						return resNode.error();
				}
				return node;
			}
		}
		return Unexpected(eResult::failure, L"[ioZaml.ParseNode] Unexpected end");
	}

	result<> ioZaml::WriteNode(std::wostream& out, const zamlNode& node, int indent)
	{
		std::wstring ind(indent, ' ');
		out << ind << L"<" << node.name;
		for (const auto& [k, v] : node.attributes)
			out << ' ' << k << L"=\"" << EscapeXML(v) << L"\"";
		if (node.children.empty())
		{
			out << L" />\n";
			return {};
		}
		out << L">\n";
		for (const auto& child : node.children)
			WriteNode(out, child, indent + 2);
		out << ind << L"</" << node.name << L">\n";
		return {};
	}
}
