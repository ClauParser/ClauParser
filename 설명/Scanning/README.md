#
패러독스(게임회사)의 게임(clausewitz엔진으로 개발된)의 데이터 파일은
주로 특정한 게임데이터 형식을 가진다.
#
	EU4Game  
	# line comment
	countries = {
		-1 = {
			name = "USA"
			core = { 1 2 3 } # array?
		}
	}
요런걸 소개해보고자 합니다.


#
스캐닝을하기전에? 파일에서 데이터를 읽어와야겠죠?
#
	std::ifstream inFile("input.txt");
	std::vector<std::string> result;
	std::string line;
	while (std::getline(inFile, line)) {
		result.push_back(line);
	}
요렇게하면 안됩니다. 병렬스캐닝+병렬파싱하는 시간보다 더 오래 걸렸던것 같습니다.
#
	std::ifstream inFile("input.txt", std::ios::binary);
	long long file_length = GetFileLength(inFile); // 사용자 정의 함수. (파일크기구하도록하는..)
	char* buffer = new char[file_length + 1]; // 
	// read data as a block:
	inFile.read(buffer, file_length);
	buffer[file_length] = '\0';
    
    
    
#  
Token을 어떻게 구현할지도 속도 차이가 생기는 것 같습니다.
#
	    class Token {
	    public:
		std::string str;
		int type;
	    }
요렇게 std::string을 쓰는 방법 C++ 스럽다? 라고 할 수 도 있겠지만, 경험상? 속도가 생각보다 느려지는 것 같습니다.
#
    class Token {
    public:
        const char* str; // start of token
        int len; // length of token
        int type;
    };
그래서 이런 방법으로 바꾸어봤습니다. 
#
그리고 Token를 std::vector를 이용해 저장한다면
 #
    std::vector<Token> tokens;
    tokens.reserve(file_length / 2 + 1);
    for (int i = 0; i < file_length / 4; ++i) {
        tokens.push_back(Token());
    }
#
그런데 Token대신 long long이라는 타입으로 대체한다고 생각해봅시다.
#
    std::vector<long long> tokens;
    tokens.reserve(file_length / 2 + 1);
    for (int i = 0; i < file_length / 4; ++i) {
        tokens.push_back(222);
    }
Token쓴 것보다 long long쓴 것이 속도가 더 빠릅니다.
그리고 잘 생각해보면 게임 데이터의 용량이 많아야 파일당 100MB이내이기 때문에,
long long(64bit) = token 시작 위치 (32bit) + token 길이(32-2bit) + token type(2bit, { } = 일반문자열) 으로
Token를 클래스가 아니라 long long타입으로 나타낼 수 있을 것 같습니다.
#
	long long Get(long long position, long long length, char ch, const wiz::LoadDataOption& option)
#
Sequential Scanning.
#
	int state = 0; 

	long long token_first = 0;  // 토큰 시작위치
	long long token_last = -1;  // 토큰 끝위치

	long long token_arr_count = 0;
#
	for (long long i = 0; i <= length; ++i) { // text[length] 에는 '\0'이 들어 있다.
		const char ch = text[i];

#
#
	else if (isWhitespace(ch) || '\0' == ch) {
		token_last = i - 1;
		if (token_last - token_first + 1 > 0) { // 이전(앞의?) 토큰을 추가 한다.
			// token_arr : long long 배열.
			token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
			token_arr_count++;
		}
		token_first = i + 1;
		token_last = i + 1;
	}
#
	else if (option.Right == ch) {
		token_last = i - 1;
		if (token_last - token_first + 1 > 0) { // 이전(앞의?) 토큰을 추가한다.
			token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
			token_arr_count++;
		}
		token_first = i;
		token_last = i;
		
		// 현재 } (Right)를 토큰 배열에 추가한다.
		token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
		token_arr_count++;

		token_first = i + 1;
		token_last = i + 1;

	}
#
line comment를 시작하는 #를 만나면 state가 3이되고 state가 3이면서 엔터키나 '\0'를 만나면 주석이 종료됨.
#
	else if (3 == state) {
		if ('\n' == ch || '\0' == ch) {
			state = 0;

			token_first = i + 1;
			token_last = i + 1;
		}
	}
#
주의사항 : Scanning함수에는 abc"ddd eee"fgh 를 하나의 토큰으로 간주함.
