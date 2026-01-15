
#include <stack>
#include "LeetCode/Hard.h"

int searchInsert(vector<int>& nums, int target)
{
	size_t left = 0;
	size_t right = nums.size() - 1;
	while (left <= right)
	{
		size_t mid = left + (right - left) / 2;
		if (nums[mid] == target)
		{
			return (int)mid;
		}
		else if (nums[mid] < target)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	return (int)left;
}

int main()
{
	vector<int> nums = { 0 };
	vector<int> nums1 = { 0, 0 };
	vector<int> nums2 = { 1, 3, 5, 6 };
	auto res = searchInsert(nums2, 2);

	int i = 0;
	i++;

	return 0;
}
