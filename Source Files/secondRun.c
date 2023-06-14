#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fileList.h"

/*
 * This file contains the second run:
 * 1)Translates the labels binary by the location of labels collected in the first run
 * 2)Creates the .ob file which contains the location of each line of binary and the binary itself - end product
 *
 * Receives the file name, list of instructions, list of data instructions, and the list of labels.
 */


void secondRun(char *file_name, ins* IC, ins* DC, lab* label, ins* entries, lab* externs, unsigned char hasError) {

	if(entries->next != NULL) {
		ins* entry_current = entries->next;
		//creates .ent file for the entries
		char *ent_file = (char *)malloc(sizeof(file_name));
		strcpy(ent_file, file_name);
		strcat(ent_file, ".ent");
		FILE *ent_ptr = fopen(ent_file, "w");

		while(entry_current != NULL) {
			lab* label_current = NULL;
			if(label->next != NULL) label_current = label->next;
			while(label_current != NULL) {
				if(strcmp(label_current->name, entry_current->word) == 0) {
					fprintf(ent_ptr, "%s\t%d\n", label_current->name, label_current->loc);
					break;
				}
				label_current = label_current->next;
			}

			if(label_current == NULL) {
				printf("ERROR line %d: unable to declare entry %s - no label found\n", entry_current->line, entry_current->word);
				++hasError;
				remove(ent_file);
				break;
			}
			entry_current = entry_current->next;
		}
		if(fopen(ent_file, "r")) {
			printf("File %s created\n", ent_file);
			fclose(ent_ptr);
		}

	}

	lab* extern_reference_list = (lab *)calloc(1, sizeof(lab));
	lab* extern_reference_current = extern_reference_list;

	//changes the binary of the instructions to the location of the label
	if(IC->next != NULL) {
		ins* IC_current = IC->next;
		while(IC_current != NULL) {
			lab *label_current = NULL, *extern_current = NULL;
			if(IC_current->bin == LABEL) { //labels were marked as negative during firstRun
				if(label->next != NULL) label_current = label->next;
				while(label_current != NULL) {
					if(strcmp(IC_current->word, label_current->name) == 0) {
						IC_current->bin = (label_current->loc << 2) + 2; //bin(10) for the 2 ERA bits - label is relocatable
						break;
					}
					label_current = label_current->next;
				}

				if(externs->next != NULL) extern_current = externs->next;
				while(extern_current != NULL) {
					if(strcmp(IC_current->word, extern_current->name) == 0) {
						IC_current->bin = 1; //extern location is 0, with the ERA bits equaling 01
						lab* new_reference = (lab *)calloc(1, sizeof(lab));
						new_reference->name = (char *)malloc(sizeof(char));
						strcpy(new_reference->name, extern_current->name);
						new_reference->loc = IC_current->loc;
						extern_reference_current->next = new_reference;
						extern_reference_current = extern_reference_current->next;
						break;
					}
					extern_current = extern_current->next;
				}
				//no label is found
				if(label_current == NULL && extern_current == NULL) {
					printf("ERROR line %d: %s label not found\n", IC_current->line, IC_current->word);
					printf("Errors found: %d. Program terminated.\n", ++hasError);
					return;
				}
			}
			IC_current = IC_current->next;
		}
	}
	if(hasError) {
		printf("Errors found: %d. Program terminated.\n", hasError);
		return;
	}


	//creating .ext file containing the references to the external labels
	if(extern_reference_list->next != NULL) {

		char *ext_file = (char *)malloc(sizeof(file_name));
		strcpy(ext_file, file_name);
		strcat(ext_file, ".ext");
		FILE *ext_ptr = fopen(ext_file, "w");

		extern_reference_current = extern_reference_list->next;
		while(extern_reference_current != NULL) {
			fprintf(ext_ptr, "%s\t%d\n", extern_reference_current->name, extern_reference_current->loc);
			extern_reference_current = extern_reference_current->next;
		}
		printf("File %s created\n", ext_file);
		fclose(ext_ptr);
	}

	//creating .ob file containing the memory location and the corresponding binary code
	char *ob_file = (char *)malloc(sizeof(file_name));
	strcpy(ob_file, file_name);
	strcat(ob_file, ".ob");
	FILE *ob_ptr = fopen(ob_file, "w");

	ins *IC_current = NULL, *DC_current = NULL;
	if(IC->next != NULL) IC_current = IC->next; //first node is a placeholder
	if(DC->next != NULL) DC_current = DC->next;

	while(IC_current != NULL) {
		fprintf(ob_ptr, "0%d\t",IC_current->loc);

		for(int j = 13; j >= 0; j--) fputc('.' + ((IC_current->bin >> j) & 1), ob_ptr);
		fputc('\n', ob_ptr);

		IC_current = IC_current->next;
	}
	while(DC_current != NULL) {
		fprintf(ob_ptr, "0%d\t",DC_current->loc);

		for(int j = 13; j >= 0; j--) fputc('.' + ((DC_current->bin >> j) & 1), ob_ptr);
		fputc('\n', ob_ptr);

		DC_current = DC_current->next;
	}
	printf("File %s created\n", ob_file);
	fclose(ob_ptr);
	/*
	//debugging
	char *debug_file = malloc(sizeof(file_name));
	strcpy(debug_file, file_name);
	strcat(debug_file, ".txt");
	FILE *debug_ptr = fopen(debug_file, "w");

	if(IC->next != NULL) IC_current = IC->next; //first node is a placeholder
	if(DC->next != NULL) DC_current = DC->next;

	while(IC_current != NULL) {
		fprintf(debug_ptr, "%d\t%s\t",IC_current->loc, IC_current->word);

		for(int j = 13; j >= 0; j--) fputc('0' + ((IC_current->bin >> j) & 1), debug_ptr);
		fputc('\n', debug_ptr);

		IC_current = IC_current->next;
	}
	while(DC_current != NULL) {
		fprintf(debug_ptr, "%d\t%s\t",DC_current->loc, DC_current->word);

		for(int j = 13; j >= 0; j--) fputc('0' + ((DC_current->bin >> j) & 1), debug_ptr);
		fputc('\n', debug_ptr);

		DC_current = DC_current->next;
	}
*/
}
