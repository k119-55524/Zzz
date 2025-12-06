
#include <stack>
#include "LeetCode/Hard.h"

bool isCloseChar(char in, char out)
{
	switch (in)
	{
	case '(': return out == ')';
	case '{': return out == '}';
	case '[': return out == ']';
	default: return false;
	}
}

bool isValid(string& s)
{
	if (s.length() < 2)
		return false;

	stack<int> st;
	for (int i = 0; i < s.length(); i++)
	{
		char c = s[i];
		if (c == '(' || c == '{' || c == '[')
		{
			st.push(c);
		}
		else
		{
			if (st.empty())
				return false;

			char top = st.top();
			st.pop();
			if (!isCloseChar(top, c))
				return false;
		}
	}

	return st.empty();
}


int main()
{
	string s = "({}{[]})";
	string s1 = "([])";
	string s2 = "([(])";
	string s3 = "((";
	string s4 = "){";
	bool res = isValid(s4);

	int i = 0;
	i++;

	return 0;
}
