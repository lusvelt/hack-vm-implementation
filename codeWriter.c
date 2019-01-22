#include "includes.h"
#include "helper.h"
#include "parser.h"

FILE *fout;
char outputFileName[FILE_NAMES_MAX_LENGTH];
int comparisons = -1;

void writeInit() {
    fprintf(fout, "@256\nD=A\n@SP\nM=D\n");
}

void initializeCodeWriter(const char* outputFileName) {
    fout = fopen(outputFileName, "w");
    writeInit();
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

void push(int index, char address[]) {
    fprintf(fout, "@%d\nD=A\n@%s\nA=D+M\nD=M\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", index, address);
}

void pop(int index, char address[]) {
    fprintf(fout, "@%d\nD=A\n@%s\nD=D+M\n@R5\nM=D\n@SP\nAM=M-1\nD=M\n@R5\nA=M\nM=D\n", index, address);
}

void writePushPop(enum commandType type, enum memorySegment segment, int index) {
    if (type == C_PUSH) {
        if (segment == M_CONSTANT)
            fprintf(fout, "@%d\nD=A\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", index);
        else if (segment == M_STATIC)
            fprintf(fout, "@%d\nD=A\n@16\nA=D+A\nD=M\n@SP\nA=M\nM=D\nD=A+1\n@SP\nM=D\n", index);
        else if (segment == M_ARGUMENT)
            push(index, "ARG");
        else if (segment == M_LOCAL)
            push(index, "LCL");
    } else if (type == C_POP) {
        if (segment == M_CONSTANT)
            throwError("Cannot pop from segment 'constant'");
        else if (segment == M_STATIC)
            fprintf(fout, "@%d\nD=A\n@16\nD=D+A\n@R5\nM=D\n@SP\nAM=M-1\nD=M\n@R5\nA=M\nM=D\n", index);
        else if (segment == M_ARGUMENT)
            pop(index, "ARG");
        else if (segment == M_LOCAL)
            pop(index, "LCL");
    }
}

void writeLabel(char label[]) {
    fprintf(fout, "(%s)\n", label);
}

void writeGoto(char label[]) {
    fprintf(fout, "@%s\n0;JMP\n", label);
}

void writeIf(char label[]) {
    fprintf(fout, "@SP\nA=M-1\nD=M\n@%s\nD;JNE\n", label);
}

void translate() {
    if (command.type == C_PUSH || command.type == C_POP)
        writePushPop(command.type, command.segment, command.index);
    else if (command.type == C_ARITHMETIC)
        writeArithmetic(command.args[0]);
    else if (command.type == C_LABEL)
        writeLabel(command.label);
    else if (command.type == C_GOTO)
        writeGoto(command.label);
    else if (command.type == C_IF)
        writeIf(command.label);
}

void close() {
    fclose(fout);
}
