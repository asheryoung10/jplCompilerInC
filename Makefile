jplCompilerInC : main.c sourceCode.c sourceCode.h
	clang main.c sourceCode.c -o jplCompilerInC
run : jplCompilerInC four.jpl
	./jplCompilerInC four.jpl