#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fileList.h"


/*
 * This file contains the pre-assembly process:
 * 1)finding macros and removing them from the file
 * 2)inserting macros when called upon
 * 3)saving the new file with .am extension
 *
 * This function uses data structures from fileList and its functions from dataFunctions.
 */



//receives file name to run preAssembly on, and returns the file name of the pre assembled file
void preAssembly(char *file_name) {
	char *file = malloc(strlen(file_name));
	strcpy(file, file_name);
	strcat(file, ".as");

	FILE *ptr = fopen(file, "r");

	char *new_file = malloc(sizeof(file_name));
	strcpy(new_file, file_name);
	strcat(new_file, ".am");

	FILE *nptr = fopen(new_file, "w");

	//search for macros and save them as a macro
	//replace those lines of code with the macro

	node2d *mhead = malloc(sizeof(node2d)); //macro head
	node2d *mcurrent = mhead; //last macro node


	char *line;
	size_t buffsize = 0;
	line = (char *)malloc(1);
	int line_count = 0; //contains the line number of the read file for errors
	while(getline(&line, &buffsize, ptr) != -1) {
		++line_count;
		node *word = malloc(sizeof(node)); //word contains the head of the words in a line contained in a list
		word = spaceSplit(line);
		if(listLength(word) == 0) continue; //if it is an empty line, read the next line

		int mpos = searchNode(word, "mcr"); //mpos contains the position of the word mcr in the line (if not found is 0)
		//if the word mcr is found in the line, save its position
		if(mpos) { //if the word mcr is found

			// checking valid macro statement
			if(listLength(word) > 2 || mpos != 1) { //if the macro statement has more than "mcr name", and mcr is not the first word
				printf("ERROR line %d: invalid macro statement\n", line_count);
				continue; //moves on to the next line
			}
			//else creates new macro
			node2d *temp = malloc(sizeof(node2d)); //new macro
			node *current = malloc(sizeof(node));//current line in the new macro

			temp->name = word->next->next->value; //second word in the line, which is the macro name
			temp->lines = current;
			node *macro_line; //list with the words in the line

			//loop that fills the lines of the macro
			while(1) {
				int l = getline(&line, &buffsize, ptr) != -1; //l contains the char len of the line
				if(l == -1) goto END_PRE_ASSEMBLY;

				++line_count;

				macro_line = spaceSplit(line);
				if(searchNode(macro_line, "endmcr")) break;

				node *new_line = malloc(sizeof(node));
				new_line->value = (char *)malloc(strlen(line));
				strncpy(new_line->value, line, strlen(line));
				current->next = new_line;
				current = new_line;
			}

			if(listLength(macro_line) > 1) { //if there is more than just the word "endmcr" it is an invalid statement
				printf("ERROR line %d: endmcr command needs to be in a seperate line\n",line_count);
				continue;
			}
			mcurrent->next = temp;
			mcurrent = temp;
		}
		//not a macro statement
		else {
			//check if the line is to call a declared macro
			int macro_index = searchMacro(mhead, word->next);
			if(macro_index) {

				if(listLength(word) > 1) {
					printf("ERROR line %d: call for a macro command needs to be in a seperate line\n", line_count);
					continue;
				}

				node2d *temp = mhead->next;
				for(int i=1; i < macro_index && temp!=NULL; i++) temp = temp->next;


				//if the macro is not empty
				if(listLength(temp->lines) > 0) {
					node *copy_line = temp->lines->next;

					while(copy_line != NULL && copy_line->value != NULL) {
						fprintf(nptr, "%s", copy_line->value);
						copy_line = copy_line->next;
					}
				}
			}
			//line has nothing to do with macros - copies it directly to the .am file
			else {
				fprintf(nptr, "%s", line);
			}
		}
	}
	fclose(nptr);
	fclose(ptr);
	END_PRE_ASSEMBLY:;
}



