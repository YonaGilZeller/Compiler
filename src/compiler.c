#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fileList.h"

//receives the requested file names from the user through the terminal and runs the compilation process
int main() {
	node *files;
	files = (node *)malloc(sizeof(node));

	//input
	char *input;
	size_t buffsize=0;
	//default input size to 1
	input = (char *)malloc(1);
	//gets input of file names from the user
	printf("Please enter the files to be compiled\n");
	getline(&input, &buffsize, stdin);
	getFiles(input, files);

	files = files->next;
	//runs the compiler on all of the files inputed
	while(files != NULL) {
		printf("File name: %s\n", files->value);
		if(validFile(files)) {
			preAssembly(files->value);
			firstRun(files->value);
			//secondRun is run at the end of firstRun
		}

		files = files->next;
	}

}



