.PHONY: compile run clean_compile

bin/compiler_functions.o: Compiler/compiler_functions.cpp
	g++ -Wall -Wextra -c Compiler/compiler_functions.cpp -o bin/compiler_functions.o

bin/compiler_main.o: Compiler/main.cpp
	g++ -Wall -Wextra -c Compiler/main.cpp -o bin/compiler_main.o

bin/common_functions.o: Common/common_functions.cpp
	g++ -Wall -Wextra -c Common/common_functions.cpp -o bin/common_functions.o

compile: bin/compiler_functions.o bin/compiler_main.o bin/common_functions.o
	g++ -Wall -Wextra bin/common_functions.o bin/compiler_functions.o bin/compiler_main.o -o bin/compile.exe
	bin/./compile.exe

bin/stack_functions.o: Stack/src/stack_functions.cpp
	g++ -Wall -Wextra -c Stack/src/stack_functions.cpp -o bin/stack_functions.o

bin/cpu_functions.o: CPU/cpu_functions.cpp
	g++ -Wall -Wextra -c CPU/cpu_functions.cpp -o bin/cpu_functions.o

bin/cpu_main.o: CPU/main.cpp
	g++ -Wall -Wextra -c CPU/main.cpp -o bin/cpu_main.o

run: bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o bin/common_functions.o
	g++ -Wall -Wextra bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o bin/common_functions.o -o bin/run.exe
	bin/./run.exe

clean:
	rm bin/cpu_functions.o bin/cpu_main.o bin/stack_functions.o bin/run.exe bin/compiler_functions.o bin/compiler_main.o bin/compile.exe bin/common_functions.o