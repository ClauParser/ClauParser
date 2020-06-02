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
이런걸 소개해보고자 합니다.


#
스캐닝을하기전에? 파일에서 데이터를 읽어와야겠죠?
#
	std::ifstream inFile("input.txt");
	std::vector<std::string> result;
	std::string line;
	while (std::getline(inFile, line)) {
		result.push_back(line);
	}
이렇게하면 안됩니다. 병렬스캐닝+병렬파싱하는 시간보다 더 오래 걸렸던것 같습니다.
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
이렇게 std::string을 쓰는 방법 C++ 스럽다? 라고 할 수 도 있겠지만, 경험상? 속도가 생각보다 느려지는 것 같습니다.
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
(이제는 ScanningNew함수처럼 abc "ddd eee" fgh 3개의 토큰으로 간주한다.)

#
 그럼 병렬 스캐닝(Parallel Scanning)은 어떻게 해야할까요?
그냥 { } = whitespace등으로 나눈다음 각각 스캐닝을 할 수 있을까요?

"test xyz"  <- 이 문장에서 가운데 space로 텍스트가 나누어졌을때, 
"test, xyz" 에서 "test의 "가 여는 따옴표인지 닫는 따옴표인지 아는것이 쉽지가 않습니다. 
 더욱이 #"test xyz" 같이 앞에 주석이라도 있는 경우라면 제대로 스캐닝하기가 쉽지 않겠죠. 
 
 그래서 제가 생각한? 아이디어는 스캐닝을 크게 두 과정으로 나눠서
첫번째 과정(전처리?과정?)에서 " "를 하나의 토큰으로 보는것이 아니라 
일단 "를 길이가 1인 토큰 하나로 보는 것입니다. 

 또한 주석(#처럼 한글자로 시작하는)도 길이가 1인 토큰 1개로 보고 
두번째 과정에서 첫번째 과정에서 만든 토큰 후보?들을 적절하게 골라내고 필요하다면 수정하는 것입니다.

 첫번째 과정은 state같은? 변수가 필요하지않기때문에 쉽게 병렬화가 가능합니다.
두번째 과정은 Sequential하게 진행됩니다.

# 
첫번째 과정 함수

	void _Scanning(char* text, long long num, const long long length,
			long long*& token_arr, long long&  _token_arr_size, const LoadDataOption& option) 
두번째 과정 부분
	
	int state = 0;
	long long qouted_start;
	long long slush_start;

	for (long long i = 0; i < token_count; ++i) { // 전체 크기만큼이 아니라  토큰 후보? 개수만큼 반복한다!
		const long long len = GetLength(tokens[i]);
		const char ch = text[GetIdx(tokens[i])];
		const long long idx = GetIdx(tokens[i]);
		const bool isToken2 = IsToken2(tokens[i]);

		if (isToken2) {
			if (0 == state && '\"' == ch) {
				state = 1;
				qouted_start = i;
			}
			else if (0 == state && option.LineComment == ch) {
				state = 2;
			}
			else if (1 == state && '\\' == ch) {
				state = 3;
				slush_start = idx;
			}
			else if (1 == state && '\"' == ch) {
				state = 0;

				{
					long long idx = GetIdx(tokens[qouted_start]);
					long long len = GetLength(tokens[qouted_start]);
					
					len = GetIdx(tokens[i]) - idx + 1;

					tokens[real_token_arr_count] = Get(idx, len, text[idx], option);
					real_token_arr_count++;
				}
			}
			else if (3 == state) {
				if (idx != slush_start + 1) {
					--i;
				}
				state = 1;
			}
			else if (2 == state && ('\n' == ch || '\0' == ch)) {
				state = 0;
			}
		}
		else if (0 == state) { // '\\' case?
			tokens[real_token_arr_count] = tokens[i];
			real_token_arr_count++;
		}
	}
#
주의사항 : 여기서는 abc"123 456"def는 abc, "123 456", def 3개의 토큰으로 구분된다.
