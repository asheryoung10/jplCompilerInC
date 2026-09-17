SOURCES = main.c sourceCode.c token.c lexer.c parse.c ast.c
jplCompilerInC : $(SOURCES) 
	clang $(SOURCES) -o jplCompilerInC
testRun : jplCompilerInC
	./jplCompilerInC -p compare/049.jpl
run: jplCompilerInC
	./jplCompilerInC $(FLAGS) $(TEST)