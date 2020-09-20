
#define _CRT_SECURE_NO_WARNINGS
#define USE_SIMD
//#include <vld.h>
#include <chrono>

#include <iostream>

#include "clau_parser.h" // need C++17

#include <ctime>


int main(void)
{
	{
		clau_parser::Maker maker;

		clau_parser::UserType* result = maker.NewItem("x", "1")
			.NewUserType("test")
			.NewItem("a", "0")
			.NewItem("b", "1")
			.EndUserType()
			.NewItem("str", "\"wow\"").Get();

		std::cout << result->ToString() << "\n";

		delete result;
	}


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
	/*
	for (int i = 0; i <= 32; ++i) {
		clau_parser::UserType global;

		auto start = std::chrono::steady_clock::now();

		std::cout << i << "th \n";
		clau_parser::LoadData::LoadDataFromFile(fileName, global, i, i);


		auto last = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(last - start);

		std::cout << dur.count() << "ms" << "\n";
	
		//clau_parser::LoadData::SaveWizDB2(global, "output.eu4");
	}
	*/
	for (int i = 0; i < 5; ++i) {
		clau_parser::UserType global;


		auto start = std::chrono::steady_clock::now();

		clau_parser::LoadData::LoadDataFromFile(fileName, global, 0, 0, true);
		auto last = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(last - start);

		std::cout << dur.count() << "ms" << "\n";

		//clau_parser::LoadData::SaveWizDB2(global, "output.eu4");
	}

	for (int i = 0; i < 0; ++i) {
		clau_parser::UserType global;


		auto start = std::chrono::steady_clock::now();

		clau_parser::LoadData::LoadDataFromFile(fileName, global, 0, i);
		auto last = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(last - start);

		std::cout << dur.count() << "ms" << "\n";

		//clau_parser::LoadData::SaveWizDB2(global, "output.eu4");
	}

	return 0;
}
