#ifndef _PARSER_H_
#define _PARSER_H_

#include "includes.h"

#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fin, *fout;
char currentCommand[COMMAND_MAX_LENGTH];
char inputFileName[FILE_NAMES_MAX_LENGTH], outputFileName[FILE_NAMES_MAX_LENGTH];
int currentLine;

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
    char* arg1;
    int arg2;
};

struct command_t command;

void initializeParser(const char *inputFileName, const char *outputFileName);
int hasMoreCommands();
void scanIgnoreSpaces(char *c, int *comment, const int stopAtNewLine);
void throwError(const char *error);
int advance();
void parseCommand();
void checkCommand();
char* arg1();
char* arg2();
void saveChanges();

#endif