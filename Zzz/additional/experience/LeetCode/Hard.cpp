
#include "Hard.h"

//double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2)
//{
//	int l = nums1.size() + nums2.size();
//	int ctr = (l / 2) + 1;
//
//	int i1 = 0, i2 = 0;
//	vector<int> n(ctr);
//	for (int i = 0; i < ctr; i++)
//	{
//		if (i1 >= nums1.size())
//		{
//			n[i] = nums2[i2++];
//		}
//		else
//		{
//			if (i2 >= nums2.size())
//			{
//				n[i] = nums1[i1++];
//			}
//			else
//			{
//				if (nums1[i1] < nums2[i2])
//				{
//					n[i] = nums1[i1++];
//				}
//				else
//				{
//					n[i] = nums2[i2++];
//				}
//			}
//		}
//	}
//
//	if (l % 2 == 1)
//		return n[ctr - 1];
//	else
//		return (n[ctr - 1] + n[ctr - 2]) / 2.0;
//}

//double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
//	// Гарантируем, что nums1 меньше для оптимизации
//	if (nums1.size() > nums2.size())
//		return findMedianSortedArrays(nums2, nums1);
//
//	int s1 = nums1.size();
//	int s2 = nums2.size();
//	int left = 0, right = s1;
//	while (left <= right) {
//		// Разделяем nums1 на partition1
//		int partition1 = (left + right) / 2;
//		// Разделяем nums2 так, чтобы левая часть = правой
//		int partition2 = (s1 + s2 + 1) / 2 - partition1;
//
//		// Границы разделов
//		int maxLeft1 = (partition1 == 0) ? INT_MIN : nums1[partition1 - 1];
//		int minRight1 = (partition1 == s1) ? INT_MAX : nums1[partition1];
//
//		int maxLeft2 = (partition2 == 0) ? INT_MIN : nums2[partition2 - 1];
//		int minRight2 = (partition2 == s2) ? INT_MAX : nums2[partition2];
//
//		// Проверяем правильность разделения
//		if (maxLeft1 <= minRight2 && maxLeft2 <= minRight1) {
//			// Нашли правильное разделение
//			if ((s1 + s2) % 2 == 0) {
//				return (max(maxLeft1, maxLeft2) + min(minRight1, minRight2)) / 2.0;
//			}
//			else {
//				return max(maxLeft1, maxLeft2);
//			}
//		}
//		else if (maxLeft1 > minRight2) {
//			// Слишком много взяли из nums1
//			right = partition1 - 1;
//		}
//		else {
//			// Слишком мало взяли из nums1
//			left = partition1 + 1;
//		}
//	}
//
//	return 0.0;
//}

double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2)
{
	if (nums1.size() > nums2.size())
		return findMedianSortedArrays(nums2, nums1);

	size_t s1 = nums1.size();
	size_t s2 = nums2.size();
	size_t left = 0, right = s1;
	while (left <= right) {
		size_t partition1 = (left + right) / 2;
		size_t partition2 = (s1 + s2 + 1) / 2 - partition1;

		int maxLeft1 = (partition1 == 0) ? numeric_limits<int>::min() : nums1[partition1 - 1];
		int minRight1 = (partition1 == s1) ? numeric_limits<int>::max() : nums1[partition1];
		int maxLeft2 = (partition2 == 0) ? numeric_limits<int>::min() : nums2[partition2 - 1];
		int minRight2 = (partition2 == s2) ? numeric_limits<int>::max() : nums2[partition2];

		if (maxLeft1 <= minRight2 && maxLeft2 <= minRight1)
		{
			if ((s1 + s2) % 2 == 0)
				return (max(maxLeft1, maxLeft2) + min(minRight1, minRight2)) / 2.0;
			else
				return max(maxLeft1, maxLeft2);
		}
		else
		{
			if (maxLeft1 > minRight2)
				right = partition1 - 1;
			else
				left = partition1 + 1;
		}
	}

	return 0.0;
}

int search(vector<int>& nums, int target)
{
	size_t left = 0;
	size_t right = nums.size() - 1;
	while (left <= right)
	{
		size_t c = left + (right - left) / 2;
		if (nums[c] == target)
			return (int)c;

		if (nums[c] < target)
			left = c + 1;
		else
			right = c - 1;
	}

	return -1;
}