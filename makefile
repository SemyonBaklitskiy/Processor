.PHONY: compile run disassemble clean_compile

bin/compiler_functions.o: Compiler/src/compiler_functions.cpp Compiler/includes/compiler_functions.h 
	if [ ! -d bin ]; then mkdir bin; fi
	g++ -Wall -Wextra -ICommon/includes/ -ICompiler/includes/ -c Compiler/src/compiler_functions.cpp -o bin/compiler_functions.o

bin/compiler_main.o: Compiler/src/main.cpp 
	g++ -Wall -Wextra -ICommon/includes/ -ICompiler/includes/ -c Compiler/src/main.cpp -o bin/compiler_main.o

bin/common_functions.o: Common/src/common_functions.cpp Common/includes/common_functions.h Common/includes/common.h
	g++ -Wall -Wextra -ICommon/includes/ -c Common/src/common_functions.cpp -o bin/common_functions.o

compile: bin/compile.exe
	bin/./compile.exe

bin/compile.exe: bin/compiler_functions.o bin/compiler_main.o bin/common_functions.o
	g++ -Wall -Wextra bin/common_functions.o bin/compiler_functions.o bin/compiler_main.o -o bin/compile.exe

bin/stack_functions.o: Stack/src/stack_functions.cpp Stack/includes/stack_functions.h
	g++ -Wall -Wextra -IStack/includes/ -c Stack/src/stack_functions.cpp -o bin/stack_functions.o

bin/cpu_functions.o: CPU/src/cpu_functions.cpp CPU/includes/cpu_functions.h
	g++ -Wall -Wextra -IStack/includes/ -ICPU/includes/ -ICommon/includes/ -c CPU/src/cpu_functions.cpp -o bin/cpu_functions.o

bin/cpu_main.o: CPU/src/main.cpp
	if [ ! -d bin ]; then mkdir bin; fi
	g++ -Wall -Wextra -IStack/includes/ -ICPU/includes/ -ICommon/includes/ -c CPU/src/main.cpp -o bin/cpu_main.o

run: bin/run.exe
	bin/./run.exe

bin/run.exe: bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o bin/common_functions.o
	g++ -Wall -Wextra bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o bin/common_functions.o -o bin/run.exe

bin/disassembler_functions.o: Disassembler/src/disassembler_functions.cpp Disassembler/includes/disassembler_functions.h
	g++ -Wall -Wextra -IDisassembler/includes/ -ICommon/includes/ -c Disassembler/src/disassembler_functions.cpp -o bin/disassembler_functions.o

bin/disassembler_main.o: Disassembler/src/main.cpp
	if [ ! -d bin ]; then mkdir bin; fi
	g++ -Wall -Wextra -IDisassembler/includes/ -ICommon/includes/ -c Disassembler/src/main.cpp -o bin/disassembler_main.o

disassemble: bin/disassemble.exe
	bin/./disassemble.exe

bin/disassemble.exe: bin/disassembler_main.o bin/disassembler_functions.o bin/common_functions.o
	g++ -Wall -Wextra bin/disassembler_main.o bin/disassembler_functions.o bin/common_functions.o -o bin/disassemble.exe

clean:
	rm -r bin/

sanitizer: Stack/src/stack_functions.cpp Stack/includes/stack_functions.h CPU/src/cpu_functions.cpp CPU/includes/cpu_functions.h Common/src/common_functions.cpp Common/includes/common_functions.h Common/includes/common.h CPU/src/main.cpp
	g++ -Wall -Wextra -fsanitize=address -ICommon/includes/ -c Common/src/common_functions.cpp -o bin/common_functions.o
	g++ -Wall -Wextra -fsanitize=address -IStack/includes/ -c Stack/src/stack_functions.cpp -o bin/stack_functions.o
	g++ -Wall -Wextra -fsanitize=address -IStack/includes/ -ICPU/includes/ -ICommon/includes/ -c CPU/src/cpu_functions.cpp -o bin/cpu_functions.o
	g++ -Wall -Wextra -fsanitize=address -IStack/includes/ -ICPU/includes/ -ICommon/includes/ -c CPU/src/main.cpp -o bin/cpu_main.o
	g++ -Wall -Wextra -fsanitize=address bin/cpu_main.o bin/cpu_functions.o bin/stack_functions.o bin/common_functions.o -o bin/sanitizer.exe
	bin/./sanitizer.exe

