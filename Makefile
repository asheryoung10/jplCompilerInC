SOURCES = main.c sourceCode.c token.c lexer.c parse.c ast.c
jplCompilerInC : $(SOURCES) 
	clang $(SOURCES) -o jplCompilerInC
testRun : jplCompilerInC four.jpl
	./jplCompilerInC four.jpl
run: jplCompilerInC
	./jplCompilerInC $(FLAGS) $(TEST)