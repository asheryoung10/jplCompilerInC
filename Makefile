SOURCES = main.c sourceCode.c token.c lexer.c
jplCompilerInC : $(SOURCES) 
	clang $(SOURCES) -o jplCompilerInC
run : jplCompilerInC four.jpl
	./jplCompilerInC four.jpl