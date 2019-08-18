﻿
#ifndef CLAU_PARSER_H
#define CLAU_PARSER_H

#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <string>

#include <fstream>

#include <algorithm>

#include <thread>
#include <execution>

namespace wiz {
	template <typename T>
	inline T pos_1(const T x, const int base = 10) 
	{
		if (x >= 0) { return x % base; }// x - ( x / 10 ) * 10; }
		else { return (x / base) * base - x; }
		// -( x - ( (x/10) * 10 ) )
	}


	template <typename T> /// T <- char, int, long, long long...
	std::string toStr(const T x) /// chk!!
	{
		const int base = 10;
		if (base < 2 || base > 16) { return "base is not valid"; }
		T i = x;

		const int INT_SIZE = sizeof(T) << 3; ///*8
		char* temp = new char[INT_SIZE + 1 + 1]; /// 1 NULL, 1 minus
		std::string tempString;
		int k;
		bool isMinus = (i < 0);
		temp[INT_SIZE + 1] = '\0'; ///ë¬¸ìžœì‹œ..

		for (k = INT_SIZE; k >= 1; k--) {
			T val = pos_1<T>(i, base); /// 0 ~ base-1
									   /// number to ['0'~'9'] or ['A'~'F']
			if (val < 10) { temp[k] = val + '0'; }
			else { temp[k] = val - 10 + 'A'; }

			i /= base;

			if (0 == i) { // «ìž.
				k--;
				break;
			}
		}

		if (isMinus) {
			temp[k] = '-';
			tempString = std::string(temp + k);//
		}
		else {
			tempString = std::string(temp + k + 1); //
		}
		delete[] temp;

		return tempString;
	}


	class LoadDataOption
	{
	public:
		char LineComment = '#';	// # 
		char Left = '{', Right = '}';	// { }
		char Assignment = '=';	// = 
		char Removal = ',';		// ',' 
	};

	inline bool isWhitespace(const char ch)
	{
		switch (ch)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\v':
		case '\f':
			return true;
			break;
		}
		return false;
	}

	inline int Equal(const char option, const char ch)
	{
		if (ch == option) {
			return 0;
		}
		return -1;
	}

	class Scanner
	{
	private:
		char* start;
		char* last;
	public:
		const wiz::LoadDataOption* option; 
		long long* token_arr;
		long long* token_arr_len;
		long long num;
	public:
		Scanner(char* start, char* last, const wiz::LoadDataOption* option,
			long long* token_arr, long long num, long long* token_arr_len) 
			: start(start), last(last), option(option)
		{
			this->token_arr = token_arr;
			this->num = num;
			this->token_arr_len = token_arr_len;
		}
		~Scanner() {
			//
		}
	public:
		void operator() () {
			long long token_arr_count = 0;
			long long start_idx = 0;
			long long last_idx = 0;
			char* token_first = start;
			char* token_last = start;
			int state = 0;

			long long now_idx = 0;

			for (long long i = 0; start + i < last; ++i, ++now_idx) {
				char* x = start + i;
				long long offset = 0;
				int idx;

				if (0 == state && '\'' == *x) {
					state = 2;

					token_last = x;
					last_idx = now_idx;
				}
				else if (2 == state && '\\' == *(x - 1) && '\'' == *x) {
					token_last = x;
					last_idx = now_idx;
				}
				else if (2 == state && '\'' == *x) {
					state = 0; token_last = x;
					last_idx = now_idx;
				}
				else if (2 == state && '\n' == *x) {
					throw "Error : \n is included in \' ~ \' ";
				}
				else if (0 == state && '\"' == *x) {
					state = 1;

					token_last = x;
					last_idx = now_idx;
				}
				else if (1 == state && '\\' == *(x - 1) && '\"' == *x) {
					token_last = x;
					last_idx = now_idx;
				}
				else if (1 == state && '\n' == *x) {
					throw "error : \n is included.. in \" ~ \" ";
				}
				else if (1 == state && '\"' == *x) {
					state = 0; token_last = x;
					last_idx = now_idx;
				}
				else if (0 == state && -1 != (idx = Equal(option->Removal, *x)))
				{
					token_last = x - 1;
					last_idx = now_idx - 1;

					if (token_last >= 0 && token_last - token_first + 1 > 0) {
						token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
						token_arr_count++;

						{
							if (token_last - token_first + 1 == 1) {
								if (start[start_idx] == option->Left) {
									token_arr[token_arr_count - 1] += 1;
								}
								if (start[start_idx] == option->Right) {
									token_arr[token_arr_count - 1] += 2;
								}
								if (start[start_idx] == option->Assignment) {
									token_arr[token_arr_count - 1] += 3;
								}
							}
						}

						token_first = x + 1;
						start_idx = now_idx + 1;
					}
					else {
						token_first = token_first + 1;
						start_idx = start_idx + 1;
					}
					continue;
				}
				else if (0 == state && -1 != (idx = Equal(option->Assignment, *x))) {
					token_last = x - 1;
					last_idx = now_idx - 1;

					if (token_last >= 0 && token_last - token_first + 1 > 0) {
						token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
						token_arr_count++;

						{
							if (token_last - token_first + 1 == 1) {
								if (start[start_idx] == option->Left) {
									token_arr[token_arr_count - 1] += 1;
								}
								if (start[start_idx] == option->Right) {
									token_arr[token_arr_count - 1] += 2;
								}
								if (start[start_idx] == option->Assignment) {
									token_arr[token_arr_count - 1] += 3;
								}
							}
						}

						token_first = x + 1;
						start_idx = now_idx + 1;
						
						token_arr[token_arr_count] = ((i + num) << 32) + ((1) << 2) + 3;
						token_arr_count++;
					}
					else {
						token_arr[token_arr_count] = ((i + num) << 32) + ((1) << 2) + 3;
						token_arr_count++;

						token_first = token_first + 1;
						start_idx = start_idx + 1;
					}
				}
				else if (0 == state && isWhitespace(*x)) { // isspace ' ' \t \r \n , etc... ?
					token_last = x - 1;
					last_idx = now_idx - 1;
					if (token_last >= 0 && token_last - token_first + 1 > 0) {
						token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
						token_arr_count++;

						{
							if (token_last - token_first + 1 == 1) {
								if (start[start_idx] == option->Left) {
									token_arr[token_arr_count - 1] += 1;
								}
								if (start[start_idx] == option->Right) {
									token_arr[token_arr_count - 1] += 2;
								}
								if (start[start_idx] == option->Assignment) {
									token_arr[token_arr_count - 1] += 3;
								}
							}
						}

						token_first = x + 1;

						start_idx = now_idx + 1;
					}
					else
					{
						token_first = token_first + 1;
						start_idx = start_idx + 1;
					}
				}
				else if (0 == state && -1 != (idx = Equal(option->Left, *x))) {
					token_last = x - 1;
					last_idx = now_idx - 1;
					if (token_last >= 0 && token_last - token_first + 1 > 0) {
						token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
						token_arr_count++;

						{
							if (token_last - token_first + 1 == 1) {
								if (start[start_idx] == option->Left) {
									token_arr[token_arr_count - 1] += 1;
								}
								if (start[start_idx] == option->Right) {
									token_arr[token_arr_count - 1] += 2;
								}
								if (start[start_idx] == option->Assignment) {
									token_arr[token_arr_count - 1] += 3;
								}
							}
						}

						token_first = x + 1;

						start_idx = now_idx + 1;

						token_arr[token_arr_count] = ((i + num) << 32) + ((1) << 2) + 1;

						token_arr_count++;
					}
					else {
						token_arr[token_arr_count] = ((i + num) << 32) + ((1) << 2) + 1;
						token_arr_count++;

						token_first = token_first + 1;
						start_idx = start_idx + 1;
					}
				}
				else if (0 == state && -1 != (idx = Equal(option->Right, *x))) {
					token_last = x - 1;
					last_idx = now_idx - 1;
					if (token_last >= 0 && token_last - token_first + 1 > 0) {
						token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
						token_arr_count++;
						{
							if (token_last - token_first + 1 == 1) {
								if (start[start_idx] == option->Left) {
									token_arr[token_arr_count - 1] += 1;
								}
								if (start[start_idx] == option->Right) {
									token_arr[token_arr_count - 1] += 2;
								}
								if (start[start_idx] == option->Assignment) {
									token_arr[token_arr_count - 1] += 3;
								}
							}
						}

						token_first = x + 1;
						start_idx = now_idx + 1;

						token_arr[token_arr_count] = ((i + num) << 32) + ((1) << 2) + 2;
						token_arr_count++;
					}
					else {
						token_arr[token_arr_count] = ((i + num) << 32) + ((1) << 2) + 2;
						token_arr_count++;

						token_first = token_first + 1;
						start_idx = start_idx + 1;
					}
				}
				else if (0 == state &&
					-1 != Equal(*x, option->LineComment)) {
					token_last = x - 1;
					last_idx = now_idx - 1;
					if (token_last >= 0 && token_last - token_first + 1 > 0) {
						token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
						token_arr_count++;
						{
							if (token_last - token_first + 1 == 1) {
								if (start[start_idx] == option->Left) {
									token_arr[token_arr_count - 1] += 1;
								}
								if (start[start_idx] == option->Right) {
									token_arr[token_arr_count - 1] += 2;
								}
								if (start[start_idx] == option->Assignment) {
									token_arr[token_arr_count - 1] += 3;
								}
							}
						}

						x = token_last + 1;
						now_idx = last_idx + 1;
						token_first = token_last + 1;
						start_idx = last_idx + 1;
						token_last = token_last + 1;

						last_idx = last_idx + 1;
					}

					for (; x <= last; ++x) {
						if (*x == '\n' || *x == '\0') // cf) '\r' ? '\0'?
						{
							break;
						}

						i++;
						now_idx++;
						token_last++;

						last_idx++;
					}

					token_first = x + 1;
					start_idx = now_idx + 1;

					token_last = x + 1;
					last_idx = now_idx + 1;
					continue;
				}
				else {
					//
				}

				token_last = x + offset;

				last_idx = now_idx + offset;

			}

			if (token_first < last)
			{
				if (last - 1 - token_first + 1 > 0) {
					token_arr[token_arr_count] = ((start_idx + num) << 32) + (((long long)token_last - (long long)token_first + 1) << 2) + 0;
					token_arr_count++;

					{
						if (last - 1 - token_first + 1 == 1) {
							if (start[start_idx] == option->Left) {
								token_arr[token_arr_count - 1] += 1;
							}
							if (start[start_idx] == option->Right) {
								token_arr[token_arr_count - 1] += 2;
							}
							if (start[start_idx] == option->Assignment) {
								token_arr[token_arr_count - 1] += 3;
							}
						}
					}
				}
			}

			if (this->token_arr_len) {
				*(this->token_arr_len) = token_arr_count;
			}

			if (state != 0) {
				throw "Scanning Error in qouted string";
			}
		}
	};

	static std::pair<bool, int> Scan(std::ifstream& inFile, const int num, bool* isFirst, const wiz::LoadDataOption& option, int thr_num,
		char*& _buffer, long long* _buffer_len, long long*& _token_arr, long long* _token_arr_len)
	{
		if (inFile.eof()) {
			return { false, 0 };
		}

		std::string temp;
		char* buffer = nullptr;
		std::vector<long long> start(thr_num + 1, 0);
		std::vector<long long> last(thr_num + 1, 0);
		std::vector<long long> token_arr_len(thr_num + 1, 0);
		long long file_length;

		if (thr_num > 0) {
			inFile.seekg(0, inFile.end);
			unsigned long long length = inFile.tellg();
			inFile.seekg(0, inFile.beg);

			file_length = length;
			buffer = new char[file_length + 1]; // 

			// read data as a block:
			inFile.read(buffer, file_length);

			buffer[file_length] = '\0';

			start[0] = 0;


			for (int i = 1; i < thr_num; ++i) {
				start[i] = file_length / thr_num * i;
				for (int x = start[i]; x <= file_length; ++x) {
					if ('\n' == (buffer[x]) || '\0' == buffer[x]) {
						start[i] = x;
						break;
					}
				}
			}
			for (int i = 0; i < thr_num - 1; ++i) {
				last[i] = start[i + 1] - 1;
				for (int x = last[i]; x <= file_length; ++x) {
					if ('\n' == (buffer[x]) || '\0' == buffer[x]) {
						last[i] = x;
						break;
					}
				}
			}
			last[thr_num - 1] = file_length;
		}
		else {
			inFile.seekg(0, inFile.end);
			unsigned long long length = inFile.tellg();
			inFile.seekg(0, inFile.beg);

			file_length = length;
			buffer = new char[file_length + 1]; // 

			// read data as a block:
			inFile.read(buffer, file_length);
			buffer[file_length] = '\0';

			start[0] = 0;
			last[0] = file_length;
		}

		long long* token_arr = nullptr;
		
		if (thr_num > 0) {
			std::vector<std::thread> thr(thr_num);

			token_arr = new long long[file_length];
			
			std::vector<long long> counter(thr_num, 0);
			for (int i = 0; i < thr_num; ++i) {
				thr[i] = std::thread(Scanner(buffer + start[i], buffer + last[i], &option,
					token_arr + start[i], start[i], &counter[i]));
			}

			for (int i = 0; i < thr_num; ++i) {
				thr[i].join();
			}

			long long sum = 0;
			
			for (int i = 1; i < counter.size(); ++i) {
				sum += counter[i - 1];

				memmove(token_arr + sum, token_arr + start[i], counter[i] * sizeof(long long) / sizeof(char));
			}

			*_token_arr_len = sum + counter.back();
		}
		else {
			token_arr = new long long[file_length];

			long long len;
			Scanner scanner(buffer + start[0], buffer + last[0], &option, token_arr, start[0], &len);

			scanner();

			*_token_arr_len = len;
		}

		_buffer = buffer;
		_token_arr = token_arr;
		*_buffer_len = file_length;

		return{ true, 1 };
	}

	class InFileReserver
	{
	private:
		std::ifstream* pInFile;
		bool isFirst;
	public:
		int Num;
	public:
		explicit InFileReserver(std::ifstream& inFile)
		{
			pInFile = &inFile;
			Num = 1;
			isFirst = true;
		}
		bool end()const { return pInFile->eof(); } //
	public:
		bool operator() (const wiz::LoadDataOption& option, int thr_num, char*& buffer, long long* buffer_len, long long*& token_arr, long long* token_arr_len)
		{
			bool x = Scan(*pInFile, Num, &isFirst, option, thr_num, buffer, buffer_len, token_arr, token_arr_len).second > 0;
			return x;
		}
	};

	class Type {
	private:
		std::string name;

		void chk() {
			/*
			if ("_" == name || (String::startsWith(name, "$ut") && name.size() > 3) || (String::startsWith(name, "$it") && name.size() > 3)) {
			std::cout << "name is " << name << std::endl;
			std::cout << "in funciton chk() in Type" << std::endl;
			GETCH();
			throw "ERROR for name in Type";
			}
			*/
		}
	public:
		explicit Type(const std::string& name = "", const bool valid = true) : name(name) { }//chk();  }
		explicit Type(std::string&& name, const bool valid = true) : name(move(name)) { }//chk(); }
		Type(const Type& type)
			: name(type.name)
		{
			//chk();
		}
		virtual ~Type() { }
		bool IsFail() const { // change body
			return "" == name;
		}
		const std::string& GetName()const {
			return name;
		}
		void SetName(const std::string& name)
		{
			this->name = name;

			//chk();
		}
		void SetName(std::string&& name)
		{
			this->name = name;

			//chk();
		}
		bool operator==(const Type& t) const {
			return name == t.name;
		}
		bool operator<(const Type& t) const {
			return name < t.name;
		}
		Type& operator=(const Type& type)
		{
			name = type.name;
			return *this;
		}
		void operator=(Type&& type)
		{
			name = std::move(type.name);
		}
	};

	template < class T >
	class ItemType : public Type {
	public:
		typedef T item_type; //
	private:
		//std::vector<T> arr;
		T data;
		bool inited;
	public:
		ItemType(const ItemType<T>& ta) : Type(ta), data(ta.data), inited(ta.inited)
		{

		}
		ItemType(ItemType<T>&& ta) : Type(std::move(ta))
		{
			data = std::move(ta.data);
			inited = ta.inited;
		}
	public:
		explicit ItemType()
			: Type("", true), inited(false) { }
		explicit ItemType(const std::string& name, const T& value, const bool valid = true)
			:Type(name, valid), data(value), inited(true)
		{

		}
		explicit ItemType(std::string&& name, T&& value, const bool valid = true)
			:Type(move(name), valid), data(move(value)), inited(true)
		{

		}
		virtual ~ItemType() { }
	public:
		void Remove(const int idx = 0)
		{
			data = T();
			inited = false;
		}
		bool Push(const T& val) { /// do not change..!!
			if (inited) { throw "ItemType already inited"; }
			data = val;
			inited = true;

			return true;
		}
		bool Push(T&& val) {
			if (inited) { throw "ItemType already inited"; }
			data = std::move(val);
			inited = true;

			return true;
		}
		T& Get(const int index = 0) {
			if (!inited) {
				throw "ItemType, not inited";
			}
			return data;
		}
		const T& Get(const int index = 0) const {
			if (!inited) {
				throw "ItemType, not inited";
			}
			return data;
		}
		void Set(const int index, const T& val) {
			if (!inited) {
				throw "ItemType, not inited";
			} // removal?
			data = val;
		}
		void Set(const int index, T&& val) {
			if (!inited) {
				throw "ItemType, not inited";
			} // removal?
			data = std::move(val);
		}
		int size()const {
			return inited ? 1 : 0;
		}
		bool empty()const { return !inited; }
		std::string ToString()const
		{
			if (Type::GetName().empty()) {
				return Get(0);
			}
			return Type::GetName() + " = " + Get(0);
		}
	public:
		ItemType<T>& operator=(const ItemType<T>& ta)
		{
			Type::operator=(ta);
			ItemType<T> temp = ta;

			data = std::move(temp.data);
			inited = temp.inited;
			return *this;
		}
		ItemType<T>& operator=(ItemType<T>&& ta)
		{
			Type::operator=(ta);
			if (data == ta.data) { return *this; }

			data = std::move(ta.data);
			inited = ta.inited;
			return *this;
		}
	};

	class UserType : public Type {
	private:
		class UserTypeCompare
		{
		public:
			bool operator() (const UserType* x, const UserType* y) const {
				return x->GetName() < y->GetName();
			}
		};
		class ItemTypeStringPtrCompare {
		public:
			bool operator() (const ItemType<std::string>* x, const ItemType<std::string>* y) const {
				return x->GetName() < y->GetName();
			}
		};
		int binary_find_ut(const std::vector<UserType*>& arr, const UserType& x) const
		{
			if (arr.empty()) { return -1; }

			int left = 0, right = arr.size() - 1;
			int middle = (left + right) / 2;

			while (left <= right) {
				if (arr[middle]->GetName() == x.GetName()) {
					return middle;
				}
				else if (arr[middle]->GetName() < x.GetName()) {
					left = middle + 1;
				}
				else {
					right = middle - 1;
				}

				middle = (left + right) / 2;
			}
			return -1;
		}
		int binary_find_it(const std::vector<ItemType<std::string>*>& arr, const ItemType<std::string>& x) const {
			if (arr.empty()) { return -1; }

			int left = 0, right = arr.size() - 1;
			int middle = (left + right) / 2;

			while (left <= right) {
				if (arr[middle]->GetName() == x.GetName()) {
					return middle;
				}
				else if (arr[middle]->GetName() < x.GetName()) {
					left = middle + 1;
				}
				else {
					right = middle - 1;
				}

				middle = (left + right) / 2;
			}
			return -1;
		}
	public:
		void PushComment(const std::string& comment)
		{
			commentList.push_back(comment);
		}
		void PushComment(std::string&& comment)
		{
			commentList.push_back(move(comment));
		}
		int GetCommentListSize()const { return commentList.size(); }
		const std::string& GetCommentList(const int idx) const { return commentList[idx]; }
		std::string& GetCommentList(const int idx) {
			return commentList[idx];
		}
	public:
		int GetIListSize()const { return ilist.size(); }
		int GetItemListSize()const { return itemList.size(); }
		int GetUserTypeListSize()const { return userTypeList.size(); }
		ItemType<std::string>& GetItemList(const int idx) { return itemList[idx]; }
		const ItemType<std::string>& GetItemList(const int idx) const { return itemList[idx]; }
		UserType*& GetUserTypeList(const int idx) { return userTypeList[idx]; }
		const UserType*& GetUserTypeList(const int idx) const { return const_cast<const UserType * &>(userTypeList[idx]); }

		bool IsItemList(const int idx) const
		{
			return ilist[idx] == 1;
		}
		bool IsUserTypeList(const int idx) const
		{
			return ilist[idx] == 2;
		}

		void AddItemList(const ItemType<std::string>& strTa)
		{
			for (int i = 0; i < strTa.size(); ++i) {
				this->AddItem(strTa.GetName(), strTa.Get(i));
			}
		}
		void AddItemList(ItemType<std::string>&& strTa)
		{
			for (int i = 0; i < strTa.size(); ++i) {
				this->AddItem(std::move(strTa.GetName()), std::move(strTa.Get(i)));
			}
		}
	public:
		void Clear() {
			itemList.clear();
			ilist.clear();
			userTypeList.clear();

			// todo?
		}

		void SetParent(UserType* other)
		{
			parent = other;
		}
		UserType* GetParent() { return parent; }
		const UserType* GetParent()const { return parent; }

		void LinkUserType(UserType* ut) // danger fucntion?
		{
			userTypeList.push_back(ut);
			ilist.push_back(2);
			ut->parent = this;

			useSortedUserTypeList = false;
		}
	private:
		UserType* parent = nullptr;
		std::vector<std::string> commentList;
		std::vector<int> ilist;
		std::vector< ItemType<std::string> > itemList;
		std::vector< UserType* > userTypeList;
		mutable std::vector< ItemType<std::string>* > sortedItemList;
		mutable std::vector< UserType* > sortedUserTypeList;
		mutable bool useSortedItemList = false;
		mutable bool useSortedUserTypeList = false;
		bool noRemove = false;
	public:
		explicit UserType(std::string&& name, bool noRemove = false) : Type(move(name)), parent(nullptr), noRemove(noRemove) { }
		explicit UserType(const std::string& name = "", bool noRemove = false) : Type(name), parent(nullptr), noRemove(noRemove) { } //, userTypeList_sortFlagA(true), userTypeList_sortFlagB(true) { }
		UserType(const UserType& ut) : Type(ut.GetName()) {
			Reset(ut);  // Initial
		}
		UserType(UserType&& ut) : Type(move(ut.GetName())) {
			Reset2(std::move(ut));
		}
		virtual ~UserType() {
			if (false == noRemove) {
				_Remove();
			}
		}
		UserType& operator=(const UserType& ut) {
			if (this == &ut) { return *this; }
			Type::operator=(ut);

			RemoveUserTypeList();
			Reset(ut);
			return *this;
		}
		UserType& operator=(UserType&& ut) {
			if (this == &ut) { return *this; }

			Type::operator=(std::move(ut));
			RemoveUserTypeList();
			Reset2(std::move(ut));
			return *this;
		}
	private:
		void Reset(const UserType& ut) { 
										 //	userTypeList_sortFlagA = ut.userTypeList_sortFlagA;
										 //userTypeList_sortFlagB = ut.userTypeList_sortFlagB;

			ilist = ut.ilist;
			itemList = ut.itemList;
			//parent = ut.parent;
			commentList = ut.commentList;

			//sortedItemList = ut.sortedItemList;
			sortedUserTypeList = ut.sortedUserTypeList;

			useSortedItemList = false; // ut.useSortedItemList;
			useSortedUserTypeList = ut.useSortedUserTypeList;

			noRemove = ut.noRemove;

			userTypeList.reserve(ut.userTypeList.size());

			for (int i = 0; i < ut.userTypeList.size(); ++i) {
				userTypeList.push_back(new UserType(*ut.userTypeList[i]));
				userTypeList.back()->parent = this;
			}
			if (useSortedUserTypeList) {
				sortedUserTypeList.clear();
				for (int i = 0; i < userTypeList.size(); ++i) {
					sortedUserTypeList.push_back(userTypeList[i]);
				}
			}
		}
		void Reset2(UserType&& ut) {
			//std::swap( userTypeList_sortFlagA, ut.userTypeList_sortFlagA );
			//std::swap( userTypeList_sortFlagB, ut.userTypeList_sortFlagB );

			//no use - //parent = ut.parent;
			//no use - //ut.parent = nullptr; /// chk..
			ilist = std::move(ut.ilist);
			itemList = std::move(ut.itemList);
			commentList = std::move(ut.commentList);

			//sortedItemList = std::move(ut.sortedItemList);
			sortedUserTypeList = std::move(ut.sortedUserTypeList);

			std::swap(this->noRemove, ut.noRemove);

			useSortedItemList = false; // ut.useSortedItemList;
			useSortedUserTypeList = ut.useSortedUserTypeList;

			userTypeList.reserve(ut.userTypeList.size());

			for (int i = 0; i < ut.userTypeList.size(); ++i) {
				userTypeList.push_back(std::move(ut.userTypeList[i]));
				ut.userTypeList[i] = nullptr;
				userTypeList.back()->parent = this;
			}
			ut.userTypeList.clear();

			if (useSortedUserTypeList) {
				sortedUserTypeList.clear();
				for (int i = 0; i < userTypeList.size(); ++i) {
					sortedUserTypeList.push_back(userTypeList[i]);
				}
			}
		}

		void _Remove()
		{
			//parent = nullptr;
			ilist = std::vector<int>();
			itemList = std::vector< ItemType<std::string> >();

			sortedItemList.clear();
			sortedUserTypeList.clear();

			useSortedItemList = false;
			useSortedUserTypeList = false;
			RemoveUserTypeList();

			commentList.clear();
		}
		// static ??
	public:
		int GetIlistIndex(const int index, const int type)
		{
			return _GetIlistIndex(ilist, index, type);
		}
		int GetUserTypeIndexFromIlistIndex(const int ilist_idx)
		{
			return _GetUserTypeIndexFromIlistIndex(ilist, ilist_idx);
		}
		int GetItemIndexFromIlistIndex(const int ilist_idx)
		{
			return _GetItemIndexFromIlistIndex(ilist, ilist_idx);
		}
	private:
		/// val : 1 or 2
		int _GetIndex(const std::vector<int>& ilist, const int val, const int start = 0) {
			for (int i = start; i < ilist.size(); ++i) {
				if (ilist[i] == val) { return i; }
			}
			return -1;
		}
		// test? - need more thinking!
		int _GetItemIndexFromIlistIndex(const std::vector<int>& ilist, const int ilist_idx) {
			if (ilist.size() == ilist_idx) { return ilist.size(); }
			int idx = _GetIndex(ilist, 1, 0);
			int item_idx = -1;

			while (idx != -1) {
				item_idx++;
				if (ilist_idx == idx) { return item_idx; }
				idx = _GetIndex(ilist, 1, idx + 1);
			}

			return -1;
		}
		int _GetUserTypeIndexFromIlistIndex(const std::vector<int>& ilist, const int ilist_idx) {
			if (ilist.size() == ilist_idx) { return ilist.size(); }
			int idx = _GetIndex(ilist, 2, 0);
			int usertype_idx = -1;

			while (idx != -1) {
				usertype_idx++;
				if (ilist_idx == idx) { return usertype_idx; }
				idx = _GetIndex(ilist, 2, idx + 1);
			}

			return -1;
		}
		/// type : 1 or 2
		int _GetIlistIndex(const std::vector<int>& ilist, const int index, const int type) {
			int count = -1;

			for (int i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == type) {
					count++;
					if (index == count) {
						return i;
					}
				}
			}
			return -1;
		}
	public:
		void RemoveItemList(const int idx)
		{
			// left shift start idx, to end, at itemList. and resize!
			for (int i = idx + 1; i < GetItemListSize(); ++i) {
				itemList[i - 1] = std::move(itemList[i]);
			}
			itemList.resize(itemList.size() - 1);
			//  ilist left shift and resize - count itemType!
			int count = 0;
			for (int i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 1) { count++; }
				if (count == idx + 1) {
					// iï¿½ï¿½ï¿½ï¿½ left shift!and resize!
					for (int k = i + 1; k < ilist.size(); ++k) {
						ilist[k - 1] = std::move(ilist[k]);
					}
					ilist.resize(ilist.size() - 1);
					break;
				}
			}

			useSortedItemList = false;
		}
		void RemoveUserTypeList(const int idx, const bool chk = true)
		{
			if (chk && userTypeList[idx]) {
				delete userTypeList[idx];
			}
			//std::cout << GetUserTypeListSize() << std::endl;
			// left shift start idx, to end, at itemList. and resize!
			for (int i = idx + 1; i < GetUserTypeListSize(); ++i) {
				userTypeList[i - 1] = std::move(userTypeList[i]);
			}
			userTypeList.resize(userTypeList.size() - 1);
			//  ilist left shift and resize - count itemType!
			int count = 0;
			for (int i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 2) { count++; }
				if (count == idx + 1) {
					// iï¿½ï¿½ï¿½ï¿½ left shift!and resize!
					for (int k = i + 1; k < ilist.size(); ++k) {
						ilist[k - 1] = std::move(ilist[k]);
					}
					ilist.resize(ilist.size() - 1);
					break;
				}
			}

			useSortedUserTypeList = false;
		}
		void RemoveItemList(const std::string& varName)
		{
			int k = _GetIndex(ilist, 1, 0);
			std::vector<ItemType<std::string>> tempDic;
			for (int i = 0; i < itemList.size(); ++i) {
				if (varName != itemList[i].GetName()) {
					tempDic.push_back(itemList[i]);
					k = _GetIndex(ilist, 1, k + 1);
				}
				else {
					// remove item, ilist left shift 1.
					for (int j = k + 1; j < ilist.size(); ++j) {
						ilist[j - 1] = ilist[j];
					}
					ilist.resize(ilist.size() - 1);
					k = _GetIndex(ilist, 1, k);
				}
			}
			itemList = std::move(tempDic);

			useSortedItemList = false;
		}
		void RemoveItemList() /// ALL
		{
			itemList = std::vector<ItemType<std::string>>();
			//
			std::vector<int> temp;
			for (int i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 2)
				{
					temp.push_back(2);
				}
			}
			ilist = move(temp);

			useSortedItemList = false;
		}
		void RemoveEmptyItem() // fixed..
		{
			int k = _GetIndex(ilist, 1, 0);
			std::vector<ItemType<std::string>> tempDic;
			for (int i = 0; i < itemList.size(); ++i) {
				if (itemList[i].size() > 0) {
					tempDic.push_back(itemList[i]);
					k = _GetIndex(ilist, 1, k + 1);
				}
				else {
					// remove item, ilist left shift 1.
					for (int j = k + 1; j < ilist.size(); ++j) {
						ilist[j - 1] = ilist[j];
					}
					ilist.resize(ilist.size() - 1);
					k = _GetIndex(ilist, 1, k);
				}
			}
			itemList = move(tempDic);

			useSortedItemList = false;
		}
		void Remove()
		{
			/// parent->removeUserType(name); - ToDo - X
			ilist = std::vector<int>();
			itemList = std::vector< ItemType<std::string> >();

			RemoveUserTypeList();

			commentList.clear();

			sortedItemList.clear();
			sortedUserTypeList.clear();

			useSortedItemList = false;
			useSortedUserTypeList = false;
			//parent = nullptr;
		}
		void RemoveUserTypeList() { /// chk memory leak test!!
			for (int i = 0; i < userTypeList.size(); i++) {
				if (nullptr != userTypeList[i]) {
					delete userTypeList[i]; //
					userTypeList[i] = nullptr;
				}
			}
			// DO Empty..
			userTypeList.clear();

			std::vector<int> temp;
			for (int i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 1)
				{
					temp.push_back(1);
				}
			}
			ilist = move(temp);

			useSortedUserTypeList = false;
		}
		void RemoveUserTypeList(const std::string& varName, const bool chk = true)
		{
			int k = _GetIndex(ilist, 2, 0);
			std::vector<UserType*> tempDic;
			for (int i = 0; i < userTypeList.size(); ++i) {
				if (varName != userTypeList[i]->GetName()) {
					tempDic.push_back(userTypeList[i]);
					k = _GetIndex(ilist, 2, k + 1);
				}
				else {
					if (chk && userTypeList[i]) {
						delete userTypeList[i];
					}
					// remove usertypeitem, ilist left shift 1.
					for (int j = k + 1; j < ilist.size(); ++j) {
						ilist[j - 1] = ilist[j];
					}
					ilist.resize(ilist.size() - 1);
					k = _GetIndex(ilist, 2, k);
				}
			}
			userTypeList = move(tempDic);

			useSortedUserTypeList = false;
		}
		//			
		void RemoveList(const int idx) // ilist_idx!
		{
			// chk whether item or usertype.
			// find item_idx or usertype_idx.
			// remove item or remove usertype.
			if (ilist[idx] == 1) {
				int item_idx = -1;

				for (int i = 0; i < ilist.size() && i <= idx; ++i) {
					if (ilist[i] == 1) { item_idx++; }
				}

				RemoveItemList(item_idx);
			}
			else {
				int usertype_idx = -1;

				for (int i = 0; i < ilist.size() && i <= idx; ++i) {
					if (ilist[i] == 2) { usertype_idx++; }
				}

				RemoveUserTypeList(usertype_idx);
			}
		}
	public:
		bool empty()const { return ilist.empty(); }

		// chk
		void InsertItemByIlist(const int ilist_idx, const std::string& name, const std::string& item) {
			ilist.push_back(1);
			for (int i = ilist.size() - 1; i > ilist_idx; --i) {
				ilist[i] = ilist[i - 1];
			}
			ilist[ilist_idx] = 1;


			int itemIndex = _GetItemIndexFromIlistIndex(ilist, ilist_idx);

			itemList.emplace_back("", std::string(""));

			if (itemIndex != -1) {
				for (int i = itemList.size() - 1; i > itemIndex; --i) {
					itemList[i] = move(itemList[i - 1]);
				}
				itemList[itemIndex] = ItemType<std::string>(name, item);
			}
			else {
				itemList[0] = ItemType<std::string>(name, item); // chk!!
			}

			useSortedItemList = false;
		}
		void InsertItemByIlist(const int ilist_idx, std::string&& name, std::string&& item) {
			ilist.push_back(1);


			for (int i = ilist.size() - 1; i > ilist_idx; --i) {
				ilist[i] = ilist[i - 1];
			}
			ilist[ilist_idx] = 1;

			int itemIndex = _GetItemIndexFromIlistIndex(ilist, ilist_idx);

			itemList.emplace_back("", std::string(""));
			if (itemIndex != -1) {
				for (int i = itemList.size() - 1; i > itemIndex; --i) {
					itemList[i] = move(itemList[i - 1]);
				}
				itemList[itemIndex] = ItemType<std::string>(move(name), move(item));
			}
			else {
				itemList[0] = ItemType<std::string>(move(name), move(item));
			}


			useSortedItemList = false;
		}
		// chk
		void InsertUserTypeByIlist(const int ilist_idx, UserType&& item) {
			ilist.push_back(2);
			UserType* temp = new UserType(std::move(item));

			temp->parent = this;

			for (int i = ilist.size() - 1; i > ilist_idx; --i) {
				ilist[i] = ilist[i - 1];
			}
			ilist[ilist_idx] = 2;

			int userTypeIndex = _GetUserTypeIndexFromIlistIndex(ilist, ilist_idx);
			userTypeList.push_back(nullptr);
			if (userTypeIndex != -1) {
				for (int i = userTypeList.size() - 1; i > userTypeIndex; --i) {
					userTypeList[i] = std::move(userTypeList[i - 1]);
				}
				userTypeList[userTypeIndex] = temp;
			}
			else {
				userTypeList[0] = temp;
			}


			useSortedUserTypeList = false;
		}
		void InsertUserTypeByIlist(const int ilist_idx, const UserType& item) {
			ilist.push_back(2);
			UserType* temp = new UserType(item);

			temp->parent = this;


			for (int i = ilist.size() - 1; i > ilist_idx; --i) {
				ilist[i] = ilist[i - 1];
			}
			ilist[ilist_idx] = 2;

			int userTypeIndex = _GetUserTypeIndexFromIlistIndex(ilist, ilist_idx);
			userTypeList.push_back(nullptr);
			if (userTypeIndex != -1) {
				for (int i = userTypeList.size() - 1; i > userTypeIndex; --i) {
					userTypeList[i] = std::move(userTypeList[i - 1]);
				}
				userTypeList[userTypeIndex] = temp;
			}
			else {
				userTypeList[0] = temp;
			}

			useSortedUserTypeList = false;
		}

		/*
		// chk
		void InsertItem(const int item_idx, const std::string& name, const std::string& item) {
		int ilist_idx = _GetIlistIndex(ilist, item_idx, 1);

		ilist.push_back(0);
		for (int i = ilist_idx + 1; i < ilist.size(); ++i) {
		ilist[i] = ilist[i - 1];
		}
		ilist[ilist_idx] = 1;

		itemList.emplace_back("", "");
		for (int i = item_idx + 1; i < itemList.size(); ++i) {
		itemList[i] = move(itemList[i - 1]);
		}
		itemList[item_idx] = ItemType<std::string>(name, item);
		}
		void InsertItem(const int item_idx, std::string&& name, std::string&& item) {
		int ilist_idx = _GetIlistIndex(ilist, item_idx, 1);

		ilist.push_back(0);
		for (int i = ilist_idx + 1; i < ilist.size(); ++i) {
		ilist[i] = ilist[i - 1];
		}
		ilist[ilist_idx] = 1;

		itemList.emplace_back("", "");
		for (int i = item_idx + 1; i < itemList.size(); ++i) {
		itemList[i] = move(itemList[i - 1]);
		}
		itemList[item_idx] = ItemType<std::string>(move(name), move(item));
		}
		// chk
		void InsertUserType(const int ut_idx, UserType&& item) {
		int ilist_idx = _GetIlistIndex(ilist, ut_idx, 2);
		UserType* temp = new UserType(std::move(item));

		temp->parent = this;

		ilist.push_back(0);
		for (int i = ilist_idx + 1; i < ilist.size(); ++i) {
		ilist[i] = ilist[i - 1];
		}
		ilist[ilist_idx] = 2;

		userTypeList.push_back(nullptr);
		for (int i = ut_idx + 1; i < userTypeList.size(); ++i) {
		userTypeList[i] = userTypeList[i - 1];
		}
		userTypeList[ut_idx] = temp;
		}
		void InsertUserType(const int ut_idx, const UserType& item) {
		int ilist_idx = _GetIlistIndex(ilist, ut_idx, 2);
		UserType* temp = new UserType(item);

		temp->parent = this;

		ilist.push_back(0);
		for (int i = ilist_idx + 1; i < ilist.size(); ++i) {
		ilist[i] = ilist[i - 1];
		}
		ilist[ilist_idx] = 2;

		userTypeList.push_back(nullptr);
		for (int i = ut_idx + 1; i < userTypeList.size(); ++i) {
		userTypeList[i] = userTypeList[i - 1];
		}
		userTypeList[ut_idx] = temp;
		}
		*/
		//
		void ReserveIList(int offset)
		{
			if (offset > 0) {
				ilist.reserve(offset);
			}
		}
		void ReserveItemList(int offset)
		{
			if (offset > 0) {
				itemList.reserve(offset);
			}
		}
		void ReserveUserTypeList(int offset)
		{
			if (offset > 0) {
				userTypeList.reserve(offset);
			}
		}
		void AddItem(std::string&& name, std::string&& item) {
			itemList.emplace_back(move(name), move(item));
			ilist.push_back(1);

			useSortedItemList = false;
		}
		void AddItem(const std::string& name, const std::string& item) {
			itemList.emplace_back(name, item);
			ilist.push_back(1);

			useSortedItemList = false;
		}
		void AddItem(std::vector<std::string>&& name, std::vector<std::string>&& item, const int n) {
			// name.size() == item.size()
			int start_idx = itemList.size();
			itemList.reserve(itemList.size() + n);
			ilist.reserve(ilist.size() + n);
			int end_idx = itemList.size() + n;

			{
				for (int i = start_idx; i < end_idx; ++i) {
					itemList.push_back(ItemType<std::string>(std::move(name[i - start_idx]), std::move(item[i - start_idx])));
					ilist.push_back(1);
				}
			}
		}
		void AddItem(const std::vector<std::string>& name, const std::vector<std::string>& item, const int n) {
			// name.size() == item.size()
			int start_idx = itemList.size();
			itemList.reserve(itemList.size() + n);
			ilist.reserve(ilist.size() + n);
			int end_idx = itemList.size() + n;

			{
				for (int i = start_idx; i < end_idx; ++i) {
					itemList.push_back(ItemType<std::string>((name[i - start_idx]), (item[i - start_idx])));
					ilist.push_back(1);
				}
			}
		}

		void AddUserTypeItem(UserType&& item) {
			UserType* temp = new UserType(std::move(item));
			temp->parent = this;
			//temp->SetName("");

			ilist.push_back(2);

			userTypeList.push_back(temp);

			useSortedUserTypeList = false;
		}
		void AddUserTypeItem(const UserType& item) {
			UserType* temp = new UserType(item);
			temp->parent = this;

			ilist.push_back(2);

			userTypeList.push_back(temp);

			useSortedUserTypeList = false;
		}
		void AddItemAtFront(std::string&& name, std::string&& item) {
			itemList.emplace(itemList.begin(), name, item);

			ilist.insert(ilist.begin(), 1);

			useSortedItemList = false;
		}
		void AddItemAtFront(const std::string& name, const std::string& item) {
			itemList.emplace(itemList.begin(), name, item);

			ilist.insert(ilist.begin(), 1);

			useSortedItemList = false;
		}
		void AddUserTypeItemAtFront(const UserType& item) {
			UserType* temp = new UserType(item);
			temp->parent = this;

			ilist.insert(ilist.begin(), 2);

			userTypeList.insert(userTypeList.begin(), temp);

			useSortedUserTypeList = false;
		}
		void AddUserTypeItemAtFront(UserType&& item) {
			UserType* temp = new UserType(item);
			temp->parent = this;

			ilist.insert(ilist.begin(), 2);

			userTypeList.insert(userTypeList.begin(), temp);

			useSortedUserTypeList = false;
		}


		// $it?
		std::vector<ItemType<std::string>> GetItem(const std::string& name) const {
			std::vector<ItemType<std::string>> temp;
			/*if (String::startsWith(name, "$.") && name.size() >= 5) {
				// later, change to binary search?
				std::string str = name.substr(3, name.size() - 4);
				std::regex rgx(str);

				for (int i = 0; i < itemList.size(); ++i) {
					if (regex_match(itemList[i].GetName(), rgx)) {
						temp.push_back(itemList[i]);
					}
				}
			}
			else*/
			{
				if (false == useSortedItemList) {
					sortedItemList.clear();
					for (int i = 0; i < itemList.size(); ++i) {
						sortedItemList.push_back((ItemType<std::string>*) & itemList[i]);
					}

					std::sort(sortedItemList.begin(), sortedItemList.end(), ItemTypeStringPtrCompare());

					useSortedItemList = true;
				}
				// binary search
				{
					ItemType<std::string> x = ItemType<std::string>(name, "");
					int idx = binary_find_it(sortedItemList, x);
					if (idx >= 0) {
						int start = idx;
						int last = idx;

						for (int i = idx - 1; i >= 0; --i) {
							if (name == sortedItemList[i]->GetName()) {
								start--;
							}
							else {
								break;
							}
						}
						for (int i = idx + 1; i < sortedItemList.size(); ++i) {
							if (name == sortedItemList[i]->GetName()) {
								last++;
							}
							else {
								break;
							}
						}

						for (int i = start; i <= last; ++i) {
							temp.push_back(*sortedItemList[i]);
						}
					}
					else {
						//std::cout << "no found" << std::endl;
					}
				}
			}
			return temp;
		}
		// regex to SetItem?
		bool SetItem(const std::string& name, const std::string& value) {
			int index = -1;

			for (int i = 0; i < itemList.size(); ++i) {
				if (itemList[i].GetName() == name)
				{
					itemList[i].Set(0, value);
					index = i;
				}
			}

			return -1 != index;
		}
		/// add set Data
		bool SetItem(const int var_idx, const std::string& value) {
			itemList[var_idx].Set(0, value);
			return true;
		}

		//// O(N) -> O(logN)?
		std::vector<UserType*> GetUserTypeItem(const std::string& name) const { /// chk...
			std::vector<UserType*> temp;

			if (false == useSortedUserTypeList) {
				// make sortedUserTypeList.
				sortedUserTypeList = userTypeList;

				std::sort(sortedUserTypeList.begin(), sortedUserTypeList.end(), UserTypeCompare());

				useSortedUserTypeList = true;
			}
			// binary search
			{
				UserType x = UserType(name);
				int idx = binary_find_ut(sortedUserTypeList, x);
				if (idx >= 0) {
					int start = idx;
					int last = idx;

					for (int i = idx - 1; i >= 0; --i) {
						if (name == sortedUserTypeList[i]->GetName()) {
							start--;
						}
						else {
							break;
						}
					}
					for (int i = idx + 1; i < sortedUserTypeList.size(); ++i) {
						if (name == sortedUserTypeList[i]->GetName()) {
							last++;
						}
						else {
							break;
						}
					}

					for (int i = start; i <= last; ++i) {
						temp.push_back(sortedUserTypeList[i]);
					}
				}
				else {
					//std::cout << "no found" << std::endl;
				}
			}

			/*
			for (int i = 0; i < userTypeList.size(); ++i) {
			if (userTypeList[i]->GetName() == name) {
			temp.push_back(userTypeList[i]);
			}
			}
			*/
			return temp;
		}

		// deep copy.
		std::vector<UserType*> GetCopyUserTypeItem(const std::string& name) const { /// chk...
			std::vector<UserType*> temp;

			if (false == useSortedUserTypeList) {
				// make sortedUserTypeList.
				sortedUserTypeList = userTypeList;

				std::sort(sortedUserTypeList.begin(), sortedUserTypeList.end(), UserTypeCompare());

				useSortedUserTypeList = true;
			}
			// binary search
			{
				UserType x = UserType(name);
				int idx = binary_find_ut(sortedUserTypeList, x);
				if (idx >= 0) {
					int start = idx;
					int last = idx;

					for (int i = idx - 1; i >= 0; --i) {
						if (name == sortedUserTypeList[i]->GetName()) {
							start--;
						}
						else {
							break;
						}
					}
					for (int i = idx + 1; i < sortedUserTypeList.size(); ++i) {
						if (name == sortedUserTypeList[i]->GetName()) {
							last++;
						}
						else {
							break;
						}
					}

					for (int i = start; i <= last; ++i) {
						temp.push_back(new UserType(*sortedUserTypeList[i]));
					}
				}
				else {
					//std::cout << "no found" << std::endl;
				}
			}

			/*
			for (int i = 0; i < userTypeList.size(); ++i) {
			if (userTypeList[i]->GetName() == name) {
			temp.push_back(new UserType(*userTypeList[i]));
			}
			}
			*/
			return temp;
		}
	public:
		bool GetUserTypeItemRef(const int idx, UserType*& ref)
		{
			ref = userTypeList[idx];
			return true;
		}
		bool GetLastUserTypeItemRef(const std::string& name, UserType*& ref) {
			int idx = -1;

			for (int i = userTypeList.size() - 1; i >= 0; --i)
			{
				if (name == userTypeList[i]->GetName()) {
					idx = i;
					break;
				}
			}
			if (idx > -1) {
				ref = userTypeList[idx];
			}
			return idx > -1;
		}
	private:
		/// save1 - like EU4 savefiles.
		void Save1(std::ostream& stream, const UserType* ut, const int depth = 0) const {
			int itemListCount = 0;
			int userTypeListCount = 0;

			const bool existUserType = ut->GetUserTypeListSize() > 0;

			for (int i = 0; i < ut->commentList.size(); ++i) {
				for (int k = 0; k < depth; ++k) {
					stream << "\t";
				}
				stream << (ut->commentList[i]);

				if (i < ut->commentList.size() - 1 || false == ut->ilist.empty()) {
					stream << "\n";
				}
			}

			for (int i = 0; i < ut->ilist.size(); ++i) {
				//std::cout << "ItemList" << endl;
				if (ut->ilist[i] == 1) {
					for (int j = 0; j < ut->itemList[itemListCount].size(); j++) {
						std::string temp;
						if (existUserType) {
							for (int k = 0; k < depth; ++k) {
								temp += "\t";
							}
						}
						if (ut->itemList[itemListCount].GetName() != "") {
							temp += ut->itemList[itemListCount].GetName();
							temp += "=";
						}
						temp += ut->itemList[itemListCount].Get(j);
						if (j != ut->itemList[itemListCount].size() - 1) {
							temp += " ";
						}
						stream << temp;
					}
					if (i != ut->ilist.size() - 1) {
						if (existUserType) {
							stream << "\n";
						}
						else {
							stream << " "; // \n
						}
					}
					itemListCount++;
				}
				else if (ut->ilist[i] == 2) {
					// std::cout << "UserTypeList" << endl;
					for (int k = 0; k < depth; ++k) {
						stream << "\t";
					}

					if (ut->userTypeList[userTypeListCount]->GetName() != "") {
						stream << ut->userTypeList[userTypeListCount]->GetName() << "=";
					}

					stream << "{\n";

					Save1(stream, ut->userTypeList[userTypeListCount], depth + 1);
					stream << "\n";

					for (int k = 0; k < depth; ++k) {
						stream << "\t";
					}
					stream << "}";
					if (i != ut->ilist.size() - 1) {
						stream << "\n";
					}

					userTypeListCount++;
				}
			}
		}
		/// save2 - for more speed loading data!?
		void Save2(std::ostream& stream, const UserType* ut, const int depth = 0) const {
			int itemListCount = 0;
			int userTypeListCount = 0;

			for (int i = 0; i < ut->commentList.size(); ++i) {
				for (int k = 0; k < depth; ++k) {
					stream << "\t";
				}
				stream << (ut->commentList[i]);

				if (i < ut->commentList.size() - 1 || false == ut->ilist.empty()) {
					stream << "\n";
				}

			}
			for (int i = 0; i < ut->ilist.size(); ++i) {
				//std::cout << "ItemList" << endl;
				if (ut->ilist[i] == 1) {
					for (int j = 0; j < ut->itemList[itemListCount].size(); j++) {
						//for (int k = 0; k < depth; ++k) {
						//	stream << "\t";
						//}
						if (ut->itemList[itemListCount].GetName() != "")
							stream << ut->itemList[itemListCount].GetName() << " = ";
						stream << ut->itemList[itemListCount].Get(j);
						if (j != ut->itemList[itemListCount].size() - 1)
							stream << " ";
					}
					if (i != ut->ilist.size() - 1) {
						stream << " ";//"\n";
					}
					itemListCount++;
				}
				else if (ut->ilist[i] == 2) {
					// std::cout << "UserTypeList" << endl;
					if (ut->userTypeList[userTypeListCount]->GetName() != "")
					{
						stream << ut->userTypeList[userTypeListCount]->GetName() << " = ";
					}
					stream << "{\n";

					Save2(stream, ut->userTypeList[userTypeListCount], depth + 1);
					stream << "\n";

					for (int k = 0; k < depth; ++k) {
						stream << "\t";
					}
					stream << "}";
					if (i != ut->ilist.size() - 1) {
						stream << "\n";
					}
					userTypeListCount++;
				}
			}
		}

	public:
		void Save1(std::ostream& stream, int depth = 0) const {
			Save1(stream, this, depth);
		}

		std::string ItemListToString()const
		{
			std::string temp;
			int itemListCount = 0;

			for (int i = 0; i < itemList.size(); ++i) {
				for (int j = 0; j < itemList[itemListCount].size(); j++) {
					if (itemList[itemListCount].GetName() != "")
						temp = temp + itemList[itemListCount].GetName() + " = ";
					temp = temp + itemList[itemListCount].Get(j);
					if (j != itemList[itemListCount].size() - 1) {
						temp = temp + "/";
					}
				}
				if (i != itemList.size() - 1)
				{
					temp = temp + "/";
				}
				itemListCount++;
			}
			return temp;
		}
		std::string ItemListNamesToString()const
		{
			std::string temp;
			int itemListCount = 0;

			for (int i = 0; i < itemList.size(); ++i) {
				for (int j = 0; j < itemList[itemListCount].size(); j++) {
					if (itemList[itemListCount].GetName() != "")
						temp = temp + itemList[itemListCount].GetName();
					else
						temp = temp + " ";

					if (j != itemList[itemListCount].size() - 1) {
						temp = temp + "/";
					}
				}
				if (i != itemList.size() - 1)
				{
					temp = temp + "/";
				}
				itemListCount++;
			}
			return temp;
		}
		std::vector<std::string> userTypeListNamesToStringArray()const
		{
			std::vector<std::string> temp;
			int userTypeListCount = 0;

			for (int i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[userTypeListCount]->GetName() != "") {
					temp.push_back(userTypeList[userTypeListCount]->GetName());
				}
				else {
					temp.push_back(" "); // chk!! cf) wiz::load_data::Utility::Find function...
				}
				userTypeListCount++;
			}
			return temp;
		}
		std::string UserTypeListNamesToString()const
		{
			std::string temp;
			int userTypeListCount = 0;

			for (int i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[userTypeListCount]->GetName() != "") {
					temp = temp + userTypeList[userTypeListCount]->GetName();
				}
				else {
					temp = temp + " "; // chk!! cf) wiz::load_data::Utility::Find function...
				}

				if (i != itemList.size() - 1)
				{
					temp = temp + "/";
				}
				userTypeListCount++;
			}
			return temp;
		}
		std::string ToString()const
		{
			std::string temp;
			int itemListCount = 0;
			int userTypeListCount = 0;

			for (int i = 0; i < ilist.size(); ++i) {
				//std::cout << "ItemList" << endl;
				if (ilist[i] == 1) {
					for (int j = 0; j < itemList[itemListCount].size(); j++) {
						if (itemList[itemListCount].GetName() != "") {
							temp.append(itemList[itemListCount].GetName());
							temp.append(" = ");
						}
						temp.append(itemList[itemListCount].Get(j));
						if (j != itemList[itemListCount].size() - 1)
						{
							temp.append(" ");
						}
					}
					if (i != ilist.size() - 1) {
						temp.append(" ");
					}
					itemListCount++;
				}
				else if (ilist[i] == 2) {
					// std::cout << "UserTypeList" << endl;
					if (userTypeList[userTypeListCount]->GetName() != "") {
						temp.append(userTypeList[userTypeListCount]->GetName());
						temp.append(" = ");
					}
					temp.append(" { ");
					temp.append(userTypeList[userTypeListCount]->ToString());
					temp.append(" ");
					temp.append(" }");
					if (i != ilist.size() - 1) {
						temp.append(" ");
					}

					userTypeListCount++;
				}
			}
			return temp;
		}
	public:
	};

	// LoadData
	class LoadData
	{
	private:
		static std::string& check_syntax_error1(std::string& str, const wiz::LoadDataOption& opt) {
			if (1 == str.size() && (opt.Left == str[0] || opt.Right == str[0] ||
				opt.Assignment == str[0])) {
				throw "check syntax error 1 : " + str;
			}
			return str;
		}
		static int Merge(UserType* next, UserType* ut, UserType** ut_next)
		{
			//check!!
			while (ut->GetIListSize() >= 1 && ut->GetUserTypeListSize() >= 1
				&& (ut->GetUserTypeList(0)->GetName() == "#"))
			{
				ut = ut->GetUserTypeList(0);
			}

			//int chk = 0;
			bool chk_ut_next = false;

			while (true) {
				int itCount = 0;
				int utCount = 0;

				UserType* _ut = ut;
				UserType* _next = next;


				if (ut_next && _ut == *ut_next) {
					*ut_next = _next;
					//std::cout << "_ut == *ut_next" << "\n";
					chk_ut_next = true;
				}

				for (int i = 0; i < _ut->GetIListSize(); ++i) {
					if (_ut->IsUserTypeList(i)) {
						if (_ut->GetUserTypeList(utCount)->GetName() == "#") {
							_ut->GetUserTypeList(utCount)->SetName("");
						}
						else {
							{
								_next->LinkUserType(_ut->GetUserTypeList(utCount));
								_ut->GetUserTypeList(utCount) = nullptr;
							}
						}
						utCount++;
					}
					else if (_ut->IsItemList(i)) {
						_next->AddItemList(std::move(_ut->GetItemList(itCount)));
						itCount++;
					}
				}
				_ut->Remove();

				ut = ut->GetParent();
				next = next->GetParent();


				if (next && ut) {
					//
				}
				else {
					// right_depth > left_depth
					if (!next && ut) {
						return -1;
					}
					else if (next && !ut) {
						return 1;
					}

					return 0;
				}
			}
		}

	public:
		static long long GetIdx(long long x) {
			return (x >> 32) & 0x00000000FFFFFFFF;
		}
		static long long GetLength(long long x) {
			return (x & 0x00000000FFFFFFFC) >> 2;
		}
		static long long GetType(long long x) {
			return x & 3; // % 4
		}
	private:
		static bool __LoadData(const char* buffer, const long long* token_arr, long long token_arr_len, UserType* _global, const wiz::LoadDataOption* _option,
			int start_state, int last_state, UserType** next) // first, strVec.empty() must be true!!
		{
			std::vector<std::string> varVec;
			std::vector<std::string> valVec;


			if (token_arr_len <= 0) {
				return false;
			}

			UserType& global = *_global;
			const wiz::LoadDataOption& option = *_option;

			int state = start_state;
			int braceNum = 0;
			std::vector< UserType* > nestedUT(1);
			std::string var, val;

			nestedUT.reserve(10);
			nestedUT[0] = &global;


			long long count = 0;
			const long long* x = token_arr;
			const long long* x_next = x;

			for (long long i = 0; i < token_arr_len; ++i) {
				x = x_next;
				{
					x_next = x + 1;
				}
				if (count > 0) {
					count--;
					continue;
				}
				long long len = GetLength(token_arr[i]);

				switch (state)
				{
				case 0:
				{
					// Left 1
					if (len == 1 && (-1 != Equal(1, GetType(token_arr[i])) || -1 != Equal(1, GetType(token_arr[i])))) {
						//i += 1;

						if (!varVec.empty()) {

							nestedUT[braceNum]->AddItem((varVec), (valVec), varVec.size());

							varVec.clear();
							valVec.clear();
						}

						UserType temp("");

						nestedUT[braceNum]->AddUserTypeItem(temp);
						UserType* pTemp = nullptr;
						nestedUT[braceNum]->GetLastUserTypeItemRef("", pTemp);

						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) { /// changed 2014.01.23..
							nestedUT.push_back(nullptr);
						}

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;
						///

						state = 0;

					}
					// Right 2
					else if (len == 1 && (-1 != Equal(2, GetType(token_arr[i])) || -1 != Equal(2, GetType(token_arr[i])))) {
						state = 0;

						if (!varVec.empty()) {

							{
								nestedUT[braceNum]->AddItem(varVec, valVec, varVec.size());
							}


							varVec.clear();
							valVec.clear();
						}


						if (braceNum == 0) {
							UserType ut;
							ut.AddUserTypeItem(UserType("#")); // json -> "var_name" = val  // clautext, # is line comment delimiter.
							UserType* pTemp = nullptr;
							ut.GetLastUserTypeItemRef("#", pTemp);
							int utCount = 0;
							int itCount = 0;
							auto max = nestedUT[braceNum]->GetIListSize();
							for (auto i = 0; i < max; ++i) {
								if (nestedUT[braceNum]->IsUserTypeList(i)) {
									ut.GetUserTypeList(0)->AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
									utCount++;
								}
								else {
									ut.GetUserTypeList(0)->AddItemList(std::move(nestedUT[braceNum]->GetItemList(itCount)));
									itCount++;
								}
							}

							nestedUT[braceNum]->Remove();
							nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

							braceNum++;
						}

						{
							if (braceNum < nestedUT.size()) {
								nestedUT[braceNum] = nullptr;
							}
							braceNum--;
						}
					}
					else {
						if (x < token_arr + token_arr_len - 1) {
							long long _len = GetLength(token_arr[i + 1]);
							// EQ 3
							if (_len == 1 && -1 != Equal(3, GetType(token_arr[i + 1]))) { 
								var = std::string(buffer + GetIdx(token_arr[i]), len);

								state = 1;

								{
									count = 1;
								}
							}
							else {
								// var1
								if (x <= token_arr + token_arr_len - 1) {

									val = std::string(buffer + GetIdx(token_arr[i]), len);

									varVec.push_back(check_syntax_error1(var, option));
									valVec.push_back(check_syntax_error1(val, option));

									val = "";

									state = 0;

								}
							}
						}
						else
						{
							// var1
							if (x <= token_arr + token_arr_len - 1)
							{
								val = std::string(buffer + GetIdx(token_arr[i]), len);
								varVec.push_back(check_syntax_error1(var, option));
								valVec.push_back(check_syntax_error1(val, option));
								val = "";

								state = 0;

							}
						}
					}
				}
				break;
				case 1:
				{
					// LEFT 1
					if (len == 1 && (-1 != Equal(1, GetType(token_arr[i])) || -1 != Equal(1, GetType(token_arr[i])))) {

						nestedUT[braceNum]->AddItem((varVec), (valVec), varVec.size());

						varVec.clear();
						valVec.clear();


						///
						{
							nestedUT[braceNum]->AddUserTypeItem(UserType(var));
							UserType* pTemp = nullptr;
							nestedUT[braceNum]->GetLastUserTypeItemRef(var, pTemp);
							var = "";
							braceNum++;

							/// new nestedUT
							if (nestedUT.size() == braceNum) {
								nestedUT.push_back(nullptr);
							}

							/// initial new nestedUT.
							nestedUT[braceNum] = pTemp;
						}
						///
						state = 0;
					}
					else {
						if (x <= token_arr + token_arr_len - 1) {
							val = std::string(buffer + GetIdx(token_arr[i]), len);

							//i += 1;

							varVec.push_back(check_syntax_error1(var, option));
							valVec.push_back(check_syntax_error1(val, option));
							var = ""; val = "";

							state = 0;
						}
					}
				}
				break;
				default:
					// syntax err!!
					throw "syntax error ";
					break;
				}
			}

			if (next) {
				*next = nestedUT[braceNum];
			}

			if (varVec.empty() == false) {
				nestedUT[braceNum]->AddItem(varVec, valVec, varVec.size());

				varVec.clear();
				valVec.clear();
			}

			if (state != last_state) {
				throw std::string("error final state is not last_state!  : ") + toStr(state);
			}
			if (x > token_arr + token_arr_len) {
				throw std::string("error x > buffer + buffer_len: ");
			}

			return true;
		}

		static long long FindRight(const char* buffer, const long long* token_arr, long long start, long long last, const wiz::LoadDataOption& option)
		{
			for (long long a = last; a >= start; --a) {
				long long len = GetLength(token_arr[a]);
				long long val = GetType(token_arr[a]); // % 4  


				if (len == 1 && (-1 != Equal(2, val) || -1 != Equal(2, val))) { // right
					return a;
				}

				bool pass = false;
				if (len == 1 && (-1 != Equal(1, val) || -1 != Equal(1, val))) { // left
					return a;
				}
				else if (len == 1 && -1 != Equal(3, val)) { // assignment
					//
					pass = true;
				}

				if (a < last && pass == false) {
					long long len = GetLength(token_arr[a + 1]);
					long long val = GetType(token_arr[a + 1]); // % 4

					if (!(len == 1 && -1 != Equal(3, val))) // assignment
					{                // NOT
						return a;
					}
				}
			}
			return -1;
		}

		static bool _LoadData(InFileReserver& reserver, UserType& global, const wiz::LoadDataOption& option, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
		{
			const int pivot_num = parse_num - 1;
			char* buffer = nullptr;
			long long* token_arr = nullptr;
			long long buffer_total_len;
			long long token_arr_len = 0;
			bool end = false;
			{
				end = !reserver(option, lex_thr_num, buffer, &buffer_total_len, token_arr, &token_arr_len);

				if (token_arr_len <= 0) {
					return true;
				}
			}

			UserType* before_next = nullptr;
			UserType _global;

			bool first = true;
			long long sum = 0;

			while (true) {
				end = true;

				std::set<long long> _pivots;
				std::vector<long long> pivots;
				const long long num = token_arr_len; //

				if (pivot_num > 0) {
					std::vector<int> pivot;
					pivots.reserve(pivot_num);
					pivot.reserve(pivot_num);

					for (int i = 0; i < pivot_num; ++i) {
						pivot.push_back(FindRight(buffer, token_arr, (num / (pivot_num + 1)) * (i), (num / (pivot_num + 1)) * (i + 1) - 1, option));
					}

					for (int i = 0; i < pivot.size(); ++i) {
						if (pivot[i] != -1) {
							_pivots.insert(pivot[i]);
						}
					}

					for (auto& x : _pivots) {
						pivots.push_back(x);
					}
				}

				std::vector<UserType*> next(pivots.size() + 1, nullptr);

				{
					std::vector<UserType> __global(pivots.size() + 1);

					std::vector<std::thread> thr(pivots.size() + 1);

					{
						long long idx = pivots.empty() ? num - 1 : pivots[0]; // chk? - !!
						long long _llptr2_len = idx - 0 + 1;
						//__global[0].ReserveUserTypeList(llptr3_total.utNum);
						thr[0] = std::thread(__LoadData, buffer, token_arr, _llptr2_len, &__global[0], &option, 0, 0, &next[0]);
						// __LoadData4 -> __LoadData5
					}

					for (int i = 1; i < pivots.size(); ++i) {
						long long _llptr2_len = pivots[i] - (pivots[i - 1] + 1) + 1;
						//__global[i].ReserveUserTypeList(llptr3_total.utNum);
						thr[i] = std::thread(__LoadData, buffer, token_arr + pivots[i - 1] + 1, _llptr2_len,  &__global[i], &option, 0, 0, &next[i]);

					}

					if (pivots.size() >= 1) {
						long long _llptr2_len = num - 1 - (pivots.back() + 1) + 1;
						//__global[pivots.size()].ReserveUserTypeList(llptr3_total.utNum);
						thr[pivots.size()] = std::thread(__LoadData, buffer, token_arr + pivots.back() + 1,_llptr2_len,  &__global[pivots.size()],
							&option, 0, 0, &next[pivots.size()]);
					}

					// wait
					for (int i = 0; i < thr.size(); ++i) {
						thr[i].join();
					}

					// Merge
					try {
						if (__global[0].GetUserTypeListSize() > 0 && __global[0].GetUserTypeList(0)->GetName() == "#") {
							std::cout << "not valid file1\n";
							throw 1;
						}
						if (next.back()->GetParent() != nullptr) {
							std::cout << "not valid file2\n";
							throw 2;
						}

						int err = Merge(&_global, &__global[0], &next[0]);
						if (-1 == err || (pivots.size() == 0 && 1 == err)) {
							std::cout << "not valid file3\n";
							throw 3;
						}

						for (int i = 1; i < pivots.size() + 1; ++i) {
							// linearly merge and error check...
							int err = Merge(next[i - 1], &__global[i], &next[i]);
							if (-1 == err) {
								std::cout << "not valid file4\n";
								throw 4;
							}
							else if (i == pivots.size() && 1 == err) {
								std::cout << "not valid file5\n";
								throw 5;
							}
						}
					}
					catch (...) {
						delete[] buffer;
						delete[] token_arr;
						buffer = nullptr;
						throw "in Merge, error";
					}

					before_next = next.back();
				}


				if (!end) {
					//
				}
				else {
					break;
				}
			}

			delete[] buffer;
			delete[] token_arr;

			global = std::move(_global);

			return true;
		}
	public:
		static bool LoadDataFromFile(const std::string& fileName, UserType& global, int lex_thr_num, int parse_num) /// global should be empty
		{
			if (lex_thr_num <= 0) {
				lex_thr_num = std::thread::hardware_concurrency();
			}
			if (lex_thr_num <= 0) {
				lex_thr_num = 1;
			}

			if (parse_num <= 0) {
				parse_num = std::thread::hardware_concurrency();
			}
			if (parse_num <= 0) {
				parse_num = 1;
			}

			bool success = true;
			std::ifstream inFile;
			inFile.open(fileName, std::ios::binary);


			if (true == inFile.fail())
			{
				inFile.close(); return false;
			}

			UserType globalTemp;

			try {

				InFileReserver ifReserver(inFile);
				wiz::LoadDataOption option;
				option.Assignment = ('=');
				option.Left = '{'; 
				option.Right = '}';
				option.LineComment = ('#');
				option.Removal = ',';

				char* buffer = nullptr;
				ifReserver.Num = 1 << 19;
				//	strVec.reserve(ifReserver.Num);
				// cf) empty file..
				if (false == _LoadData(ifReserver, globalTemp, option, lex_thr_num, parse_num))
				{
					inFile.close();
					return false; // return true?
				}

				inFile.close();
			}
			catch (const char* err) { std::cout << err << std::endl; inFile.close(); return false; }
			catch (const std::string& e) { std::cout << e << std::endl; inFile.close(); return false; }
			catch (std::exception e) { std::cout << e.what() << std::endl; inFile.close(); return false; }
			catch (...) { std::cout << "not expected error" << std::endl; inFile.close(); return false; }


			global = std::move(globalTemp);

			return true;
		}
		static bool LoadWizDB(UserType& global, const std::string& fileName, const int thr_num) {
			UserType globalTemp = UserType("global");

			// Scan + Parse 
			if (false == LoadDataFromFile(fileName, globalTemp, thr_num, thr_num)) { return false; }
			//std::cout << "LoadData End" << std::endl;

			global = std::move(globalTemp);
			return true;
		}
		// SaveQuery
		static bool SaveWizDB(const UserType& global, const std::string& fileName, const bool append = false) {
			std::ofstream outFile;
			if (fileName.empty()) { return false; }
			if (false == append) {
				outFile.open(fileName);
				if (outFile.fail()) { return false; }
			}
			else {
				outFile.open(fileName, std::ios::app);
				if (outFile.fail()) { return false; }

				outFile << "\n";
			}

			/// saveFile
			global.Save1(outFile); // cf) friend

			outFile.close();

			return true;
		}
	};

}

#endif