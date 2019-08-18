
#include <iostream>

#include "clau_parser.h"

#include <ctime>


int main(void)
{
	std::string fileName; 

	wiz::UserType global;

	std::cin >> fileName;

	int a = clock();
	wiz::LoadData::LoadDataFromFile(fileName, global, 0, 0);
	int b = clock();

	std::cout << b - a << "ms" << "\n";

	wiz::LoadData::SaveWizDB(global, "output.eu4");

	return 0;
}
