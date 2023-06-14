#define MAX_LABEL_LENGTH 30
//twos complement maximum and minimum for 12 bits
#define MAX_NUM 2047
#define MIN_NUM -2048
#define ERROR 2048
#define LABEL 2049
#define r0 2050
#define r7 2057
#define _GNU_SOURCE
//#define INVALID[18] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string"};

/*
 * This file contains the data structures used in the compiler program.
 */

typedef struct S_list {
	char *value;
	struct S_list* next;
}node;

typedef struct S_macro {
	char *name;
	node *lines;
	struct S_macro *next;
}node2d;

typedef struct S_instruction {
	int line;
	int loc;
	char *word;
	int bin;
	struct S_instruction *next;
}ins;

typedef struct S_label {
	int loc;
	char *name;
	struct S_label *next;
}lab;

//main functions
void preAssembly(char *file_name);
void firstRun(char *file_name);
void secondRun(char *file_name, ins* IC, ins* DC, lab* label, ins* entries, lab* externs, unsigned char hasError);

//general string functions:
char* removeWhitespaces(char *input);
int getMethodValue(char *word);
int validLabel(char *word, lab* label);

//instruction functions:
ins* newIns(ins* IC, const char* name, int line);

//list-node functions:
int listLength(node *head);
int searchNode(node *head, char *word);
node* spaceSplit(char *input);
node* operandSplit(char *input);
void getFiles(char *input, node *head);
int validFile(node *input);

//2d list-node functions
int searchNode2d(node2d *head, char *word);
int searchMacro(node2d *head, node *line);




