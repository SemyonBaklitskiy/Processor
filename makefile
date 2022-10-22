.PHONY: compile run clean_compile

# TODO: do dependencies

bin/compiler_functions.o: 
	g++ -Wall -Wextra -c Compiler/compiler_functions.cpp -o bin/compiler_functions.o

bin/compiler_main.o:
	g++ -Wall -Wextra -c Compiler/main.cpp -o bin/compiler_main.o

compile: bin/compiler_functions.o bin/compiler_main.o
	g++ -Wall -Wextra bin/compiler_functions.o bin/compiler_main.o -o bin/compile.exe
	bin/./compile.exe

clean_compile:
	rm bin/compiler_functions.o bin/compiler_main.o bin/compile.exe

bin/stack_functions.o:
	g++ -Wall -Wextra -c Stack/src/stack_functions.cpp -o bin/stack_functions.o

bin/cpu_functions.o:
	g++ -Wall -Wextra -c CPU/cpu_functions.cpp -o bin/cpu_functions.o

bin/cpu_main.o:
	g++ -Wall -Wextra -c CPU/main.cpp -o bin/cpu_main.o

run: bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o
	g++ -Wall -Wextra bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o -o bin/run.exe
	bin/./run.exe

clean_run:
	rm bin/cpu_functions.o bin/cpu_main.o bin/stack_functions.o bin/run.exe