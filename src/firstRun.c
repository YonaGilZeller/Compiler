#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fileList.h"



/*
 * check if first word is a label
 * if true, save the label location and then check if it is data or a command
 * else, it is a command
 * get the command, and then the operators
 *
 */

void firstRun(char *file_name) {
	//opening .am file - post preAssembly file without macros
	char *input_file = (char *)malloc(sizeof(file_name));
	strcpy(input_file, file_name);
	strcat(input_file, ".am");
	FILE *ptr = fopen(input_file, "r");
	//head nodes of the IC/DC list respectively
	ins *IC = (ins *)calloc(1, sizeof(ins));
	IC->loc = 99; //memory begins at 0100
	ins *DC = (ins *)calloc(1, sizeof(ins));
	DC->loc = 0; //all DC instructions are added to the end of the IC
	//last nodes in the IC/DC lists respectively
	ins *DC_current = DC;
	ins *IC_current = IC;
	//list of entry labels
	ins* entries = (ins *)calloc(1, sizeof(ins));
	ins* entry_current = entries;
	//list of extern labels
	lab* externs = (lab *)calloc(1, sizeof(lab));
	lab* extern_current = externs;
	//list of labels
	lab *label = (lab *)calloc(1, sizeof(ins));
	lab *label_current = label;

	unsigned char lab_type = 1; //label flag for saving its location between DC and IC
	unsigned char hasError = 0; //error flag so the program does not create any files if there is an error
	//scans .am file line by line
	size_t buffsize = 0;
	char *line = (char *)malloc(1);
	int line_count = 0; //contains the line number of the read file for errors
	while(getline(&line, &buffsize, ptr) != -1) {

		//split the line into words
		++line_count;
		node *word = (node *)calloc(1, sizeof(node)); //contains the line split into words (by spaces) in a list
		word = spaceSplit(line);

		if(word->next == NULL) continue; //empty line

		word = word->next; //first node is always empty

		if(*word->value == ';') continue; //if the line is a comment, skip to the next line

		/*
		 * This code does the following:
		 * 1) Is there a label? if true, save the label
		 * 2) Find the command
		 * 3) Find the command parameters
		 * 4) Depending on the type of parameters/command create instructions in binary
		 */

		//check label - check if the first word contains ":"
		char *i = strchr(word->value, ':');
		if(i != NULL) {
			if(*++i != 0 || !validLabel(word->value, label)) { //if : is not the last char in the string or not a valid label name
				printf("ERROR line %d: invalid declaration of label\n", line_count);
				printf("Reminder: Labels are comprised of up to 30 letters (a-z, A-Z) and digits (0-9).\nThe first char must also be a letter.\n");
				++hasError;
				continue;
			}
			//valid label - store label location
			lab *new_label = (lab *)calloc(1, sizeof(ins));
			int lab_len = strlen(word->value)-1;
			new_label->name = (char *)calloc(lab_len, sizeof(char));
			strncpy(new_label->name, word->value, lab_len); //name of the label without the ':'
			label_current->next = new_label; //adding new label to the linked list of labels
			label_current = label_current->next;
			lab_type = 0;

			word = word->next; //moving to the next word in the line
		}
		//if there is a label with no command, skip to the next line
		if(word == NULL) {
			continue;
		}
		int command = 0; //index in the list "commands"
		//the order of the commands is by their respective opcode
		const char *commands[] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string", ".entry", ".extern"};

		//scanning through list of commands ->could be optimized with a better scanning algorithm
		for(;command < sizeof commands/sizeof commands[0]; command++) if(strstr(word->value, commands[command]) != NULL) break;

		//if no command is found
		if(command == sizeof commands/sizeof commands[0]) {
			printf("ERROR line %d: invalid command %s\n", line_count, word->value);
			++hasError;
			continue;
		}
		//determines the type of command - two operand, one operand, no operand, data, entry, extern
		i = strstr(line, commands[command]) + strlen(commands[command]) + 1; //getting index of the end of the word command

		//.entry
		if(command == 18) {
			//gets the name of the label
			while(isspace(*i)) ++i;
			char *first = i;
			while(!isspace(*i)) ++i;
			//creates a new entry node containing the name of the label
			ins* new_entry = (ins *)calloc(1, sizeof(ins));
			new_entry->word = (char *)calloc(i-first, sizeof(char));
			strncpy(new_entry->word, first, i-first);
			new_entry->line = line_count;
			entry_current->next = new_entry;
			entry_current = entry_current->next;
			continue;
		}
		//.extern
		else if(command == 19) {
			//get the name of the label
			while(isspace(*i)) ++i;
			char *first = i;
			while(!isspace(*i)) ++i;
			//creates a new extern node containing the name of the label
			lab *new_extern = (lab *)calloc(1, sizeof(ins));
			new_extern->name = (char *)calloc(i-first, sizeof(char));
			strncpy(new_extern->name, first, i-first); //name of the label without the ':'
			extern_current->next = new_extern; //adding new label to the linked list of labels
			extern_current = extern_current->next;
			continue;
		}

		//instruction

		ins *new_op;

		//checks if the command is an instruction or data and adds it to the IC/DC list
		//if the line has a label, gives the label its location based on if the command is DC or IC
		if(command <= 15) {
			new_op = newIns(IC_current, commands[command], line_count); //creates a new instruction
			new_op->bin = command << 6; //opcode bits 6-9
			IC_current = new_op;
			if(lab_type == 0){ label_current->loc = IC_current->loc; lab_type++;} //gives the label its location
		}
		else {//data
			if(lab_type == 0) {label_current->loc = -1 * (DC_current->loc + 1); lab_type++;}//the labels location is negative to distinguish between data and instruction labels
		}

		//no additional code is required for "stop" and "rts" - their instruction is created by default

		//.data
		if(command == 16) {
			while(*i != '\n' || i!= 0) { //continue getting new numbers until it is the EOL
				while(isspace(*i)) ++i;
				char *first = i; //first number
				if(*i == '-' || *i == '+') ++i; //if there is a plus or minus sign, ignore and skip to the next char
				while(*i >= '0' && *i <= '9') i++;

				//creates a new instruction containing the number
				ins *new_num = newIns(DC_current, "", line_count);
				DC_current = new_num;
				strncpy(new_num->word, first, i-first); //copies the number
				new_num->bin = atoi(first);
				//checks if there are more numbers in the line
				while(isspace(*i)) ++i;
				if(*i == 0 || *i == '\n') break;
				if(*i != ',') {
					printf("ERROR line %d: invalid declaration of data\n", line_count);
					++hasError;
					goto NEW_LINE;
				}
				++i;
			}

		}
		//.string
		else if(command == 17) {
			//checks .string proper syntax
			while(isspace(*i)) ++i;
			if(*i != '"') {
				printf("ERROR line %d: syntax error - invalid declaration of string data %d\n", line_count, *i);
				++hasError;
				goto NEW_LINE;
			}
			++i; //skis over " char
			//loop that collects the chars and saves them as instructions until EOL or " char to signal the end of the string
			while(*i != '"' && *i != '\n' && i != 0) {
				if(*i >= 32 && *i <= 126) { //each char in the string needs to be a printable char from the ASCII table
					ins *new_char = newIns(DC_current, "", line_count);
					DC_current = new_char;
					strncpy(new_char->word, i, 1); //copies the single char
					new_char->bin = *i;

				}
				//if it is an invalid char, prints an error and skips it
				else {
					printf("ERROR line %d: invalid char during declaration of string data\n", line_count);
					++hasError;
				}
				++i;
			}
			//checks that the last char is " - valid syntax
			if(*i != '"') {
				printf("ERROR line %d: syntax error - invalid declaration of string data\n", line_count);
				++hasError;
				goto NEW_LINE;
			}
			//adding 0 to the end of the string
			ins *new_char = newIns(DC_current, "\0", line_count);
			DC_current = new_char;
		}
		//two operand commands
		else if((command >= 0 && command <=3) || command == 6) {
			//two operand

			//extracts the two operands from the line
			//extracting the "source" operand
			while(isspace(*i)) ++i; //ignoring spaces
			char *first = i;
			while(!isspace(*i) && *i != ',') ++i;
			char *src = (char *)calloc(i-first+1, sizeof(char));

			strncpy(src, first, i-first); //copying the name of the src from the line

			while(isspace(*i)) ++i; //ignoring spaces
			if(*i != ',') {
				printf("ERROR line %d: syntax error - operands need to be divided by ','\n", line_count);
				++hasError;
				continue;
			}
			++i; //moving to char after ','
			while(isspace(*i)) ++i; //ignoring spaces
			//extracting "destination" operand
			first = i;
			while(!isspace(*i)) ++i;
			char *dst = (char *)calloc(i-first+1, sizeof(char));
			strncpy(dst, first, i-first); //copying the name of the dst from the line

			//getMethodValue determines if the operands are labels, immediate numbers, registers, or invalid (error)
			int op_src = getMethodValue(src);
			int op_dst = getMethodValue(dst);
			//invalid operands
			if(op_src == ERROR || op_dst == ERROR) {
				printf("ERROR line %d: invalid src or dst\n", line_count);
				++hasError;
				continue;
			}
			//creates an additional instruction for the source operand
			ins *src_ins = newIns(IC_current, src, line_count);
			IC_current = src_ins;

			//immediate number
			if(op_src >= MIN_NUM && op_src <= MAX_NUM) {
				src_ins->bin = op_src << 2; //the number itself are bits 2-13
				new_op->bin += 0 << 4; //src bits 4-5. immediate numbers code is 0
			}
			//label
			else if(op_src == LABEL) {
				src_ins->bin = LABEL; //label location will be inserted during secondRun
				new_op->bin += 1 << 4; //src bits 4-5. labels code is 1
			}
			//register
			else if(op_src >= r0 && op_src <= r7) {
				src_ins->bin = ((op_src-r0) << 8); //register number for src are bits 8-13
				new_op->bin += 3 << 4; //src bits 4-5. register numbers are code 3

				//if dst is also a register, creates only one additional instruction for the operands
				//dst is also a register
				if(op_dst >= r0) {
					src_ins->bin += ((op_dst-r0) << 2);
					new_op->bin += 3 << 2;
					continue;
				}
			}

			//additional instruction for the dst operand
			ins *dst_ins = newIns(IC_current, dst, line_count);
			IC_current = dst_ins;

			//label
			if(op_dst == LABEL) {
				dst_ins->bin = LABEL; //label location will be complaeted during secondRun
				new_op->bin += 1 << 2; //src data type for label is 1
			}

			if(op_dst >= MIN_NUM && op_dst <= MAX_NUM) {
				if(command == 1) { //cmp can have an immediate dst
					dst_ins->bin = (op_dst-r0) << 2; //register number for src are bits 8-13
					new_op->bin += 1 << 2; //src data type for label is 1
				}
				else {
					printf("ERROR line %d: invalid dst - cannot be an immediate number for command %s\n", line_count, commands[command]);
					++hasError;
					continue;
				}
			}

		}
		//one operand
		else if((command >= 4 && command <= 5) || (command >= 7 && command <= 8) || (command >= 11 && command <= 12)) {
			//extracting the operand from the line
			while(isspace(*i)) ++i; //ignoring spaces
			char *first = i;
			while(!isspace(*i)) ++i;
			char *src = (char *)calloc(i-first+1, sizeof(char));

			strncpy(src, first, i-first); //copying the name of the src from the line

			int op_src = getMethodValue(src);

			ins *src_ins = newIns(IC_current, src, line_count); //extra instruction that contains the location of the src
			IC_current = src_ins;
			//invalid operand
			if(op_src == ERROR) {
				printf("ERROR line %d: invalid source\n", line_count);
				++hasError;
				continue;
			}
			//operand is register
			else if(op_src >= r0 && op_src <= r7) {
				src_ins->bin = (op_src-r0) << 2; //register number shifted to bits 2-7
				new_op->bin += 3 << 2; //code 3 (register) shifted to bits 2-3
			}
			//operand is a label
			else if(op_src == LABEL) {
				src_ins->bin = LABEL; //will be completed during secondRun
				new_op->bin += 1 << 2; //code 1 (label) shifted to bits 2-3
			}
			//operand is an immediate number
			else if(op_src >= MIN_NUM && op_src <= MAX_NUM) { //must be prn command to receive an immediate as the operand
				if(command != 12) {
					printf("ERROR line %d: invalid use of immediate\n", line_count);
					++hasError;
					continue;
				}
				src_ins->bin = op_src << 2; //the extra instructions value is the numerical value of the immediate number shifted to bits 2-13
			}

		}
		//jmp, bne, and jsr
		else if((command >= 9 && command <= 10) ||(command == 13)) {
			//extracting (mandatory) label from the line
			while(isspace(*i)) ++i; //ignoring spaces
			char *first = i;
			while(!isspace(*i) && *i != '(') ++i;
			char *lab = (char *)calloc(i-first+1,sizeof(char)); //label

			strncpy(lab, first, i-first); //copying the name of the src from the line

			//we should get three instructions (binary lines):
			//1 - command instruction (jmp, bne, or jsr)
			//2 - label location
			//3 - (optional) parameter locations


			//ins for the label
			ins *lab_ins = newIns(IC_current, lab, line_count);
			IC_current = lab_ins;
			lab_ins->bin = LABEL; //location will be handled during secondRun

			new_op->bin += 1 << 2; //dst by default is of type 1 (label)


			//ins for optional parameters
			char *op1, *op2;
			if(*i == '(') { //checking if the command has parameters
				new_op->bin += 1 << 2; //dst is of type 2 (jump) because it has parameters so we add 1 to turn type 1 to type 2
				//extracting the first operand
				first = ++i;
				while(*i != ',') ++i;
				op1 = (char *)calloc(i-first+1, sizeof(char));
				strncpy(op1, first, i-first);
				//extracting the second operand
				first = ++i; //skips ',' char
				while(*i != ')') ++i;
				op2 = (char *)calloc(i-first+1, sizeof(char));
				strncpy(op2, first, i-first);

				int op1_type = getMethodValue(op1);
				int op2_type = getMethodValue(op2);
				//invalid operands
				if(op1_type == ERROR || op2_type == ERROR) {
					printf("ERROR line %d: invalid operands\n", line_count);
					++hasError;
					continue;
				}
				//analyzing the first operand type
				ins* op1_ins = newIns(IC_current, op1, line_count);
				IC_current = op1_ins;
				//operand 1 is an immediate number
				if(op1_type >= MIN_NUM && op1_type <= MAX_NUM) { //immediate number
					new_op->bin += 0 << 12; //code 0 (immediate) shifted to bits 12-13
					op1_ins->bin = op1_type << 2; //the extra instruction is the number shifted to bits 2-13
				}
				//operand 1 is a label
				else if(op1_type == LABEL){
					new_op->bin += 1 << 12; //code 1 (label) shifted to bits 12-13
					op1_ins->bin = LABEL; //location will be completed during secondRun
				}
				//operand 1 is register
				else if(op1_type >= r0 && op1_type <= r7) {
					new_op->bin += 3 << 12; //code 3 (register) shifted to bits 12-13
					op1_ins->bin = (op1_type-r0) << 8;
					//if both operands are registers, only one additional instruction is created
					if(op2_type >= r0) {
						strcat(op1_ins->word, op2); //instruction name becomes both registers
						new_op->bin += 3 << 10; //param 1 bits 10-11
						op1_ins->bin += (op2_type -r0) << 2; //only one extra instruction for the parameters
						continue;
					}
				}
				//creating instruction for the second operand
				ins* op2_ins = newIns(IC_current, op2, line_count);
				IC_current = op2_ins;
				//operand 2 is a register
				if(op2_type >= r0 && op2_type <= r7) {
					new_op->bin += 3 << 10; //param 1 bits 12-13
					op2_ins->bin = (op2_type-r0) << 2;
				}
				//operand 2 is an immediate number
				else if(op2_type >= MIN_NUM && op2_type <= MAX_NUM) {
					new_op->bin += 0 << 10;
					op2_ins->bin = op2_type << 2;
				}
				//operand 2 is a label
				else if(op2_type == LABEL){ //assuming it is a label for now
					new_op->bin += 1 << 10;
					op2_ins->bin = LABEL;
				}
			}
		}

		NEW_LINE:;
	}

	//adding the data to the end of the memory so DC is after IC
	DC_current = DC->next;
	label_current = label->next;
	int offset = IC_current->loc;
	while(DC_current != NULL) {

		DC_current->loc += offset;
		DC_current = DC_current->next;
	}
	while(label_current != NULL) {
		if(label_current->loc < 0) label_current->loc = (-1 * label_current->loc) + offset;
		label_current = label_current->next;
	}
	//passing all of the information gathered in firstRun to secondRun
	secondRun(file_name, IC, DC, label, entries, externs, hasError);
}


