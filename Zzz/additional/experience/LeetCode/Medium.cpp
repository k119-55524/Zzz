
#include "Medium.h"

ListNode* addTwoNumbers(ListNode* l1, ListNode* l2)
{
	int v1 = l1->val;
	int v2 = l2->val;
	int carry = 0;
	ListNode* result = new ListNode();
	ListNode* accum = result;
	do
	{
		int curSum = v1 + v2 + carry;
		carry = curSum / 10;
		accum->val = curSum % 10;
 
		l1 = l1 ? l1->next : nullptr;
		v1 = l1 ? l1->val : 0;

		l2 = l2 ? l2->next : nullptr;
		v2 = l2 ? l2->val : 0;

		if (!l1 && !l2 && !carry)
			break;

		accum->next = new ListNode();
		accum = accum->next;
	} while (true);

	return result;
}

int lengthOfLongestSubstring(string s)
{
	if (s.empty())
		return 0;

	unordered_map<char, int> lastIndex;
	int maxLen = 0;
	int left = 0;
	for (int right = 0; right < s.size(); right++) {
		if (lastIndex.contains(s[right]) && lastIndex[s[right]] >= left)
			left = lastIndex[s[right]] + 1;

		lastIndex[s[right]] = right;
		maxLen = max(maxLen, right - left + 1);
	}

	return maxLen;
}