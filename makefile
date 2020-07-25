clau_parser:
	g++   -std=c++1z ./source/main.cpp -I./include -lpthread -pthread  -msse2   -Wno-narrowing  -O2 -o clau_parser
