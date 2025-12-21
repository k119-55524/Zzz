
#include <stack>
#include "LeetCode/Hard.h"

int searchInsert(vector<int>& nums, int target)
{
	int left = 0;
	int right = nums.size() - 1;
	while (left <= right)
	{
		int mid = left + (right - left) / 2;
		if (nums[mid] == target)
		{
			return mid;
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

	return left;
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
