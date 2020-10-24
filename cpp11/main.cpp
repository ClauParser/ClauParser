
#include <chrono>

#include <iostream>

#include "clau_parser_cpp11.h" // need C++11


int main(void)
{

	/*
	{
		clau_parser::Maker maker("AAA");

		maker.NewItem("x", "1")
			.NewGroup("test")
			.NewItem("a", "0")
			.NewItem("b", "1")
			.EndGroup()
			.NewItem("str", "\"wow\"");

		clau_parser::Maker maker2;
		clau_parser::UserType* result = maker2.NewItem("z", "-1")
			.NewGroup(maker)
			.NewItem("abc", "def")
			.Get();

		std::cout << result->ToString() << "\n";

		delete result;
	}

	*/

	/*
	{
		clau_parser::UserType ut("Name");
		clau_parser::ItemType<std::string> it("Name", "Data");

		ut.AddItemType(it);
	}


	{
		clau_parser::UserType ut("Name");
		clau_parser::UserType global("Name");

		global.AddUserTypeItem(ut); // 내부에서 동적할당
	}




	{
		clau_parser::UserType global;
		global.AddItem("date", "1947.5.8");
		global.AddItem("save_game", "\"Russia.eu4\"");
		{
			clau_parser::UserType temp("savegame_versions");
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

	//std::cin >> fileName;
	/*
	for (int i = 0; i <= 32; ++i) {
		clau_parser::UserType global;

		auto start = std::chrono::steady_clock::now();

		std::cout << i << "th \n";
		clau_parser::LoadData::LoadDataFromFile(fileName, global, i, i);


		auto last = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(last - start);

		std::cout << dur.count() << "ms" << "\n";

		//clau_parser::LoadData::Saveclau_parserDB2(global, "output.eu4");
	}
	*/
	for (int i = 0; i < 5; ++i) {
		clau_parser11::UserType global;


		auto start = std::chrono::steady_clock::now();
		clau_parser11::LoadData::LoadDataFromFile(fileName, global, 0, 0, false); // true - error!  fixed..
		auto last = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(last - start);

		std::cout << dur.count() << "ms" << "\n";

		//clau_parser::LoadData::Saveclau_parserDB2(global, "output.eu4");
	}

	for (int i = 0; i < 0; ++i) {
		clau_parser11::UserType global;


		auto start = std::chrono::steady_clock::now();

		clau_parser11::LoadData::LoadDataFromFile(fileName, global, 0, i);
		auto last = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(last - start);

		std::cout << dur.count() << "ms" << "\n";

		//clau_parser::LoadData::Saveclau_parserDB2(global, "output.eu4");
	}

	return 0;
}
