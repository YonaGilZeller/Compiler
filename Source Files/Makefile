CREATOR = gcc -Wall -ansi -pedantic -std=c99

compiler: compiler.o firstRun.o preAssembly.o secondRun.o dataFunctions.o
	$(CREATOR) -o compiler compiler.o firstRun.o preAssembly.o secondRun.o dataFunctions.o 
	
compiler.o: compiler.c 
	$(CREATOR) -c compiler.c 

preAssembly.o: preAssembly.c
	$(CREATOR) -c preAssembly.c	

firstRun.o: firstRun.c
	$(CREATOR) -c firstRun.c
	
secondRun.o: secondRun.c
	$(CREATOR) -c secondRun.c
	
dataFunctions.o: dataFunctions.c
	$(CREATOR) -c dataFunctions.c