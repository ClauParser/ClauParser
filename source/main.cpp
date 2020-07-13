
#define _CRT_SECURE_NO_WARNINGS
//#include <vld.h>

#include <iostream>

#include "clau_parser.h"

#include <ctime>


int main(void)
{
	/*
	{
		wiz::UserType ut("Name");
		wiz::ItemType<std::string> it("Name", "Data");
		
		ut.AddItemType(it);
	}


	{
		wiz::UserType ut("Name");
		wiz::UserType global("Name");

		global.AddUserTypeItem(ut); // 내부에서 동적할당
	}




	{
		wiz::UserType global;
		global.AddItem("date", "1947.5.8");
		global.AddItem("save_game", "\"Russia.eu4\"");
		{
			wiz::UserType temp("savegame_versions");
			temp.AddItem("", "\"1.18.1.0\"");
			temp.AddItem("", "\"1.18.2.0\"");
			temp.AddItem("", "\"1.18.3.0\"");
			temp.AddItem("", "\"1.18.4.0\"");
			temp.AddItem("", "\"1.19.1.0\"");
			global.AddUserTypeItem(temp);
		}

		std::cout << global.ToString();
	}
	*/
	
	std::string fileName = "input.eu4"; 
	
	std::cin >> fileName;

	{
		wiz::UserType global;

		int a = clock();

		wiz::LoadData::LoadDataFromFile(fileName, global, 0, 0, true);

		int b = clock();

		std::cout << b - a << "ms" << "\n";

		//wiz::LoadData::SaveWizDB2(global, "output.eu4");
	}


	{
		wiz::UserType global;

		int a = clock();

		wiz::LoadData::LoadDataFromFile(fileName, global, 0, 0, false);

		int b = clock();

		std::cout << b - a << "ms" << "\n";

		//wiz::LoadData::SaveWizDB2(global, "output.eu4");
	}


	//wiz::LoadData::SaveWizDB2(global, "output.eu4");
	

	return 0;
}
