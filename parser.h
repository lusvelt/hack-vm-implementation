#ifndef _PARSER_H_
#define _PARSER_H_

#include <sys/types.h>
#include <dirent.h>
#include "includes.h"
#include "helper.h"
#include "parser.h"

DIR *dp;
char files[MAX_FILES][FILE_NAMES_MAX_LENGTH];
FILE *fin;
char currentCommand[COMMAND_MAX_LENGTH];
char inputFileName[FILE_NAMES_MAX_LENGTH];
int currentLine;
int filesCount;
int currentFile;
char input[FILE_NAMES_MAX_LENGTH];

enum commandType {
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL
};

enum memorySegment {
    M_ARGUMENT,
    M_LOCAL,
    M_STATIC,
    M_CONSTANT,
    M_THIS,
    M_THAT,
    M_POINTER,
    M_TEMP
};

struct command_t {
    enum commandType type;
    enum memorySegment segment;
    int index;
    char label[SYMBOL_MAX_LENGTH];
    int vars;
    char** args;
};

struct command_t command;

void initializeParser(const char *input);
int hasMoreCommands();
void scanIgnoreSpaces(char *c, int *comment, const int stopAtNewLine);
void throwError(const char *error);
int advance();
char** splitBySpaces(char s[]);
void checkArguments(int expected);
enum memorySegment getSegment(char s[]);
void checkIndex(int index, int max);
int getIndex(char s[]);
int getVars(char s[]);
void parseAndCheckCommand();
char* arg1();
char* arg2();
void nextFile();

#endif