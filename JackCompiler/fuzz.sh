#/usr/bin/sh
clang++ CompilationEngine.cpp Fuzz.cpp JackTokenizer.cpp SymbolTable.cpp Token.cpp UnexpectedTokenException.cpp Variable.cpp VmWriter.cpp -Wall -Wextra -Werror -Wpedantic -I. -g -fsanitize=fuzzer,address -o fuzz \
&& ASAN_OPTIONS=detect_leaks=1 ./fuzz

