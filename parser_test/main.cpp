
#include <iostream>
#include "parser.h"

#include <ctime>

int main(void)
{
	wiz::UserType global;
	for (int i = 0; i < 1; ++i) {
		for (int j = 0; j < 1; ++j) {
			global = wiz::UserType();
			int a = clock();
			wiz::LoadData::LoadDataFromFile("input.eu4", global, i, j); // i == 1 : use other algorithm
			int b = clock();
			std::cout << b - a << "ms\n";
		}
	}
	wiz::LoadData::SaveWizDB(global, "output.txt");

	return 0;
}
