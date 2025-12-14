
#include <stack>
#include "LeetCode/Hard.h"

int removeElement(vector<int>& nums, int val)
{
	if (nums.size() == 0) return 0;

	int curr = 0;
	int i = 0;
	for (; i < nums.size(); i++)
	{
		if (nums[i] != val)
		{
			nums[curr] = nums[i];
			curr++;
		}
	}

	return curr;
}

int main()
{
	vector<int> nums = { 0 };
	vector<int> nums1 = { 0, 0 };
	vector<int> nums2 = { 0, 0, 1, 1, 1, 2, 2, 3, 3, 4 };
	int res = removeElement(nums2, 2);

	int i = 0;
	i++;

	return 0;
}
