
#include <iostream>

int main()
{
	for (int X = 2; X < 10; X = X + 1)
	{
		for (int Y = 2; Y < 10; Y = Y + 1)
		{
			std::cout << X << " * " << Y << " = " << X * Y << "\n";
		}

		std::cout << "\n";
	}

	return 0;
}