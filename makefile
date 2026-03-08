all:
	flex -+ rubix.l 
	g++ -o lexer lex.yy.cc my_lexer.cpp 
clean:
	rm lexer lex.yy.cc
