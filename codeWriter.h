#ifndef _CODE_WRITER_H_
#define _CODE_WRITER_H_

#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fout;
char outputFileName[FILE_NAMES_MAX_LENGTH];
char currentFunction[SYMBOL_MAX_LENGTH];
int comparisonsCount;
int callsCount;

void writeInit();
void initializeCodeWriter(const char* outputFileName);
void setFileName(const char* fileName);
void prepareUnaryOperation();
void prepareBinaryOperation();

// instructions bust be a valid assembly code string with \n at the end of each instruction
// M is the first operand, D is the second (if present)
void operate(const char instructions[], int isBinary, ...);

void compare(const char jumpMnemonic[]);

void writeArithmetic(const char* command);
void writePushPop(enum commandType type, enum memorySegment segment, int index);
void writeLabel(char label[]);
void writeGoto(char label[]);
void writeIf(char label[]);
void writeCall(char functionName[], int numArgs);
void writeReturn();
void writeFunction(char functionName[], int numLocals);
void translate();
void initializeStatics();
void close();

#endif