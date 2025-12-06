
#include "Easy.h"

vector<int> TwoSum(vector<int>& nums, int target)
{
	unordered_map<int, int> um;

	for (int i = 0; i < nums.size(); i++)
	{
		int v = nums[i];
		int t = target - v;

		if (um.contains(t))
			return {um[t], i};

		um[v] = i;
	}

	return {};
}