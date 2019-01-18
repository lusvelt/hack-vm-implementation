#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fout;
char outputFileName[FILE_NAMES_MAX_LENGTH];
int comparisons = -1;

void initializeCodeWriter(const char* outputFileName) {
    fout = fopen(outputFileName, "w");
    fprintf(fout, "@256\nD=A\n@SP\nM=D\n");
}

void setFileName(const char* fileName) {

}

void prepareOperation() {
    fprintf(fout, "@SP\nA=M\nA=A-1\n");
}

void prepareSecondOperand() {
    fprintf(fout, "D=M\nA=A-1\n");
}

void updateAfterBinaryOperation() {
    fprintf(fout, "D=A+1\n@SP\nM=D\n");
}

void operate(const char instructions[], int isBinary) {
    prepareOperation();

    if (isBinary)
        prepareSecondOperand();

    fprintf(fout, instructions);
    
    if (isBinary)
        updateAfterBinaryOperation();
}

void compare(const char jumpMnemonic[]) {
    operate("M=M-D\n", 1);
    prepareOperation();

    fprintf(fout, "D=M\n@START_CMP:%d\nD;%s\n", comparisons, jumpMnemonic);
    fprintf(fout, "@ELSE_CMP:%d\n0;JMP\n", comparisons);
    fprintf(fout, "(START_CMP:%d)\n@SP\nA=M\nA=A-1\nM=0\nM=!M\n", comparisons);
    fprintf(fout, "@END_CMP:%d\n0;JMP\n", comparisons);
    fprintf(fout, "(ELSE_CMP:%d)\n@SP\nA=M\nA=A-1\nM=0\n", comparisons);
    fprintf(fout, "(END_CMP:%d)\n", comparisons);

    comparisons++;
}

void writeArithmetic(const char* command) {
    if (strcmp(command, "add") == 0)
        operate("M=M+D\n", 1);
    else if (strcmp(command, "sub") == 0)
        operate("M=M-D\n", 1);
    else if (strcmp(command, "neg") == 0)
        operate("M=-M\n", 0);
    else if (strcmp(command, "eq") == 0)
        compare("JEQ");
    else if (strcmp(command, "gt") == 0)
        compare("JGT");
    else if (strcmp(command, "lt") == 0)
        compare("JLT");
    else if (strcmp(command, "and") == 0)
        operate("M=M&D\n", 1);
    else if (strcmp(command, "or") == 0)
        operate("M=M|D\n", 1);
    else if (strcmp(command, "not") == 0)
        operate("M=!M\n", 0);        
}

void writePushPop(enum commandType type, enum memorySegment segment, int index) {
    if (type == C_PUSH) {
        if (segment == M_CONSTANT)
            fprintf(fout, "@%d\nD=A\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", index);
    } else if (type == C_POP) {

    }
}

void translate() {
    if (command.type == C_PUSH || command.type == C_POP)
        writePushPop(command.type, command.segment, command.index);
    else if (command.type == C_ARITHMETIC)
        writeArithmetic(command.args[0]);
}

void close() {
    fclose(fout);
}
