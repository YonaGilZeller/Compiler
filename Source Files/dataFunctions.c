#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fileList.h"


/*
 * This file contains the functions for the data structures defined in "fileList.h".
 */

//returns the length of a list (does not include head)
//listlength receives the head node of a list and returns the lists size
//input: (node) head node of a list
//output: (int) number of nodes in the list
int listLength(node *head) {
	int i = 0;
	head = head->next;
	for(;head != NULL; i++) head = head->next;
	return i;
}
//searches for a string within a list of strings and returns its index. returns 0 if the string is not found
//input: (node) head node of a list
// 		 (char*) string to be searched within the list
//output: if the string is not found 0, if found returns its index within the list
int searchNode(node *head, char *word) {
	head = head->next;//the heads value is always null, so we immediately skip to the next node
	//scans through the list and compares the nodes to the inputed word
	//returns the position of the node if found (assuming the head is 0)
	for(int i=1; head != NULL; i++) {
		if(head->value != NULL && !strcmp(head->value, word)) return i; //if the value is  null, we would get a segmentation error
		head = head->next;
	}
	return 0; //returns 0 if not found
}

//searches to see if a macro of an inputed word exists
//input: (node2d*) head node of a list
//		 (char*) string
//output: if the string is not found 0, if it's found the nodes position
int searchNode2d(node2d *head, char *word) {
	head = head->next;
	for(int i=1; head != NULL; i++) {
		if(head->name != NULL && !strcmp(head->name, word)) return i;
		head = head->next;
	}
	return 0; //returns 0 if not found

}

//searches a line for a macro name, returns the macro position in the macro list if found
//input: (node2d*): 2d list
//		 (node*) list
//output: if the macro is not found 0, if it is found its position is returned
int searchMacro(node2d *head, node *line) {
	int out = 0;
	while(line != NULL) {
		int val = searchNode2d(head, line->value);
		if(val) { //if it is found
			if(out) {
				printf("ERROR line %d: invalid call for macro statement\n", __LINE__);
				return 0;
			}
			out = val;
		}
		line = line->next;
	}
	return out;
}


//splits a given string into words by isspace function (and removes them) and by "(),", returns a list of the words
//input: (char*) string
//output: (node*) list of strings separated by spaces and parenthesis
node* spaceSplit(char *input) {
	node *head = (node *)calloc(1, sizeof(node));
	node *current_node = head;

	char *current_char = input;

	while(*current_char != 0) {
		//before the word
		while(isspace(*current_char)) ++current_char;
		//after the word
		char *first = current_char;
		if(current_char == 0) break;
		int len = 0;
		while(!isspace(*current_char) && *current_char != 0) { ++current_char; ++len;};
		if(len == 0) break; //rechecks if the current char is the end of the string
		//return output

		char *word = (char *)calloc(len, sizeof(char));
		strncpy(word, first, len);
		node *temp = (node *)calloc(1, sizeof(node));
		temp->value = word;
		current_node->next = temp;
		current_node = current_node->next;

	}
	return head;
}


//remove whitespaces/tabs before and after the first word in a string
//input: (char*) string
//output: (char*) string without whitespaces and tabs
char* removeWhitespaces(char *input) {
	//before the word
	char *first = input;
	while(isspace(*first)) ++first;
	//after the word
	char *last = first;
	int len = 0;
	while(!isspace(*last)) { ++last; ++len;};
	//return output
	char *output = (char *)calloc(len, sizeof(char));
	strncpy(output, first, len);
	return output;

}

//getMethodValue determines if a string is an immediate number, a label, a register or an invalid operand
//input: (char*) string
//output: (int) if register r0-r7, LABEL if label, the number itself if it is an immediate number, and ERROR if it is invalid
int getMethodValue(char *word) {

	if(*word == '#') { //immediate number
		int val = atoi(word+1);
		if(val >= MIN_NUM && val <= MAX_NUM) return val; //valid number within the parameters of a 12 bit 2s compliment value
		else return ERROR; //error invalid immediate number
	}
	//register
	if(*word == 'r') {
		if(*(word+1) >= '0' && *(word+1) <= '7' && *(word+2) == 0) return r0 + (*(word+1) - '0'); //register
		else return ERROR; //error invalid register
	}
	//else assume it is a label - if the label does not exist we will receive an error during secondRun
	return LABEL;
}

//appends a list of instructions with a new instruction
//input: (ins*) last node in a list of instructions
//		 (char*) name of the instruction
//		 (int) line number for error handling
//output: (ins*) new instruction created
ins* newIns(ins* IC, const char* name, int line) {
	ins* new_ins = (ins *)calloc(1, sizeof(ins));
	int word_len = strlen(name);
	new_ins->word = (char *)calloc(word_len, sizeof(char));
	strncpy(new_ins->word, name, word_len);
	new_ins->line = line;
	new_ins->loc = IC->loc + 1;
	IC->next = new_ins;

	return new_ins;
}

//checks if a label name is valid - first char needs to be a letter, and the name cannot be another label or a saved term (such as a command name)
//input: string with ':' at the end
//output: 0 if invalid, 1 if valid
int validLabel(char *word, lab* label) {
	const char *commands[] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string", ".entry", ".extern"};
	if(!isalpha(*word)) { //first
		return 0;
	}
	//checking if it equals to an already defined label
	int label_len = strlen(word)-1;
	char *name = (char *)calloc(label_len, sizeof(char)); //contains the label name without ':'
	strncpy(name, word, label_len);
	label = label->next; //head is empty
	while(label != NULL) {
		if(strcmp(name, label->name) == 0) return 0;
		label = label->next;
	}
	//checking that the label is comprised of letters and numbers and is less than 30 chars
	char *i = name+1;
	for(int counter = 1; *i != 0; ++counter, ++i) {
		if(!isalpha(*i) && !isdigit(*i)) return 0; //labels can only be letters and numbers
		if(counter >= MAX_LABEL_LENGTH) return 0;
	}

	//checking if it is the name of a command

	for(int i=0;i < sizeof commands/sizeof commands[0]; i++) if(strstr(name, commands[i]) != NULL) return 0;
	//valid label
	return 1;
}

//receives the user input from main and separates the string to file names
//returns a list of nodes containing file names
void getFiles(char *input, node *head) {

	node *current = head;

	//seperating the string by commas
	const char comma[2] = ",";
	char *token;

	token = strtok(input, comma);
	//loops until no commas are found
	while(token != NULL) {
		token = removeWhitespaces(token);
		//creates a new node with the string until the next comma and adds it to the list
		node *temp = (node *)calloc(1, sizeof(node));
		temp->value = token;
		current->next = temp;
		current = temp;

		token = strtok(NULL, comma);
	}
}

//receives a file name from main and makes sure the file name is valid for the compiler
//input: node containing a file name
//output: 1 if valid, 0 if invalid
int validFile(node *input) {
	//file names must be inputted without the file extension
	if(strchr(input->value, '.') != NULL) {
		printf("ERROR: %s invalid file name\nPlease input the file names without the file extension\n", input->value);
		return 0;
	}
	//checking that filename.as exists in the system
	char *file_name = (char *)calloc(strlen(input->value), sizeof(char));
	strcpy(file_name, input->value);
	strcat(file_name, ".as");
	FILE *fp = fopen(file_name, "r");
	//file is not found
	if(fp == NULL) {
		printf("ERROR: %s could not find file\n", file_name);
		return 0;
	}
	//valid file
	fclose(fp);
	return 1;
}




