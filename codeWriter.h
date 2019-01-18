#ifndef _CODE_WRITER_H_
#define _CODE_WRITER_H_

#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fout;
char outputFileName[FILE_NAMES_MAX_LENGTH];

void initializeCodeWriter(const char* outputFileName);
void setFileName(const char* fileName);
void prepareOperation();
void prepareSecondOperand();
void updateAfterBinaryOperation();

// instructions bust be a valid assembly code string with \n at the end of each instruction
// M is the first operand, D is the second (if present)
void operate(const char instructions[], int isBinary, ...);

void compare(const char jumpMnemonic[]);

void writeArithmetic(const char* command);
void writePushPop(enum commandType type, enum memorySegment segment, int index);
void translate();
void close();

#endif