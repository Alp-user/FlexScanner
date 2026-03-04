all:
	flex -+ rubix.l 
	g++ -o lexer lex.yy.cc my_lexer.cpp 
	./lexer inputs/rubix.txt
clean:
	rm lexer lex.yy.cc
